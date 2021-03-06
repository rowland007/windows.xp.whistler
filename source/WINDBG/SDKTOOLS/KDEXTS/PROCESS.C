/*++

Copyright (c) 2000  Microsoft Corporation

Module Name:

    process.c

Abstract:

    WinDbg Extension Api

Author:

    Wesley Witt (wesw) 15-Aug-1993

Environment:

    User Mode.

Revision History:

--*/

#include <handle.h>

extern ULONG   STeip, STebp, STesp;
static PHANDLETABLE   PspCidTable;
static HANDLETABLE    CapturedPspCidTable;


BOOLEAN
GetTheSystemTime (
    OUT PLARGE_INTEGER Time
    );


DECLARE_API( process )

/*++

Routine Description:

    Dumps the active process list.

Arguments:

    None.

Return Value:

    None.

--*/

{
    ULONG ProcessToDump;
    ULONG Flags;
    ULONG Result;
    LIST_ENTRY List;
    PLIST_ENTRY Next;
    ULONG ProcessHead;
    PEPROCESS Process;
    EPROCESS ProcessContents;
    PETHREAD Thread;
    ETHREAD ThreadContents;

    ProcessToDump = 0xFFFFFFFF;
    Flags = 0xFFFFFFFF;
    sscanf(args,"%lx %lx",&ProcessToDump,&Flags);
    if (ProcessToDump == 0xFFFFFFFF) {
        ProcessToDump = (ULONG)GetCurrentProcessAddress( dwProcessor, hCurrentThread, NULL );
        if (ProcessToDump == 0) {
            dprintf("Unable to get current process pointer.\n");
            return;
        }
        if (Flags == 0xFFFFFFFF) {
            Flags = 3;
        }
    }

    if (ProcessToDump == 0) {
        dprintf("**** NT ACTIVE PROCESS DUMP ****\n");
        if (Flags == 0xFFFFFFFF) {
            Flags = 3;
        }
    }

    if (ProcessToDump < MM_USER_PROBE_ADDRESS) {
        ProcessHead = GetExpression( "PsActiveProcessHead" );
        if (!ProcessHead) {
            dprintf("Unable to get value of PsActiveProcessHead\n");
            return;
        }

        if (!ReadMemory( ProcessHead, &List, sizeof(LIST_ENTRY), &Result )) {
            dprintf("Unable to get value of PsActiveProcessHead\n");
            return;
        }

        if (ProcessToDump != 0) {
            dprintf("Searching for Process with Cid == %lx\n", ProcessToDump);
        }

        Next = List.Flink;
        if (Next == NULL) {
            dprintf("PsActiveProcessHead is NULL!\n");
            return;
        }
    }
    else {
        Next = NULL;
        ProcessHead = 1;
        }

    while((ULONG)Next != ProcessHead) {
        if (Next != NULL) {
            Process = CONTAINING_RECORD(Next,EPROCESS,ActiveProcessLinks);
        }
        else {
            Process = (PEPROCESS)ProcessToDump;
        }

        if (!ReadMemory( (DWORD)Process, &ProcessContents, sizeof(EPROCESS), &Result )) {
            dprintf("Unable to read _EPROCESS at %lx\n",Process);
            return;
        }

        if (ProcessToDump == 0 ||
            ProcessToDump < MM_USER_PROBE_ADDRESS && ProcessToDump == (ULONG)ProcessContents.UniqueProcessId ||
            ProcessToDump > MM_USER_PROBE_ADDRESS && ProcessToDump == (ULONG)Process
           ) {
            if (DumpProcess (&ProcessContents, Process, Flags) && Flags & 6) {
                Next = ProcessContents.Pcb.ThreadListHead.Flink;

                while ( Next != &Process->Pcb.ThreadListHead) {

                    Thread = (PETHREAD)(CONTAINING_RECORD(Next,KTHREAD,ThreadListEntry));
                    if (!ReadMemory((DWORD)Thread,
                                    &ThreadContents,
                                    sizeof(ETHREAD),
                                    &Result)) {
                        dprintf("Unable to read _ETHREAD at %lx\n",Thread);
                        break;
                    }

                    if (!DumpThread("        ", &ThreadContents, Thread, Flags)) {
                        break;
                        }


                    Next = ((PKTHREAD)&ThreadContents)->ThreadListEntry.Flink;

                    if (CheckControlC()) {
                        return;
                    }
                }
                EXPRLastDump = (ULONG)Process;
                dprintf("\n");
            }

            if (ProcessToDump != 0) {
                return;
            }
        }

        if (Next == NULL) {
            return;
        }
        Next = ProcessContents.ActiveProcessLinks.Flink;

        if (CheckControlC()) {
            return;
        }
    }
    return;
}


DECLARE_API( thread )

/*++

Routine Description:

    Dumps the specified thread.

Arguments:

    None.

Return Value:

    None.

--*/

{
    ULONG       Address;
    ULONG       Flags;
    ULONG       result;
    PETHREAD    Thread;
    ETHREAD     ThreadContents;

    Address = 0xFFFFFFFF;
    Flags = 6;
    sscanf(args,"%lx %lx",&Address,&Flags);
    if (Address == 0xFFFFFFFF) {
        Address = (ULONG)GetCurrentThreadAddress( (USHORT)dwProcessor, hCurrentThread);
    }

    Thread = (PETHREAD)(PVOID)Address;
    if ( !ReadMemory( (DWORD)Thread,
                      &ThreadContents,
                      sizeof(ETHREAD),
                      &result) ) {
        dprintf("%08lx: Unable to get thread contents\n", Thread );
        return;
    }

    DumpThread ("", &ThreadContents, Thread, Flags);
    EXPRLastDump = (ULONG)Thread;
    return;

}

BOOL
DumpProcess (
    IN PEPROCESS ProcessContents,
    IN PEPROCESS RealProcessBase,
    IN ULONG Flags
    )
{
    HANDLETABLE HandleTable;
    ULONG NumberOfHandles;
    WCHAR Buf[256];
    ULONG Result;
    LARGE_INTEGER RunTime;
    TIME_FIELDS Times;
    ULONG KeTimeIncrement;
    ULONG TimeIncrement;

    if (ProcessContents->Pcb.Header.Type != ProcessObject) {
        dprintf("TYPE mismatch for process object at %lx\n",RealProcessBase);
        return FALSE;
    }

    NumberOfHandles = 0;
    if (ProcessContents->ObjectTable) {
        if (ReadMemory((DWORD)ProcessContents->ObjectTable,
                         &HandleTable,
                         sizeof(HandleTable),
                         &Result)) {
            NumberOfHandles = HandleTable.CountTableEntries;
        }
    }
    dprintf("PROCESS %08lx  Cid: %04lx  Peb: %08x  ParentCid: %04lx  DirBase: %04lx  ObjectTable: %08lx(%u)\n",
            RealProcessBase,
            ProcessContents->UniqueProcessId,
            ProcessContents->Peb,
            ProcessContents->InheritedFromUniqueProcessId,
            ProcessContents->Pcb.DirectoryTableBase[ 0 ],
            ProcessContents->ObjectTable,
            NumberOfHandles
           );

    if ( ProcessContents->ImageFileName[ 0 ] != '\0' ) {
        strcpy((char *)Buf,ProcessContents->ImageFileName);
        }
    else {
        strcpy((char *)Buf,"System Process");
        }
    dprintf("    Image: %s\n",Buf);
    if (!(Flags & 1)) {
        dprintf("\n");
        return TRUE;
        }

    dprintf("    VadRoot %lx  Clone Root %lx Private Pages %lx Modified Pages %lx\n",ProcessContents->VadRoot,
                        ProcessContents->CloneRoot,
                        ProcessContents->NumberOfPrivatePages,
                        ProcessContents->ModifiedPageCount);
    dprintf("    %X MutantState %s OwningThread %lx\n",
            &RealProcessBase->ProcessMutant,
            ProcessContents->ProcessMutant.Header.SignalState ? "Signalled" : "Locked",
            ProcessContents->ProcessMutant.OwnerThread);

    if (ProcessContents->LockCount != 1) {
        dprintf("    Process Lock Owned by Thread %lx\n",
                ProcessContents->LockOwner);
    }

    //
    // Primary token
    //


    dprintf("    Token                             %lx\n", ProcessContents->Token                        );

    //
    // Get the time increment value which is used to compute runtime.
    //
    KeTimeIncrement = GetExpression( "KeTimeIncrement" );
    if (KeTimeIncrement) {
        ReadMemory( KeTimeIncrement, &TimeIncrement, sizeof(ULONG), &Result );
    }

    GetTheSystemTime (&RunTime);

    RunTime = RtlLargeIntegerSubtract(
                RunTime,
                *(PLARGE_INTEGER)&ProcessContents->CreateTime
                );

    RtlTimeToElapsedTimeFields ( &RunTime, &Times);
    dprintf("    ElapsedTime                      %3ld:%02ld:%02ld.%04ld\n",
              Times.Hour,
              Times.Minute,
              Times.Second,
              Times.Milliseconds);

    RunTime = RtlEnlargedUnsignedMultiply(ProcessContents->Pcb.UserTime,
                                          TimeIncrement);

    RtlTimeToElapsedTimeFields ( &RunTime, &Times);
    dprintf("    UserTime                        %3ld:%02ld:%02ld.%04ld\n",
              Times.Hour,
              Times.Minute,
              Times.Second,
              Times.Milliseconds);

    RunTime = RtlEnlargedUnsignedMultiply(ProcessContents->Pcb.KernelTime,
                                          TimeIncrement);

    RtlTimeToElapsedTimeFields ( &RunTime, &Times);
    dprintf("    KernelTime                      %3ld:%02ld:%02ld.%04ld\n",
              Times.Hour,
              Times.Minute,
              Times.Second,
              Times.Milliseconds);

    dprintf("    QuotaPoolUsage[PagedPool]         %ld\n",ProcessContents->QuotaPoolUsage[PagedPool]       );
    dprintf("    QuotaPoolUsage[NonPagedPool]      %ld\n",ProcessContents->QuotaPoolUsage[NonPagedPool]    );
    dprintf("    Working Set Sizes (now,min,max)  (%ld, %ld, %ld)\n",ProcessContents->Vm.WorkingSetSize,
                                                                    ProcessContents->Vm.MinimumWorkingSetSize,
                                                                    ProcessContents->Vm.MaximumWorkingSetSize);
    dprintf("    PeakWorkingSetSize                %ld\n",ProcessContents->Vm.PeakWorkingSetSize           );
    dprintf("    VirtualSize                       %ld Mb\n",ProcessContents->VirtualSize /(1024*1024)     );
    dprintf("    PeakVirtualSize                   %ld Mb\n",ProcessContents->PeakVirtualSize/(1024*1024)  );
    dprintf("    PageFaultCount                    %ld\n",ProcessContents->Vm.PageFaultCount               );
    dprintf("    MemoryPriority                    %s\n",ProcessContents->Vm.MemoryPriority ? "FOREGROUND" : "BACKGROUND" );
    dprintf("    BasePriority                      %ld\n",ProcessContents->Pcb.BasePriority);
    dprintf("    CommitCharge                      %ld\n",ProcessContents->CommitCharge                    );


    dprintf("\n");
    return TRUE;
}


UCHAR *WaitReasonList[] = {
    "Executive",
    "FreePage",
    "PageIn",
    "PoolAllocation",
    "DelayExecution",
    "Suspended",
    "UserRequest",
    "WrExecutive",
    "WrFreePage",
    "WrPageIn",
    "WrPoolAllocation",
    "WrDelayExecution",
    "WrSuspended",
    "WrUserRequest",
    "WrEventPairHigh",
    "WrEventPairLow",
    "WrLpcReceive",
    "WrLpcReply",
    "WrVirtualMemory",
    "WrPageOut",
    "Spare1",
    "Spare2",
    "Spare3",
    "Spare4",
    "Spare5",
    "Spare6",
    "Spare7"};

BOOL
DumpThread (
    IN char *Pad,
    IN PETHREAD Thread,
    IN PETHREAD RealThreadBase,
    IN ULONG Flags
    )
{
    TIME_FIELDS Times;
    LARGE_INTEGER RunTime;
    ULONG Address;
    ULONG Result;
    KMUTANT WaitObject;
    PVOID PointerWaitObject = &WaitObject;
    PKWAIT_BLOCK WaitBlock;
    KWAIT_BLOCK OutsideBlock;
    ULONG WaitOffset;
    PEPROCESS Process;
    CHAR Buffer[80];
    ULONG KeTimeIncrement;
    ULONG TimeIncrement;
    ULONG frames = 0;
    ULONG i;
    ULONG displacement;
    EXTSTACKTRACE stk[20];
#ifdef TARGET_i386
    KSWITCHFRAME SwitchFrame;
#endif


    if (Thread->Tcb.Header.Type != ThreadObject) {
        dprintf("TYPE mismatch for thread object at %lx\n",RealThreadBase);
        return FALSE;
    }

    dprintf("%sTHREAD %lx  Cid %lx.%lx  Teb: %08x ",
        Pad,
        RealThreadBase,
        Thread->Cid.UniqueProcess,
        Thread->Cid.UniqueThread,
        Thread->Tcb.Teb
        );


    switch (Thread->Tcb.State) {
        case Initialized:
            dprintf("%s\n","INITIALIZED");break;
        case Ready:
            dprintf("%s\n","READY");break;
        case Running:
            dprintf("%s\n","RUNNING");break;
        case Standby:
            dprintf("%s\n","STANDBY");break;
        case Terminated:
            dprintf("%s\n","TERMINATED");break;
        case Waiting:
            dprintf("%s","WAIT:");break;
        case Transition:
            dprintf("%s","TRANSITION");break;
    }

    if (!(Flags & 2)) {
        dprintf("\n");
        return TRUE;
        }

    if (Thread->Tcb.State == Waiting) {
        dprintf(" (%s) %s %s\n",
            WaitReasonList[Thread->Tcb.WaitReason],
            (Thread->Tcb.WaitMode==KernelMode) ? "KernelMode" : "UserMode",Thread->Tcb.Alertable ? "Alertable" : "Non-Alertable");
        if ( Thread->Tcb.SuspendCount ) {
            dprintf("SuspendCount %lx\n",Thread->Tcb.SuspendCount);
        }
        if ( Thread->Tcb.FreezeCount ) {
            dprintf("FreezeCount %lx\n",Thread->Tcb.FreezeCount);
        }

        WaitOffset =
               (ULONG)Thread->Tcb.WaitBlockList - (ULONG)RealThreadBase;

        if (WaitOffset > (ULONG)sizeof(ETHREAD)) {
            if (!ReadMemory((DWORD)Thread->Tcb.WaitBlockList,
                            &OutsideBlock,
                            sizeof(KWAIT_BLOCK),
                            &Result)) {
                dprintf("%sunable to get Wait object\n",Pad);
                goto BadWaitBlock;
            }
            WaitBlock = &OutsideBlock;
        } else {
            WaitBlock = (PKWAIT_BLOCK)((ULONG)Thread + WaitOffset);
        }

        do {

            dprintf("%s    %lx  ",Pad,WaitBlock->Object);

            if (!ReadMemory((DWORD)WaitBlock->Object,
                            &WaitObject,
                            sizeof(KMUTANT),
                            &Result)) {
                dprintf("%sunable to get Wait object\n",Pad);
                break;
            }

            switch (WaitObject.Header.Type) {
                case 0:
                    dprintf("NotificationEvent\n");
                    break;
                case EventObject:
                    dprintf("SynchronizationEvent\n");
                    break;
                case SemaphoreObject:
                    dprintf("Semaphore Limit 0x%lx\n",
                             ((PKSEMAPHORE)PointerWaitObject)->Limit);
                    break;
                case ThreadObject:
                    dprintf("Thread\n");
                    break;
                case TimerObject:
                    dprintf("Timer\n");
                    break;
                case EventPairObject:
                    dprintf("EventPair\n");
                    break;
                case ProcessObject:
                    dprintf("ProcessObject\n");
                    break;
                case MutantObject:
                    dprintf("Mutant - owning thread %lx\n",
                            ((PKMUTANT)PointerWaitObject)->OwnerThread);
                    break;
                default:
                    dprintf("Unknown\n");
                    break;
            }

            if (WaitBlock->NextWaitBlock == Thread->Tcb.WaitBlockList) {
                break;
            }
            WaitOffset =
                   (ULONG)WaitBlock->NextWaitBlock - (ULONG)RealThreadBase;

            if (WaitOffset > (ULONG)sizeof(ETHREAD)) {

                if (!ReadMemory((DWORD)WaitBlock->NextWaitBlock,
                                &OutsideBlock,
                                sizeof(KWAIT_BLOCK),
                                &Result)) {
                    dprintf("%sunable to get Wait object\n",Pad);
                    break;
                }
                WaitBlock = &OutsideBlock;
            } else {
                WaitBlock = (PKWAIT_BLOCK)((ULONG)Thread + WaitOffset);
            }
        } while ( TRUE );
    }

BadWaitBlock:
    if (!(Flags & 4)) {
        dprintf("\n");
        return TRUE;
    }


    if (Thread->LpcReplyMessageId != 0) {
        dprintf("%sWaiting for reply to LPC MessageId %08x:\n",Pad,Thread->LpcReplyMessageId);
    }

    if (Thread->LpcReplyMessage != NULL) {
        LPCP_MESSAGE MsgContents, *p;

        dprintf("%sPending LPC Reply Message:\n",Pad);
        Address = (ULONG)Thread->LpcReplyMessage;
        if (!ReadMemory((DWORD)Address,
                        &MsgContents,
                        sizeof(MsgContents),
                        &Result)) {
            dprintf("unable to get LPC msg\n");
        } else {
            p = &MsgContents;
            dprintf("%s    %08lx: [%08lx,%08lx]\n",
                    Pad, Address, p->Entry.Blink, p->Entry.Flink
                   );
        }
    }

    if (Thread->IrpList.Flink != Thread->IrpList.Blink ||
        Thread->IrpList.Flink != &RealThreadBase->IrpList
       ) {

        ULONG IrpListHead = (ULONG)&RealThreadBase->IrpList;
        PLIST_ENTRY Next;
        IRP IrpContents;
        PIRP p;
        ULONG Counter = 0;

        Next = Thread->IrpList.Flink;
        dprintf("%sIRP List:\n",Pad);
        while (((ULONG)Next != IrpListHead) && (Counter < 17)) {
            Counter += 1;
            Address = (ULONG)CONTAINING_RECORD(Next,IRP,ThreadListEntry);
            if (!ReadMemory((DWORD)Address,
                           &IrpContents,
                           sizeof(IRP),
                           &Result)) {
                dprintf( "%sunable to get IRP object\n", Pad );
                break;
            }

            p = &IrpContents;
            dprintf("%s    %08lx: (%04x,%04x) Flags: %08lx  Mdl: %08lx\n",
                    Pad,Address,p->Type,p->Size,p->Flags,p->MdlAddress);

            Next = p->ThreadListEntry.Flink;
        }
    }

    //
    // Impersonation information
    //

    if (Thread->Client != NULL) {
        dprintf("%sImpersonation token: %lx\n", Pad, Thread->Client);
    } else {
        dprintf("%sNot impersonating\n", Pad);
    }

    Process = CONTAINING_RECORD(Thread->Tcb.ApcState.Process,EPROCESS,Pcb);
    dprintf("%sOwning Process %lx\n", Pad, Process);

    GetTheSystemTime (&RunTime);

    dprintf("%sWaitTime (seconds)      %ld\n",
              Pad,
              Thread->Tcb.WaitTime);

    dprintf("%sContext Switch Count    %ld\n",
              Pad,
              Thread->Tcb.ContextSwitches);

    //
    // Get the time increment value which is used to compute runtime.
    //
    KeTimeIncrement = GetExpression( "KeTimeIncrement" );
    if (KeTimeIncrement) {
        ReadMemory( KeTimeIncrement, &TimeIncrement, sizeof(ULONG), &Result );
    }

    RunTime = RtlEnlargedUnsignedMultiply(Thread->Tcb.UserTime,
                                          TimeIncrement);

    RtlTimeToElapsedTimeFields ( &RunTime, &Times);
    dprintf("%sUserTime                %3ld:%02ld:%02ld.%04ld\n",
              Pad,
              Times.Hour,
              Times.Minute,
              Times.Second,
              Times.Milliseconds);

    RunTime = RtlEnlargedUnsignedMultiply(Thread->Tcb.KernelTime,
                                          TimeIncrement);

    RtlTimeToElapsedTimeFields ( &RunTime, &Times);
    dprintf("%sKernelTime              %3ld:%02ld:%02ld.%04ld\n",
              Pad,
              Times.Hour,
              Times.Minute,
              Times.Second,
              Times.Milliseconds);

    if (Thread->PerformanceCountHigh != 0) {
        dprintf("%sPerfCounterHigh         0x%lx %08lx\n",
                Pad,
                Thread->PerformanceCountHigh,
                Thread->PerformanceCountHigh);
    } else if (Thread->PerformanceCountLow != 0) {
        dprintf("%sPerfCounter             %lu\n",Pad,Thread->PerformanceCountLow);
    }

    dumpSymbolicAddress((ULONG)Thread->StartAddress, Buffer, TRUE);
    dprintf("%sStart Address %s\n",
        Pad,
        Buffer
        );

    if (Thread->Win32StartAddress)
        if (Thread->LpcReceivedMsgIdValid)
            {
            dprintf("%sLPC Server thread working on message Id %x\n",
                Pad,
                Thread->LpcReceivedMessageId
                );
            }
        else
            {
            dumpSymbolicAddress((ULONG)Thread->Win32StartAddress, Buffer, TRUE);
            dprintf("%sWin32 Start Address %s\n",
                Pad,
                Buffer
                );
            }

    dprintf("%sInitial Sp %lx Current Sp %lx\n",
        Pad,
        Thread->Tcb.InitialStack,
        Thread->Tcb.KernelStack
        );

    dprintf("%sPriority %ld BasePriority %ld PriorityDecrement %ld DecrementCount %ld\n",
        Pad,
        Thread->Tcb.Priority,
        Thread->Tcb.BasePriority,
        Thread->Tcb.PriorityDecrement,
        Thread->Tcb.DecrementCount
        );

    if (!Thread->Tcb.KernelStackResident) {
        dprintf("Kernel stack not resident.\n");
    }

#ifdef TARGET_i386
        //
        // Get SwitchFrame and perform backtrace providing EBP,ESP,EIP
        // (full FPO backtrace context)
        //

        if (ReadMemory( (DWORD) Thread->Tcb.KernelStack,
                        (PVOID) &SwitchFrame,
                        sizeof (SwitchFrame),
                        &Result)) {

            frames = StackTrace( SwitchFrame.Ebp,
                                 (ULONG) Thread->Tcb.KernelStack +
                                                       sizeof(SwitchFrame),
                                 SwitchFrame.RetAddr,
                                 stk,
                                 20 );

            if (frames) {
                STeip = SwitchFrame.RetAddr;
                STebp = SwitchFrame.Ebp;
                STesp = (ULONG) Thread->Tcb.KernelStack +
                                sizeof(SwitchFrame);
            }
        }
#elif defined(TARGET_MIPS) || defined(TARGET_ALPHA)
        if (Thread->Tcb.State == Running) {
            //
            // this will generate a stacktrace for the current thread
            //
            frames = StackTrace( 0, 0, 0, stk, 20 );
        } else {
            frames = StackTrace( (DWORD)Thread->Tcb.KernelStack,
                                 (DWORD)Thread->Tcb.KernelStack,
                                 0,
                                 stk,
                                 20 );
        }
#endif

        for (i=0; i<frames; i++) {

            if (i==0) {
                dprintf( "%sChildEBP RetAddr  Args to Child\n", Pad );
            }

            Buffer[0] = '!';
            GetSymbol((LPVOID)stk[i].ProgramCounter, Buffer, &displacement);

            dprintf( "%s%08x %08x %08x %08x %08x %s",
                     Pad,
                     stk[i].FramePointer,
                     stk[i].ReturnAddress,
                     stk[i].Args[0],
                     stk[i].Args[1],
                     stk[i].Args[2],
                     Buffer
                   );

            if (displacement) {
                dprintf( "+0x%x", displacement );
            }

            dprintf( "\n" );


        }

        if (frames) {





        }

    dprintf("\n");
    return TRUE;
}

DECLARE_API( processfields )

/*++

Routine Description:

    Displays the field offsets for EPROCESS type.

Arguments:

    None.

Return Value:

    None.

--*/

{

    dprintf(" EPROCESS structure offsets:\n\n");

    dprintf("    Pcb:                               0x%lx\n", FIELD_OFFSET(EPROCESS, Pcb) );
    dprintf("    ExitStatus:                        0x%lx\n", FIELD_OFFSET(EPROCESS, ExitStatus) );
    dprintf("    LockEvent:                         0x%lx\n", FIELD_OFFSET(EPROCESS, LockEvent) );
    dprintf("    LockCount:                         0x%lx\n", FIELD_OFFSET(EPROCESS, LockCount) );
    dprintf("    CreateTime:                        0x%lx\n", FIELD_OFFSET(EPROCESS, CreateTime) );
    dprintf("    ExitTime:                          0x%lx\n", FIELD_OFFSET(EPROCESS, ExitTime) );
    dprintf("    LockOwner:                         0x%lx\n", FIELD_OFFSET(EPROCESS, LockOwner) );
    dprintf("    UniqueProcessId:                   0x%lx\n", FIELD_OFFSET(EPROCESS, UniqueProcessId) );
    dprintf("    ActiveProcessLinks:                0x%lx\n", FIELD_OFFSET(EPROCESS, ActiveProcessLinks) );
    dprintf("    QuotaPeakPoolUsage[0]:             0x%lx\n", FIELD_OFFSET(EPROCESS, QuotaPeakPoolUsage) );
    dprintf("    QuotaPoolUsage[0]:                 0x%lx\n", FIELD_OFFSET(EPROCESS, QuotaPoolUsage) );
    dprintf("    PagefileUsage:                     0x%lx\n", FIELD_OFFSET(EPROCESS, PagefileUsage) );
    dprintf("    CommitCharge:                      0x%lx\n", FIELD_OFFSET(EPROCESS, CommitCharge) );
    dprintf("    PeakPagefileUsage:                 0x%lx\n", FIELD_OFFSET(EPROCESS, PeakPagefileUsage) );
    dprintf("    PeakVirtualSize:                   0x%lx\n", FIELD_OFFSET(EPROCESS, PeakVirtualSize) );
    dprintf("    VirtualSize:                       0x%lx\n", FIELD_OFFSET(EPROCESS, VirtualSize) );
    dprintf("    Vm:                                0x%lx\n", FIELD_OFFSET(EPROCESS, Vm) );
    dprintf("    LastProtoPteFault:                 0x%lx\n", FIELD_OFFSET(EPROCESS, LastProtoPteFault) );
    dprintf("    DebugPort:                         0x%lx\n", FIELD_OFFSET(EPROCESS, DebugPort) );
    dprintf("    ExceptionPort:                     0x%lx\n", FIELD_OFFSET(EPROCESS, ExceptionPort) );
    dprintf("    ObjectTable:                       0x%lx\n", FIELD_OFFSET(EPROCESS, ObjectTable) );
    dprintf("    Token:                             0x%lx\n", FIELD_OFFSET(EPROCESS, Token) );
    dprintf("    WorkingSetLock:                    0x%lx\n", FIELD_OFFSET(EPROCESS, WorkingSetLock) );
    dprintf("    WorkingSetPage:                    0x%lx\n", FIELD_OFFSET(EPROCESS, WorkingSetPage) );
    dprintf("    ProcessOutswapEnabled:             0x%lx\n", FIELD_OFFSET(EPROCESS, ProcessOutswapEnabled) );
    dprintf("    ProcessOutswapped:                 0x%lx\n", FIELD_OFFSET(EPROCESS, ProcessOutswapped) );
    dprintf("    AddressSpaceInitialized:           0x%lx\n", FIELD_OFFSET(EPROCESS, AddressSpaceInitialized) );
    dprintf("    AddressSpaceDeleted:               0x%lx\n", FIELD_OFFSET(EPROCESS, AddressSpaceDeleted) );
    dprintf("    AddressCreationLock:               0x%lx\n", FIELD_OFFSET(EPROCESS, AddressCreationLock) );
    dprintf("    ForkInProgress:                    0x%lx\n", FIELD_OFFSET(EPROCESS, ForkInProgress) );
    dprintf("    VmOperation:                       0x%lx\n", FIELD_OFFSET(EPROCESS, VmOperation) );
    dprintf("    VmOperationEvent:                  0x%lx\n", FIELD_OFFSET(EPROCESS, VmOperationEvent) );
    dprintf("    PageDirectoryPte:                  0x%lx\n", FIELD_OFFSET(EPROCESS, PageDirectoryPte) );
    dprintf("    LastFaultCount:                    0x%lx\n", FIELD_OFFSET(EPROCESS, LastFaultCount) );
    dprintf("    VadRoot:                           0x%lx\n", FIELD_OFFSET(EPROCESS, VadRoot) );
    dprintf("    VadHint:                           0x%lx\n", FIELD_OFFSET(EPROCESS, VadHint) );
    dprintf("    CloneRoot:                         0x%lx\n", FIELD_OFFSET(EPROCESS, CloneRoot) );
    dprintf("    NumberOfPrivatePages:              0x%lx\n", FIELD_OFFSET(EPROCESS, NumberOfPrivatePages) );
    dprintf("    NumberOfLockedPages:               0x%lx\n", FIELD_OFFSET(EPROCESS, NumberOfLockedPages) );
    dprintf("    ForkWasSuccessful:                 0x%lx\n", FIELD_OFFSET(EPROCESS, ForkWasSuccessful) );
    dprintf("    ExitProcessCalled:                 0x%lx\n", FIELD_OFFSET(EPROCESS, ExitProcessCalled) );
    dprintf("    CreateProcessReported:             0x%lx\n", FIELD_OFFSET(EPROCESS, CreateProcessReported) );
    dprintf("    SectionHandle:                     0x%lx\n", FIELD_OFFSET(EPROCESS, SectionHandle) );
    dprintf("    Peb:                               0x%lx\n", FIELD_OFFSET(EPROCESS, Peb) );
    dprintf("    SectionBaseAddress:                0x%lx\n", FIELD_OFFSET(EPROCESS, SectionBaseAddress) );
    dprintf("    QuotaBlock:                        0x%lx\n", FIELD_OFFSET(EPROCESS, QuotaBlock) );
    dprintf("    LastThreadExitStatus:              0x%lx\n", FIELD_OFFSET(EPROCESS, LastThreadExitStatus) );
    dprintf("    WorkingSetWatch:                   0x%lx\n", FIELD_OFFSET(EPROCESS, WorkingSetWatch) );
    dprintf("    LpcPort:                           0x%lx\n", FIELD_OFFSET(EPROCESS, LpcPort) );
    dprintf("    InheritedFromUniqueProcessId:      0x%lx\n", FIELD_OFFSET(EPROCESS, InheritedFromUniqueProcessId) );
    dprintf("    GrantedAccess:                     0x%lx\n", FIELD_OFFSET(EPROCESS, GrantedAccess) );
    dprintf("    DefaultHardErrorProcessing         0x%lx\n", FIELD_OFFSET(EPROCESS, DefaultHardErrorProcessing) );
    dprintf("    LdtInformation:                    0x%lx\n", FIELD_OFFSET(EPROCESS, LdtInformation) );
    dprintf("    VadFreeHint:                       0x%lx\n", FIELD_OFFSET(EPROCESS, VadFreeHint) );
    dprintf("    VdmObjects:                        0x%lx\n", FIELD_OFFSET(EPROCESS, VdmObjects) );
    dprintf("    ProcessMutant:                     0x%lx\n", FIELD_OFFSET(EPROCESS, ProcessMutant) );
    dprintf("    ImageFileName[0]:                  0x%lx\n", FIELD_OFFSET(EPROCESS, ImageFileName) );
    dprintf("    VmTrimFaultValue:                  0x%lx\n", FIELD_OFFSET(EPROCESS, VmTrimFaultValue) );

    return;
}


DECLARE_API( threadfields )

/*++

Routine Description:

    Displays the field offsets for ETHREAD type.

Arguments:

    None.

Return Value:

    None.

--*/

{

    dprintf(" ETHREAD structure offsets:\n\n");


    dprintf("    Tcb:                           0x%lx\n", FIELD_OFFSET(ETHREAD, Tcb) );
    dprintf("    CreateTime:                    0x%lx\n", FIELD_OFFSET(ETHREAD, CreateTime) );
    dprintf("    ExitTime:                      0x%lx\n", FIELD_OFFSET(ETHREAD, ExitTime) );
    dprintf("    ExitStatus:                    0x%lx\n", FIELD_OFFSET(ETHREAD, ExitStatus) );
    dprintf("    PostBlockList:                 0x%lx\n", FIELD_OFFSET(ETHREAD, PostBlockList) );
    dprintf("    TerminationPortList:           0x%lx\n", FIELD_OFFSET(ETHREAD, TerminationPortList) );
    dprintf("    ActiveTimerListLock:           0x%lx\n", FIELD_OFFSET(ETHREAD, ActiveTimerListLock) );
    dprintf("    ActiveTimerListHead:           0x%lx\n", FIELD_OFFSET(ETHREAD, ActiveTimerListHead) );
    dprintf("    Cid:                           0x%lx\n", FIELD_OFFSET(ETHREAD, Cid) );
    dprintf("    LpcReplySemaphore:             0x%lx\n", FIELD_OFFSET(ETHREAD, LpcReplySemaphore) );
    dprintf("    LpcReplyMessage:               0x%lx\n", FIELD_OFFSET(ETHREAD, LpcReplyMessage) );
    dprintf("    LpcReplyMessageId:             0x%lx\n", FIELD_OFFSET(ETHREAD, LpcReplyMessageId) );

    dprintf("    Client:                        0x%lx\n", FIELD_OFFSET(ETHREAD, Client) );
    dprintf("    IrpList:                       0x%lx\n", FIELD_OFFSET(ETHREAD, IrpList) );
    dprintf("    TopLevelIrp:                   0x%lx\n", FIELD_OFFSET(ETHREAD, TopLevelIrp) );
    dprintf("    ReadClusterSize:               0x%lx\n", FIELD_OFFSET(ETHREAD, ReadClusterSize) );
    dprintf("    ForwardClusterOnly:            0x%lx\n", FIELD_OFFSET(ETHREAD, ForwardClusterOnly) );
    dprintf("    DisablePageFaultClustering:    0x%lx\n", FIELD_OFFSET(ETHREAD, DisablePageFaultClustering) );
    dprintf("    DeadThread:                    0x%lx\n", FIELD_OFFSET(ETHREAD, DeadThread) );
    dprintf("    HasTerminated:                 0x%lx\n", FIELD_OFFSET(ETHREAD, HasTerminated) );
    dprintf("    EventPair:                     0x%lx\n", FIELD_OFFSET(ETHREAD, EventPair) );
    dprintf("    GrantedAccess:                 0x%lx\n", FIELD_OFFSET(ETHREAD, GrantedAccess) );
    dprintf("    ThreadsProcess:                0x%lx\n", FIELD_OFFSET(ETHREAD, ThreadsProcess) );
    dprintf("    StartAddress:                  0x%lx\n", FIELD_OFFSET(ETHREAD, StartAddress) );
    dprintf("    Win32StartAddress:             0x%lx\n", FIELD_OFFSET(ETHREAD, Win32StartAddress) );
    dprintf("    LpcExitThreadCalled:           0x%lx\n", FIELD_OFFSET(ETHREAD, LpcExitThreadCalled) );
    dprintf("    HardErrorsAreDisabled:         0x%lx\n", FIELD_OFFSET(ETHREAD, HardErrorsAreDisabled) );

    return;

}

PVOID
GetCurrentProcessAddress(
    DWORD    Processor,
    HANDLE   hCurrentThread,
    PETHREAD CurrentThread
    )
{
    ULONG Result;
    ETHREAD Thread;

    if (CurrentThread == NULL) {
        CurrentThread = (PETHREAD)GetCurrentThreadAddress( (USHORT)Processor, hCurrentThread );
        if (CurrentThread == NULL) {
            return NULL;
        }
    }

    if (!ReadMemory((DWORD)CurrentThread, &Thread, sizeof(Thread), &Result)) {
        return NULL;
    }

    return CONTAINING_RECORD(Thread.Tcb.ApcState.Process,EPROCESS,Pcb);
}

PVOID
GetCurrentThreadAddress(
    USHORT Processor,
    HANDLE hCurrentThread
    )
{
    ULONG Address;

#ifdef TARGET_ALPHA

    ReadControlSpace( (USHORT)Processor,
                      (PVOID)DEBUG_CONTROL_SPACE_THREAD,
                      (PVOID)&Address,
                      sizeof(PKTHREAD),
                     );

    return CONTAINING_RECORD(Address, ETHREAD, Tcb);

#elif defined(TARGET_MIPS)

    KPRCB  Prcb;

    if (!ReadPcr(Processor, &Prcb, &Address, hCurrentThread)) {
        dprintf("Unable to read PCR for Processor %u\n",Processor);
        return NULL;
    }

    return CONTAINING_RECORD(Prcb.CurrentThread,ETHREAD,Tcb);

#elif defined(TARGET_i386)

    KPCR  Pcr;
    PKPCR pp;

    pp = &Pcr;
    if (!ReadPcr(Processor, pp, &Address, hCurrentThread)) {
        dprintf("Unable to read PCR for Processor %u\n",Processor);
        return NULL;
    }

    return CONTAINING_RECORD(pp->PrcbData.CurrentThread,ETHREAD,Tcb);

#else

#error( "unknown processor type" )

#endif
}


#if defined(TARGET_i386)
#define SYSTEM_TIME_ADDRESS  KI_USER_SHARED_DATA
#elif defined(TARGET_MIPS)
#define SYSTEM_TIME_ADDRESS  KIPCR2
#elif defined(TARGET_ALPHA)
#define SYSTEM_TIME_ADDRESS  KI_USER_SHARED_DATA
#else
#error( "unknown target machine" );
#endif


BOOLEAN
GetTheSystemTime (
    OUT PLARGE_INTEGER Time
    )
{
    KUSER_SHARED_DATA  sud;
    ULONG              Result;


    ZeroMemory( Time, sizeof(*Time) );

    if (!ReadMemory(SYSTEM_TIME_ADDRESS,
                    &sud,
                    sizeof(KUSER_SHARED_DATA),
                    &Result)) {
        dprintf( "unable to read memory @ SharedUserData\n");
        return FALSE;
    }

    *Time = *(PLARGE_INTEGER)&sud.SystemTime;

    return TRUE;
}

VOID
dumpSymbolicAddress(
    ULONG Address,
    PUCHAR Buffer,
    BOOL AlwaysShowHex
    )
{
    ULONG displacement;
    PUCHAR s;

    Buffer[0] = '!';
    GetSymbol((LPVOID)Address, Buffer, &displacement);
    s = Buffer + strlen( Buffer );
    if (s == Buffer) {
        sprintf( s, "0x%08x", Address );
        }
    else {
        if (displacement != 0) {
            sprintf( s, "+0x%x", displacement );
            }
        if (AlwaysShowHex) {
            sprintf( s, " (0x%08x)", Address );
            }
        }

    return;
}

BOOLEAN
FetchProcessStructureVariables(
    VOID
    )
{
    ULONG Result;
    static BOOLEAN HavePspVariables = FALSE;

    if (HavePspVariables) {
        return TRUE;
    }

    PspCidTable = (PHANDLETABLE)GetExpression( "PspCidTable" );
    if ( !PspCidTable ||
         !ReadMemory((DWORD)PspCidTable,
                     &PspCidTable,
                     sizeof(PspCidTable),
                     &Result) ) {
        dprintf("%08lx: Unable to get value of PspCidTable\n",PspCidTable);
        return FALSE;
    }

    HavePspVariables = TRUE;
    return TRUE;
}


PVOID
LookupUniqueId(
    HANDLE UniqueId
    )
{
    ULONG Result;
    ULONG TableIndex;
    PULONG pEntry;
    ULONG Entry;
    static BOOLEAN HavePspCidTable = FALSE;

    if (!HavePspCidTable) {
        if ( !ReadMemory( (DWORD)PspCidTable,
                          &CapturedPspCidTable,
                          sizeof(CapturedPspCidTable),
                          &Result) ) {
            return NULL;
        }

        HavePspCidTable = TRUE;
        }

    pEntry = (PULONG)CapturedPspCidTable.TableEntries;
    TableIndex = (ULONG)UniqueId - 1;
    if (TableIndex < CapturedPspCidTable.CountTableEntries) {
        pEntry = (PULONG)
            ((PCHAR)CapturedPspCidTable.TableEntries +
             (TableIndex << (CapturedPspCidTable.LogSizeTableEntry + 2))
            );

        if ( !ReadMemory( (DWORD)pEntry,
                          &Entry,
                          sizeof( Entry ),
                          &Result) ) {
            Entry = 0;
            }
        else
        if (TestFreePointer( Entry )) {
            Entry = 0;
            }
        }
    else {
        Entry = 0;
        }

    return (PVOID)Entry;
}
