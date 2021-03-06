/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    dhcpcli.h

Abstract:

    Private header file for the client end of the DHCP server service.

Author:

    Madan Appiah (madana) 10-Sep-1993
    Manny Weiser (mannyw) 11-Aug-2000

Revision History:

--*/

//
//  NT public header files
//

#include "windows.h"
#include "rpc.h"

//
//  DHCP public header files
//

#include "dhcp.h"


//
//  Local RPC built files
//

#include "dhcp_cli.h"

#define USE_TCP

