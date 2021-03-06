/*++

Copyright (c) 2000  Microsoft Corporation

Module Name:

    conrqust.c

Abstract:

    This module contains the handler for console requests.

Author:

    Avi Nathan (avin) 17-Jul-2000

Environment:

    User Mode Only

Revision History:

    Ellen Aycock-Wright (ellena) 15-Sept-2000 Modified for POSIX

--*/

#define WIN32_ONLY
#include "psxses.h"
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <errno.h>

//
// ServeConRequest -- perform a console request
//
//	pReq - the request message
//	pStatus - returned errno, 0 for success
//
// returns:  TRUE (reply to the caller)
//

BOOL
ServeConRequest(
	IN PSCCONREQUEST pReq,
	OUT PDWORD pStatus
	)
{
	DWORD  IoLengthDone;
	DWORD  Rc;
	HANDLE h;
	int fd;
	int error;

	*pStatus = 0;

	//
	// If the KbdThread has not been started, we do IO in the
	// conventional way.  Since starting the thread depends on the
	// user setting an environment variable, this is a backward-
	// compatibility mode.
	//

	switch (pReq->Request) {
	case ScReadFile:
		fd = (int)pReq->d.IoBuf.Handle;

		if (DoTrickyIO && _isatty(fd)) {
			IoLengthDone = TermInput(hConsoleInput,
				PsxSessionDataBase,
				(unsigned int)pReq->d.IoBuf.Len,
				pReq->d.IoBuf.Flags,
				&error
				);
		} else {
			IoLengthDone = _read(fd,
	                        PsxSessionDataBase,
	                        (unsigned int)pReq->d.IoBuf.Len);
			if (-1 == IoLengthDone) {
				error = EBADF;
			}
		}

		pReq->d.IoBuf.Len = IoLengthDone;

		if (IoLengthDone == -1) {
			*pStatus = error;
		}
		break;

	case ScWriteFile:
		fd = (int)pReq->d.IoBuf.Handle;

		if (DoTrickyIO && _isatty(fd)) {
			IoLengthDone = TermOutput(hConsoleOutput,
				(LPSTR)PsxSessionDataBase,
				(DWORD)pReq->d.IoBuf.Len);
		} else {
			// not a tty.

			IoLengthDone = _write(fd, PsxSessionDataBase,
                                (unsigned int)pReq->d.IoBuf.Len);
		}
		pReq->d.IoBuf.Len = IoLengthDone;
		if (-1 == IoLengthDone) {
			*pStatus = EBADF;
		}
		break;

	case ScKbdCharIn:
		Rc = GetPsxChar(&pReq->d.AsciiChar);
		break;

	case ScIsatty:
		if (!DoTrickyIO) {
			// then it's never a tty.
			pReq->d.IoBuf.Len = 0;
			break;
		}
		pReq->d.IoBuf.Len = (_isatty((int)pReq->d.IoBuf.Handle) != 0);
		break;

	case ScOpenFile:
		if (pReq->d.IoBuf.Flags == _O_RDONLY) {
			fd = _open("CONIN$", _O_RDWR);
		} else {
			fd = _open("CONOUT$", _O_RDWR);
		}
		pReq->d.IoBuf.Handle = (HANDLE)fd;
		break;

	case ScCloseFile:
#if 0
//
// This code causes the keybd thread to fail ReadConsole() with
// STATUS_INVALID_HANDLE.
//
		_close((int)pReq->d.IoBuf.Handle);
#endif
		break;

	default:
		*pStatus = EINVAL;
	}

	return TRUE;
}
