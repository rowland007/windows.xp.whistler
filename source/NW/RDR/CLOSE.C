/*++

Copyright (c) 2000-4  Microsoft Corporation

Module Name:

    Close.c

Abstract:

    This module implements the File Close routine for the NetWare
    redirector called by the dispatch driver.

Author:

    Colin Watson     [ColinW]    19-Dec-2000

Revision History:

--*/

#include "Procs.h"

//
//  The local debug trace level
//

#define Dbg                              (DEBUG_TRACE_CLOSE)

//
//  Local procedure prototypes
//

NTSTATUS
NwCommonClose (
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
NwCloseRcb (
    IN PIRP_CONTEXT IrpContext,
    IN PRCB Rcb
    );

NTSTATUS
NwCloseIcb (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, NwFsdClose )
#pragma alloc_text( PAGE, NwCommonClose )
#pragma alloc_text( PAGE, NwCloseRcb )
#pragma alloc_text( PAGE, NwCloseIcb )
#endif


NTSTATUS
NwFsdClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine implements the FSD part of Close.

Arguments:

    DeviceObject - Supplies the redirector device object.

    Irp - Supplies the Irp being processed

Return Value:

    NTSTATUS - The FSD status for the IRP

--*/

{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext;
    BOOLEAN TopLevel;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwFsdClose\n", 0);

    //
    //  Call the common Close routine
    //

    FsRtlEnterFileSystem();
    TopLevel = NwIsIrpTopLevel( Irp );

    try {

        IrpContext = AllocateIrpContext( Irp );
        Status = NwCommonClose( IrpContext );

    } except(NwExceptionFilter( Irp, GetExceptionInformation() )) {

        //
        // We had some trouble trying to perform the requested
        // operation, so we'll abort the I/O request with
        // the error status that we get back from the
        // execption code.
        //

        Status = NwProcessException( IrpContext, GetExceptionCode() );
    }

    NwDequeueIrpContext( IrpContext, FALSE );
    NwCompleteRequest( IrpContext, Status );

    if ( TopLevel ) {
        NwSetTopLevelIrp( NULL );
    }
    FsRtlExitFileSystem();

    //
    //  And return to our caller
    //

    DebugTrace(-1, Dbg, "NwFsdClose -> %08lx\n", Status);

    UNREFERENCED_PARAMETER( DeviceObject );

    return Status;
}


NTSTATUS
NwCommonClose (
    IN PIRP_CONTEXT IrpContext
    )

/*++

Routine Description:

    This is the common routine for closing a file.

Arguments:

    IrpContext - Supplies the Irp to process

Return Value:

    NTSTATUS - the return status for the operation

--*/

{
    PIRP Irp;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;
    NODE_TYPE_CODE nodeTypeCode;
    PVOID fsContext, fsContext2;

    PAGED_CODE();

    Irp = IrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NwCommonClose\n", 0);
    DebugTrace( 0, Dbg, "IrpContext       = %08lx\n", (ULONG)IrpContext);
    DebugTrace( 0, Dbg, "Irp              = %08lx\n", (ULONG)Irp);
    DebugTrace( 0, Dbg, "FileObject       = %08lx\n", (ULONG)irpSp->FileObject);
    try {

        //
        // Get the a referenced pointer to the node and make sure it is
        // not being closed.
        //

        if ((nodeTypeCode = NwDecodeFileObject( irpSp->FileObject,
                                                &fsContext,
                                                &fsContext2 )) == NTC_UNDEFINED) {

            DebugTrace(0, Dbg, "The file is disconnected\n", 0);

            status = STATUS_INVALID_HANDLE;

            DebugTrace(-1, Dbg, "NwCommonClose -> %08lx\n", status );
            try_return( NOTHING );
        }

        //
        // Decide how to handle this IRP.
        //

        switch (nodeTypeCode) {


        case NW_NTC_RCB:       // Close the file system

            status = NwCloseRcb( IrpContext, (PRCB)fsContext2 );
            status = STATUS_SUCCESS;
            break;

        case NW_NTC_ICB:       // Close the remote file
        case NW_NTC_ICB_SCB:   // Close the SCB

            status = NwCloseIcb( IrpContext, (PICB)fsContext2 );
            NwDereferenceUnlockableCodeSection ();
            break;

#ifdef NWDBG
        default:

            //
            // This is not one of ours.
            //

            KeBugCheck( RDR_FILE_SYSTEM );
            break;
#endif

        }

    try_exit: NOTHING;

    } finally {

        //
        //  Just in-case this handle was the last one before we unload.
        //

        NwUnlockCodeSections(TRUE);

        DebugTrace(-1, Dbg, "NwCommonClose -> %08lx\n", status);

    }

    return status;
}


NTSTATUS
NwCloseRcb (
    IN PIRP_CONTEXT IrpContext,
    IN PRCB Rcb
    )

/*++

Routine Description:

    The routine cleans up a RCB.

Arguments:

    IrpContext - Supplies the IRP context pointers for this close.

    Rcb - Supplies the RCB for MSFS.

Return Value:

    NTSTATUS - An appropriate completion status

--*/

{
    NTSTATUS status;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwCloseRcb...\n", 0);

    //
    //  Now acquire exclusive access to the Rcb
    //

    NwAcquireExclusiveRcb( Rcb, TRUE );

    status = STATUS_SUCCESS;
    --Rcb->OpenCount;

    NwReleaseRcb( Rcb );

    DebugTrace(-1, Dbg, "MsCloseRcb -> %08lx\n", status);

    //
    //  And return to our caller
    //

    return status;
}


NTSTATUS
NwCloseIcb (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb
    )

/*++

Routine Description:

    The routine cleans up an ICB.

Arguments:

    IrpContext - Supplies the IRP context pointers for this close.

    Rcb - Supplies the RCB for MSFS.

Return Value:

    NTSTATUS - An appropriate completion status

--*/
{
    NTSTATUS Status;
    PNONPAGED_SCB pNpScb;
    PVCB Vcb;
    PFCB Fcb;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwCloseIcb...\n", 0);

    ASSERT( Icb->State == ICB_STATE_CLEANED_UP ||
            Icb->State == ICB_STATE_CLOSE_PENDING );

    //
    // If this is a remote file close the remote handle.
    //

    Status = STATUS_SUCCESS;
    IrpContext->Icb = Icb;
    Fcb = Icb->SuperType.Fcb;

    if (( Icb->NodeTypeCode == NW_NTC_ICB ) ||
        ( Icb->NodeTypeCode == NW_NTC_DCB )) {

        pNpScb = Fcb->Scb->pNpScb;
        IrpContext->pNpScb = pNpScb;

        if ( Icb->HasRemoteHandle ) {

            Vcb = Fcb->Vcb;

            //
            //  Dump the write behind cache.
            //

            Status = FlushCache( IrpContext, Fcb->NonPagedFcb );

            if ( !NT_SUCCESS( Status ) ) {
                IoRaiseInformationalHardError(
                    STATUS_LOST_WRITEBEHIND_DATA,
                    &Fcb->FullFileName,
                    (PKTHREAD)IrpContext->pOriginalIrp->Tail.Overlay.Thread );
            }

            //
            //  Is this a print job?
            //  Icb->IsPrintJob will be false if a 16 bit app is
            //  responsible for sending the job.
            //

            if ( FlagOn( Vcb->Flags, VCB_FLAG_PRINT_QUEUE ) &&
                 Icb->IsPrintJob ) {

                //
                //  Yes, did we print?
                //

                if ( Icb->ActuallyPrinted ) {

                    //
                    //  Yes.  Send a close file and start queue job NCP
                    //

                    Status = ExchangeWithWait(
                                IrpContext,
                                SynchronousResponseCallback,
                                "Sdw",
                                NCP_ADMIN_FUNCTION, NCP_CLOSE_FILE_AND_START_JOB,
                                Vcb->Specific.Print.QueueId,
                                Icb->JobId );
                } else {

                    //
                    //  No.  Cancel the job.
                    //

                    Status = ExchangeWithWait(
                                IrpContext,
                                SynchronousResponseCallback,
                                "Sdw",
                                NCP_ADMIN_FUNCTION, NCP_CLOSE_FILE_AND_CANCEL_JOB,
                                Vcb->Specific.Print.QueueId,
                                Icb->JobId );
                }

            } else {

                if ( Icb->SuperType.Fcb->NodeTypeCode != NW_NTC_DCB ) {

                    //
                    //  No, send a close file NCP.
                    //

                    ASSERT( IrpContext->pTdiStruct == NULL );

                    Status = ExchangeWithWait(
                                IrpContext,
                                SynchronousResponseCallback,
                                "F-r",
                                NCP_CLOSE,
                                Icb->Handle, sizeof( Icb->Handle ) );

                } else {

                    Status = ExchangeWithWait (
                                 IrpContext,
                                 SynchronousResponseCallback,
                                 "Sb",
                                 NCP_DIR_FUNCTION, NCP_DEALLOCATE_DIR_HANDLE,
                                 Icb->Handle[0]);
                }

            }

            Icb->HasRemoteHandle = FALSE;
        }

    } else {

        pNpScb = Icb->SuperType.Scb->pNpScb;
        IrpContext->pNpScb = pNpScb;

    }

    if ( Icb->Pid != INVALID_PID ) {

        //
        //  This ICB was involved in a search, send the end job,
        //  then free the PID.
        //

        NwUnmapPid( Icb->Pid, IrpContext );
    }

    //
    //  Update the time the SCB was last used.
    //

    KeQuerySystemTime( &pNpScb->LastUsedTime );

    //
    //  Wait the SCB queue.  We do this now since NwDeleteIcb may cause
    //  a packet to be sent by this thread (from NwCleanupVcb()) while
    //  holding the RCB.  To eliminate this potential source of deadlock,
    //  queue this IrpContext to the SCB queue before acquiring the RCB.
    //
    //  Also, we mark this IRP context not reconnectable, since the
    //  reconnect logic, will try to acquire the RCB.
    //

    NwAppendToQueueAndWait( IrpContext );
    ClearFlag( IrpContext->Flags, IRP_FLAG_RECONNECTABLE );

    //
    //  Delete the ICB.
    //

    NwDeleteIcb( IrpContext, Icb );

    //
    //  And return to our caller
    //

    DebugTrace(-1, Dbg, "NwCloseIcb -> %08lx\n", Status);
    return Status;
}


