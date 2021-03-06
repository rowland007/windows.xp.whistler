/***
*mkdir.c - OS/2 make directory
*
*	Copyright (c) 1996-2000, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines function _mkdir() - make a directory
*
*Revision History:
*	06-06-89  PHG	Module created, based on asm version
*	03-07-90  GJF	Made calling type _CALLTYPE2 (for now), added #include
*			<cruntime.h>, fixed compiler warnings and fixed the
*			copyright. Also, cleaned up the formatting a bit.
*	03-30-90  GJF	Now _CALLTYPE1.
*	07-24-90  SBM	Removed '32' from API names
*	09-27-90  GJF	New-style function declarator.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Added _CRUISER_ and _WIN32 conditionals.
*	01-16-91  GJF	ANSI naming.
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <direct.h>

/***
*int _mkdir(path) - make a directory
*
*Purpose:
*	creates a new directory with the specified name
*
*Entry:
*	char *path - name of new directory
*
*Exit:
*	returns 0 if successful
*	returns -1 and sets errno if unsuccessful
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _mkdir (
	const char *path
	)
{
	ULONG dosretval;

	/* ask OS to create directory */
#ifdef	_CRUISER_

	dosretval = DOSCREATEDIR((char *)path, 0, 0);

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

        if (!CreateDirectory((LPSTR)path, (LPSECURITY_ATTRIBUTES)NULL))
                dosretval = GetLastError();
        else
                dosretval = 0;

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

	if (dosretval) {
		/* error occured -- map error code and return */
		_dosmaperr(dosretval);
		return -1;
	}

	return 0;
}
