/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	XCopy.hxx

Abstract:

	This module contains the definition for the XCOPY class, which
	implements the DOS5-compatible XCopy utility.

Author:

	Ramon Juan San Andres (ramonsa) 01-May-1990


Revision History:


--*/


#if !defined( _XCOPY_ )

#define _XCOPY_

//
//	Exit codes
//
#define 	EXIT_NORMAL 			0
#define 	EXIT_NO_FILES			1
#define 	EXIT_TERMINATED 		2
#define 	EXIT_MISC_ERROR 		4
#define 	EXIT_READWRITE_ERROR	5

#include "object.hxx"
#include "keyboard.hxx"
#include "program.hxx"

//
//	Forward references
//
DECLARE_CLASS( ARRAY );
DECLARE_CLASS( FSN_DIRECTORY );
DECLARE_CLASS( FSNODE );
DECLARE_CLASS( FSN_FILE );
DECLARE_CLASS( FSN_FILTER );
DECLARE_CLASS( TIMEINFO );
DECLARE_CLASS( XCOPY );
DECLARE_CLASS( ARGUMENT_LEXEMIZER );
DECLARE_CLASS( STRING_ARRAY );
DECLARE_CLASS( ITERATOR );

class XCOPY : public PROGRAM {

	public:

		DECLARE_CONSTRUCTOR( XCOPY );

		NONVIRTUAL
		~XCOPY (
			);

		NONVIRTUAL
		BOOLEAN
		Initialize (
			);

		NONVIRTUAL
		BOOLEAN
		DoCopy (
			);


	private:

        NONVIRTUAL
		VOID
		Construct  (
			);

        NONVIRTUAL
		VOID
		AbortIfCtrlC(
			VOID
			);

        NONVIRTUAL
		BOOLEAN
        CheckTargetSpace (
			IN OUT	PFSN_FILE	File,
			IN		PPATH		DestinationPath
			);

        NONVIRTUAL
		VOID
		CheckArgumentConsistency (
			);

		NONVIRTUAL
		BOOLEAN
		Copier (
			IN OUT	PFSN_FILE	File,
			IN		PPATH		DestinationPath
			);

		NONVIRTUAL
		VOID
		DeallocateThings (
			);

		NONVIRTUAL
		VOID
		DisplayMessageAndExit (
			IN	MSGID		MsgId,
			IN	PWSTRING	String,
			IN	ULONG		ExitCode
			);

		NONVIRTUAL
		VOID
		ExitWithError(
			IN	DWORD		ErrorCode
			);

		NONVIRTUAL
		VOID
		GetArgumentsCmd(
			);

		NONVIRTUAL
		VOID
		GetDirectoryAndFilePattern(
            IN  PPATH           Path,
            IN  BOOLEAN         CopyingManyFiles,
			OUT PPATH			*OutDirectory,
			OUT PWSTRING		*OutFilePattern
			);

		NONVIRTUAL
		VOID
		GetDirectoryAndFilters(
			IN	PPATH			Path,
			OUT PFSN_DIRECTORY	*OutDirectory,
			OUT PFSN_FILTER 	*FileFilter,
            OUT PFSN_FILTER     *DirectoryFilter,
            OUT PBOOLEAN        CopyingManyFiles
			);

		NONVIRTUAL
		VOID
		InitializeThings (
			);

		NONVIRTUAL
		BOOL
		IsCyclicalCopy(
			IN PPATH	 PathSrc,
			IN PPATH	 PathTrg
			);

		NONVIRTUAL
		BOOL
		IsFileName(
            IN PPATH     Path,
            IN BOOLEAN   CopyingManyFiles
			);

		NONVIRTUAL
        PARGUMENT_LEXEMIZER
		ParseArguments(
			IN	PWSTRING	CmdLine,
			OUT PARRAY		ArgArray
			);

		NONVIRTUAL
		PWSTRING
		QueryMessageString (
			IN MSGID	MsgId
			);

		VOID
		SetArguments(
			);

		NONVIRTUAL
		BOOLEAN
		Traverse (
			IN		PFSN_DIRECTORY			Directory,
            IN OUT  PPATH                   DestinationPath,
			IN		PFSN_FILTER 			FileFilter,
            IN      PFSN_FILTER             DirectoryFilter,
            IN      BOOLEAN                 CopyDirectoryStreams
			);

		NONVIRTUAL
		BOOLEAN
		UserConfirmedCopy (
			IN	PCFSNODE	FsNode
            );

        NONVIRTUAL
        BOOLEAN
        InitializeExclusionList(
            IN  PWSTRING    ListOfFiles
            );

        NONVIRTUAL
        BOOLEAN
        AddToExclusionList(
            IN  PWSTRING    ExclusionListFileName
            );

        NONVIRTUAL
        BOOLEAN
        IsExcluded(
            IN PCPATH   Path
            );


		//
		//	Paths, switches etc. specified in the command line.
        //

        //
        //  DOS options
        //
		PPATH			_SourcePath;
		PPATH			_DestinationPath;
        PTIMEINFO       _Date;
		BOOLEAN 		_ArchiveSwitch;
        BOOLEAN         _EmptySwitch;
		BOOLEAN 		_ModifySwitch;
		BOOLEAN 		_PromptSwitch;
		BOOLEAN 		_SubdirSwitch;
		BOOLEAN 		_VerifySwitch;
        BOOLEAN         _WaitSwitch;


        //
        //  Windows NT additional options
        //
        BOOLEAN         _ContinueSwitch;            //  Continue if error
        BOOLEAN         _CopyIfOldSwitch;           //  Only newer files
        BOOLEAN         _IntelligentSwitch;         //  Don't ask F or D
        BOOLEAN         _HiddenSwitch;              //  Copy hidden
        BOOLEAN         _ReadOnlySwitch;            //  Overwrite RO
        BOOLEAN         _SilentSwitch;              //  Silent mode
        BOOLEAN         _VerboseSwitch;             //  Verbose mode
        BOOLEAN         _DontCopySwitch;            //  Display only
        BOOLEAN         _StructureOnlySwitch;       //  Structure only
        BOOLEAN         _UpdateSwitch;              //  Apdate existing
        BOOLEAN         _CopyAttrSwitch;            //  Copy attributes
        BOOLEAN         _UseShortSwitch;            //  Use short names

        //
        //  Are we copying to diskette?
        //
        BOOLEAN         _DisketteCopy;

		//
		//	Number of files copied.
		//
		ULONG			_FilesCopied;

		//
		//	Keyboard
		//
		PKEYBOARD		_Keyboard;

		//
		//	Destination file specification. Used to convert source file
		//	names to destination file names.
		//
		PWSTRING		_FileNamePattern;

		//
		//	Indicates if it is ok to remove empty directories.
		//
		BOOLEAN 		_CanRemoveEmptyDirectories;

		//
		//	Indicate if the destination is a file, and what the target
		//	file name should be.
		//
		BOOLEAN 		_TargetIsFile;
        PPATH           _TargetPath;

        //  File name with the exclusion list.
        //
        PSTRING_ARRAY   _ExclusionList;
        PITERATOR       _Iterator;

};

INLINE
VOID
XCOPY::AbortIfCtrlC (
	VOID
	)

/*++

Routine Description:

	Aborts the program if Ctrl-C was hit.

Arguments:

    None.

Return Value:

    None.

Notes:

--*/

{
	if ( _Keyboard->GotABreak() ) {
		exit( EXIT_TERMINATED );
	}
}


#endif // _XCOPY_
