/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 2000  Microsoft Corporation

Module Name:

    OsverP.h

Abstract:


Author:

    Scott B. Suhy (ScottSu) 5/6/93

Environment:

    User Mode

--*/

#if ! defined( _OSVER_ )

#define __OSVER_

BOOL Os(void);

DWORD 
GetCSDVersion (
               void
              );

#endif // _OSVER_

BOOL OsVer(void);
