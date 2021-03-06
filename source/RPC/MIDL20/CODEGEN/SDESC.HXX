/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 2000 Microsoft Corporation

 Module Name:
	
	sdesc.hxx

 Abstract:

	Stub descriptor dictionary

 Notes:


 History:

 ----------------------------------------------------------------------------*/
#ifndef __SDESC_HXX__
#define __SDESC_HXX__
/****************************************************************************
 *	include files
 ***************************************************************************/
#include "nulldefs.h"
extern "C"
	{
	#include <stdio.h>
	#include <assert.h>
	#include <string.h>
	}
#include "cgcommon.hxx"
#include "dict.hxx"

/****************************************************************************
 *	local definitions
 ***************************************************************************/
/****************************************************************************
 *	local data
 ***************************************************************************/

/****************************************************************************
 *	externs
 ***************************************************************************/

extern int	CompareSDesc( void *, void *);
extern void	PrintSDesc( void * );

/****************************************************************************/


//
typedef struct _sdesc
	{
	PNAME			pName;
	PNAME			AllocRtnName;
	PNAME			FreeRtnName;
	PNAME			RundownRtnName;
	BOOL			fEmitted;

	void			ResetEmitted()
						{
						fEmitted = FALSE;
						}

	void			MarkAsEmitted()
						{
						fEmitted = TRUE;
						}

	BOOL			IsEmitted()
						{
						return (fEmitted == TRUE);
						}
	} SDESC;
	
// This class manages stub descriptors.

class SDESCMGR	: public Dictionary
	{
private:
public:

						SDESCMGR() : Dictionary()
							{
							}
						
						~SDESCMGR()
							{
								// UNDONE
							}

	// Register an entry.

	SDESC		*	Register( PNAME	AllocRtnName,
							  PNAME	FreeRtnName,
							  PNAME	RundownName );

	virtual
	int 			Compare (pUserType pL, pUserType pR);

	};

#endif // __SDESC_HXX__
