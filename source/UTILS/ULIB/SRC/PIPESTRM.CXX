/*++

Copyright (c) 2000	Microsoft Corporation

Module Name:

	pipestrm.cxx

Abstract:

	This module contains the definitions of the member functions
	of PIPE_STREAM class.

Author:

	Jaime Sasson (jaimes) 24-Mar-2000

Environment:

	ULIB, User Mode


--*/

#include <pch.cxx>

#define _ULIB_MEMBER_

#include "ulib.hxx"
#include "stream.hxx"
#include "bufstrm.hxx"
#include "pipestrm.hxx"

#define BUFFER_SIZE 4*1024


DEFINE_CONSTRUCTOR ( PIPE_STREAM, BUFFER_STREAM );


DEFINE_CAST_MEMBER_FUNCTION( PIPE_STREAM );


PIPE_STREAM::~PIPE_STREAM (
	)

/*++

Routine Description:

	Destroy a PIPE_STREAM.

Arguments:

	None.

Return Value:

	None.

--*/

{
}


BOOLEAN
PIPE_STREAM::Initialize(
	IN HANDLE		Handle,
	IN STREAMACCESS Access
	)

/*++

Routine Description:

	Initializes a PIPE_STREAM object.

Arguments:

	Handle - Handle to the anonymous pipe.

	Access - Access allowed to the stream.


Return Value:

	BOOLEAN - Indicates if the initialization succeeded.


--*/


{
	if( ( Access == READ_ACCESS ) || ( Access == WRITE_ACCESS ) ) {
		_PipeHandle = Handle;
		_Access = Access;
		_EndOfFile = FALSE;
		return( BUFFER_STREAM::Initialize( BUFFER_SIZE ) );
	} else {
		return( FALSE );
	}
}


BOOLEAN
PIPE_STREAM::EndOfFile(
	) CONST

/*++

Routine Description:

	Informs the caller if end of file has occurred. End of file happens
	when all bytes were read from the pipe (in the case of anonymous
	pipe, "end of file" happens when ReadFile returns STATUS_END_OF_FILE).

Arguments:

	None.

Return Value:

	A boolean value that indicates if end of file was detected.


--*/


{
	return( _EndOfFile );
}




BOOLEAN
PIPE_STREAM::FillBuffer(
	IN	PBYTE	Buffer,
	IN	ULONG	BufferSize,
	OUT PULONG	BytesRead
	)

/*++

Routine Description:

	Fills a buffer with bytes read from the pipe, if the pipe has
	READ_ACCESS.
	Returns FALSE if the pipe has WRITE_ACCESS.

Arguments:

	Buffer - Buffer where the bytes are to be stored.

	BufferSize - Size of the buffer.

	BytesRead - Pointer to the variable that will contain the number of bytes
				put in the buffer.


Return Value:

	BOOLEAN - Indicates if the operation succeeded.


--*/

{
	BOOLEAN Result;
	PBYTE	p;

	Result = FALSE;
	if( _Access == READ_ACCESS ) {
		Result = ReadFile( _PipeHandle,
						   Buffer,
						   BufferSize,
						   BytesRead,
						   NULL );
        //
        //  no bytes read means end of file (according to markl)
        //
		if( *BytesRead == 0 ) {
			_EndOfFile = TRUE;
		} else {
			p = (PBYTE)Buffer + *BytesRead;
			*p++ = '\0';
			*p	 = '\0';
		}
	}
	return( Result );
}



STREAMACCESS
PIPE_STREAM::QueryAccess(
	) CONST

/*++

Routine Description:

	Returns the type of access of the pipe stream

Arguments:

	None.

Return Value:

	The stream access.


--*/


{
	return( _Access );
}



HANDLE
PIPE_STREAM::QueryHandle(
	) CONST

/*++

Routine Description:

	Returns the file handle

Arguments:

	None.

Return Value:

	The file handle.


--*/


{
	return( _PipeHandle );
}
