/*

Copyright (c) 2000  Microsoft Corporation

Module Name:

	access.h

Abstract:

	This module contains prototypes for access related routines.

Author:

	Jameel Hyder (microsoft!jameelh)


Revision History:
	20 Sep 2000		Initial Version

Notes:	Tab stop: 4
--*/

#ifndef _ACCESS_
#define _ACCESS_

#define	AFP_READ_ACCESS		(READ_CONTROL		 |	\
							FILE_READ_ATTRIBUTES |	\
							FILE_TRAVERSE		 |	\
							FILE_LIST_DIRECTORY	 |	\
							FILE_READ_EA)

#define	AFP_WRITE_ACCESS	(FILE_ADD_FILE		 |	\
							FILE_ADD_SUBDIRECTORY|	\
							FILE_WRITE_ATTRIBUTES|	\
							FILE_WRITE_EA		 |	\
							DELETE)

#define	AFP_OWNER_ACCESS	(WRITE_DAC			  | \
							 WRITE_OWNER)

#define	AFP_MIN_ACCESS		(FILE_READ_ATTRIBUTES | \
							 READ_CONTROL)

#ifdef	i386
#pragma warning(disable:4010)
#endif

GLOBAL	SID		AfpSidWorld			EQU	\
			{ 1, 1, SECURITY_WORLD_SID_AUTHORITY, SECURITY_WORLD_RID };
GLOBAL	SID		AfpSidSystem		EQU	\
			{ 1, 1, SECURITY_NT_AUTHORITY, SECURITY_LOCAL_SYSTEM_RID };
GLOBAL	SID		AfpSidNull			EQU	\
			{ 1, 1, SECURITY_NULL_SID_AUTHORITY, SECURITY_NULL_RID };

GLOBAL	SID		AfpSidCrtrOwner		EQU	\
			{ 1, 1, SECURITY_CREATOR_SID_AUTHORITY, SECURITY_CREATOR_OWNER_RID };
GLOBAL	SID		AfpSidCrtrGroup		EQU	\
			{ 1, 1, SECURITY_CREATOR_SID_AUTHORITY, SECURITY_CREATOR_GROUP_RID };
GLOBAL	SID		AfpSidBuiltIn		EQU \
			{ 1, 1, SECURITY_NT_AUTHORITY, SECURITY_BUILTIN_DOMAIN_RID };
GLOBAL	PSID	AfpSidAdmins		EQU NULL;
GLOBAL	LONG	AfpSizeSidAdmins	EQU	0;

#define	AfpAccessMask2AfpPermissions(Rights, Mask, Type)					\
				if ((Type) == ACCESS_ALLOWED_ACE_TYPE)						\
				{															\
					if (((Mask) & AFP_READ_ACCESS) == AFP_READ_ACCESS)		\
						 (Rights) |= (DIR_ACCESS_READ | DIR_ACCESS_SEARCH);	\
					if (((Mask) & AFP_WRITE_ACCESS) == AFP_WRITE_ACCESS)	\
						(Rights) |= DIR_ACCESS_WRITE;						\
					if (((Mask) & AFP_OWNER_ACCESS) == AFP_OWNER_ACCESS)	\
						(Rights) |= DIR_ACCESS_OWNER;						\
				}															\
				else														\
				{															\
					ASSERT((Type) == ACCESS_DENIED_ACE_TYPE);				\
					if ((Mask) & AFP_READ_ACCESS)							\
						(Rights) &= ~(DIR_ACCESS_READ | DIR_ACCESS_SEARCH); \
					if ((Mask) & AFP_WRITE_ACCESS)							\
						(Rights) &= ~DIR_ACCESS_WRITE;						\
					if ((Mask) & AFP_OWNER_ACCESS)							\
						(Rights) &= ~DIR_ACCESS_OWNER;						\
				}

extern
NTSTATUS
AfpGetUserAndPrimaryGroupSids(
	IN	PSDA						pSda
);


extern
AFPSTATUS
AfpMakeSecurityDescriptorForUser(
	IN	PSDA	pSda
);


extern
AFPSTATUS
AfpGetAfpPermissions(
	IN	PSDA						pSda,
	IN	HANDLE						DirHandle,
	IN OUT struct _FileDirParms *	pFDParm
);


extern
AFPSTATUS
AfpSetAfpPermissions(
	IN	HANDLE						DirHandle,
	IN	DWORD						Bitmap,
	IN OUT struct _FileDirParms *	pFDParm
);

#ifdef	DEBUG

extern
VOID
AfpDumpSid(
	IN	PBYTE						pString,
	IN	PISID						pSid
);

extern
VOID
AfpDumpSidnMask(
	IN	PBYTE						pString,
	IN	PISID						pSid,
	IN	DWORD						Mask,
	IN	UCHAR						Type,
	IN	UCHAR						Flags
);

#else

#define	AfpDumpSid(pString, pSid)
#define	AfpDumpSidnMask(pString, pSid, Mask, Type, Flags)

#endif

#ifdef	NON_PGD_MEM_FOR_ACCESS
#define	ALLOC_ACCESS_MEM(x)	AfpAllocNonPagedMemory(x)
#else
#define	ALLOC_ACCESS_MEM(x)	AfpAllocPagedMemory(x)
#endif

#ifdef	_ACCESS_LOCALS

LOCAL	BOOLEAN
afpIsUserMemberOfGroup(
	IN	PTOKEN_GROUPS				pGroups,
	IN	PSID						pSidGroup
);


LOCAL	ACCESS_MASK
afpPermissions2NtMask(
	IN	BYTE						AfpPermissions
);

LOCAL	PACCESS_ALLOWED_ACE
afpAddAceToAcl(
	IN	PACL						pAcl,
	IN	PACCESS_ALLOWED_ACE			pAce,
	IN	ACCESS_MASK					Mask,
	IN	PSID						pSid,
	IN	BOOLEAN						fInherit,
	IN	PSID						pSidInherit OPTIONAL
);

LOCAL PACCESS_ALLOWED_ACE
afpMoveAces(
	IN	PACL						pOldDacl,
	IN	PACCESS_ALLOWED_ACE			pAceStart,
	IN	PSID						pSidOldOwner,
	IN	PSID						pSidNewOwner,
	IN	PSID						pSidOldGroup,
	IN	PSID						pSidNewGroup,
	IN	BOOLEAN						DenyAces,
	IN	OUT PACL					pNewDacl
);

#endif	// _ACCESS_LOCALS

#endif	// _ACCESS_

