#ifndef STDTYP_H
#define STDTYP_H
/*******************************************************************
    Filename: STDTYP.H
   (c) 2000, ATI Technologies Incorporated.
  $Revision:   1.0  $
      $Date:   31 Jan 1994 11:49:02  $
   $Author:   RWOLFF  $
      $Log:   S:/source/wnt/ms11/miniport/vcs/stdtyp.h  $
 * 
 *    Rev 1.0   31 Jan 1994 11:49:02   RWOLFF
 * Initial revision.
        
           Rev 1.1   03 Sep 1993 14:29:46   RWOLFF
        Type definitions used by Windows NT driver.
        
           Rev 1.1   24 Feb 1993 11:05:20   Ajith_S
        Check for MSDOS and not far as #ifdef far always returns FALSE
        
           Rev 1.0   13 Jan 1993 14:16:12   Robert_Wolff
        Initial revision.
        
      Rev 1.14   15 Apr 2000 16:26:30   Bryan_Sniderman
   Corrected WORD definition from 'unsigned int' to 'unsigned short'
   This will force a WORD to 16-bits (even in HIGH-C)
   
      Rev 1.13   06 Dec 1990 12:02:22   Chris_Brady
   Added support for Metaware HIGHC compiler.  Define P386.
   
      Rev 1.12   04 Jun 1990 15:54:18   Sandy_Lum
   Corrected syntax error
   
      Rev 1.11   17 May 1990 10:02:00   Robin_Davies
   Added TINY, UTINY base types.
   
      Rev 1.10   28 Sep 2000 11:41:14   Robin_Davies
   Made PRIVATE declarator conditional, allowing all PRIVATE labels
   to be published from the command line by adding /DPRIVATE option
   to CL options. Useful for debugging purposes.
   
      Rev 1.9   17 Sep 2000 16:10:16   Robin_Davies
   "define HUGE huge" conflicts with ANSI definition of HUGE in
   MATH.H. Replaced with "define ENORMOUSE huge".
   
      Rev 1.8   15 Sep 2000 16:39:10   Peter Liepa
   added HUGE keyword
   
      Rev 1.7   30 Jun 2000 11:06:52   Peter Michalek
   FALSE , TRUE added
   
      Rev 1.6   28 Jun 2000 15:00:28   Robin_Davies
   Added FLOAT and DOUBLE.
   
      Rev 1.5   28 Jun 2000 14:34:54   Robin_Davies
   Removed double-slash comments.
   
      Rev 1.4   21 Jun 2000 15:34:06   Peter Liepa
   removed hard tabs
   
      Rev 1.3   20 Jun 2000 15:45:20   Peter Liepa
   corrected spelling of "SUCCESS", fixed up line overflows
   
      Rev 1.2   16 Jun 2000  9:51:56   Robin_Davies
   Made headers consistent.

      Rev 1.1   14 Jun 2000 15:27:38   Robin_Davies
   Added OFFSET, SEGMENT macros.

      Rev 1.0   14 Jun 2000  9:20:24   Robin_Davies
   Initial revision.
********************************************************************/

#ifdef DOC
STDTYP.H - Portable types for C programs.

DESCRIPTION
     This file contains standard augmented types for C programs. Types
     are based on and are compatible with Microsoft OS/2 types.
     In addition, several Plum-Hall/Whitesmith standard types
     have been added.

     This header currently supports the Microsoft C 5.1 compiler.
     and the Metaware High C 386 Protected mode Version 1.6  compiler
     by defining P386.
#endif

#ifndef INT_MAX /* Defined in limits.h */
#include <limits.h> /* Ansi type limits, matching augmented types */
#endif
/*
*****************************
    Standard Defines
*****************************
*/
#define YES 1
#define NO 0

/*
 * Some platforms have already defined TRUE and/or FALSE.
 */
#ifndef TRUE
    #define TRUE    (1)     /* Function TRUE  value */
#endif
#ifndef FALSE
    #define FALSE   (0)     /* Function FALSE value */
#endif

#ifndef  P386
#define SUCCESS 0
#define FAIL 1
#endif

/*
 * Non-portable types. Data size references are not needed
 * for Windows NT, since it's a 32 bit operating system.
 * Only define the basic types for platforms (e.g. Windows NT)
 * where they aren't already defined.
 */
#ifndef WIN40

#ifndef pascal
    #define pascal
#endif
#define PASCAL  pascal
#ifndef near
    #define near
#endif
#define NEAR    near
#ifndef MSDOS
    #define far
#endif
#define FAR     far
#ifndef huge
    #define huge
#endif
#define ENORMOUS huge
#define VOID    void

#endif      /* WIN40 */

typedef unsigned char BYTE;     /* 8 bits unsigned  */
typedef unsigned short WORD;    /* 16 bits unsigned */

/*
**************************
    BASE TYPES
**************************
*/
typedef char CHAR;              /* At least 8 bits, sign indeterminate */
typedef short SHORT;            /* At least 16 bits signed */
typedef long LONG;              /* At least 32 bits signed */
typedef float FLOAT;            /* Floating point, unspecified precision */
typedef double DOUBLE;          /* Floating point, unspecified precision */

typedef unsigned char UCHAR;    /* At least 8 bits, unsigned */
typedef signed char SCHAR;      /* At least 8 bits, signed */
typedef unsigned short USHORT;  /* At least 16 bits, unsigned */
typedef unsigned long ULONG;    /* At least 32 bits, signed */

#ifdef MSDOS
#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif // NULL
#endif

typedef unsigned char UTINY;    /* Scalar, at least 8 bits , unsigned */
typedef char TINY;              /* Scalar, at least 8 bits, signed */

/*
****************************
    ARTIFICIAL TYPES
****************************
*/
#ifndef PRIVATE
   #define PRIVATE static           /* Local functions and variables */
#endif

typedef int METACHAR;            /* Sufficient to hold any character and EOF */
typedef int BOOL;                /* Most efficient Boolean,
                                         compare against zero only! */
typedef char TBOOL;              /* Smallest boolean, e.g. for use in arrays. 
                                        Compare against zero only */
typedef unsigned int BITS;       /* At least 16 bits, used for bit manipulation */
typedef unsigned char TBITS;     /* At least 8 bits, used for bit manipulation */
typedef unsigned long int LBITS; /* At least 32 bits, used for bit manipulation */

typedef unsigned int SIZE_T;    /* sufficent for sizeof(anything)
                                 e.g. malloc size. Equivalent to ansi size_t */

#if     defined(__LARGE__) || defined(__HUGE__) || defined(__COMPACT__)
typedef long    PTRDIFF_T;      /* difference between pointers.
                                         Equivalent to ansi ptrdiff_t */
#else
typedef int     PTRDIFF_T;      /* difference between pointers. 
                                        Equivalent to ansi ptrdiff_t */
#endif

#ifdef MSDOS
typedef BOOL BOOLEAN;

typedef struct _PHYSICAL_ADDRESS {
        ULONG LowPart;
        LONG HighPart;
} PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;


//
// The following structure is used to define access ranges. The ranges are
// used to indicate which ports and memory adresses are being used by the
// card.
//

typedef struct _VIDEO_ACCESS_RANGE {

    //
    // Indicates the starting memory address or port number of the range.
    // This values should be stored before being transformed by
    // VideoPortGetDeviceBase() which returns the logical address that must
    // be used by the miniport driver when referencing physical addresses.
    //

    PHYSICAL_ADDRESS RangeStart;

    //
    // Indicates the length in bytes, or number of ports in the range. This
    // value should indicate the range actually decoded by the adapter. For
    // example, if the adapter uses 7 registers but responds to eight, the
    // RangeLength should be set to 8.

    ULONG RangeLength;

    //
    // Indicates if the range is in IO space (TRUE) or in memory space (FALSE).
    //

    UCHAR RangeInIoSpace;

    //
    // Indicates if the range should be visible by the Windows display driver.
    // This is done so that a Windows display driver can access certain
    // video ports directly. This will only be allowed if the caller has the
    // required privileges (is a trusted subsystem) to access the range.
    //
    // Synchronization of access to ports or memory in the range must be
    // done explicitly by the miniport driver and the user mode process so
    // that they both don't try to program the device simultaneously.
    //
    // Non visible ranges should include video memory, ROM addresses, etc.
    // which are not required to program the device for output purposes.
    //
    //

    UCHAR RangeVisible;

    //
    // This field determines if the range can be shared with another device.
    // The rule should be applied as follow.
    //
    // - If the range of memory or IO ports should be "owned" by this driver,
    //   and that any other driver trying to access this range may cause
    //   a problem, FALSE should be returned.
    //
    // - If the range can be shared with another co-operating device driver,
    //   then the share field should be set to TRUE.
    //
    // As a guideline, the VGA miniport driver will claim all of its resources
    // as shareable so that it can be used as a VGA compatible device with
    // any other driver (such as an S3 or XGA.
    //
    // Super VGA miniport drivers that implement all the VGA functionality
    // (declared in the Registry as VGACOMPATIBLE=1) should claim the range
    // as non-shareable since they don't want the VGA to run at the same time.
    //
    // Miniports for cards such as an S3 or XGA that have an XGA on the board
    // but do not implement the VGA functionality will run with the VGA
    // miniport loaded and should therefore claim all the resources shared
    // with the VGA as shareable.
    //
    // Miniports for cards that work with a pass-through and that can be
    // connected to any VGA/SVGA card should not be using any VGA ports or
    // memory ranges ! ... but if they do they should not claim those
    // resources since they will cause a conflict in the system because the
    // SVGA cards will have claimed them as non-shareable ...
    //

    UCHAR RangeShareable;

} VIDEO_ACCESS_RANGE, *PVIDEO_ACCESS_RANGE;

#endif

/*
****************************
     POINTERS
****************************
*/

#ifdef MSDOS
typedef UCHAR *PUCHAR;                 /* pointer to UCHAR */
typedef USHORT *PUSHORT;               /* pointer to USHORT */
#endif

typedef void *PVOID;                   /* Generic untyped pointer */

typedef int (*PFUNC)();                /* Pointer to Function (model dependent) */

#ifndef  P386
typedef int (pascal far *PPFN)();      /* Generic pointer to Far Pascal Function */
typedef int (far *PCFN)();             /* Generic pointer to Far C Function */
#endif

/*
****************************
   Useful Macros
****************************
*/

#define FOREVER for(;;)
#define NOTHING /**/        /* e.g. while (condition) NOTHING */

/* Create untyped far pointer from selector and offset */
#define MAKEP(sel, off)     ((PVOID)MAKEULONG(off, sel))

/* Extract selector or offset from far pointer. e.g. OFFSET(&x) */
#define OFFSET(p)           LOWORD((VOID FAR *) (p))
#define SEGMENT(p)          HIWORD((VOID FAR *) (p))

/* Cast any variable to an instance of the specified type. */
#define MAKETYPE(v, type)   (*((type *)&v))

/* Calculate the byte offset of a field in a structure of type type. */
#define FIELDOFFSET(type, field)    ((SHORT)&(((type *)0)->field))

/* Combine l & h to form a 32 bit quantity. */
#define MAKEULONG(l, h)  ((ULONG)(((USHORT)(l)) | ((ULONG)((USHORT)(h))) << 16))
#define MAKELONG(l, h)   ((LONG)MAKEULONG(l, h))

/* Combine l & h to form a 16 bit quantity. */
#define MAKEWORD(l, h) (((WORD)(l)) | ((WORD)(h)) << 8)
#define MAKESHORT(l, h)  ((SHORT)MAKEUSHORT(l, h))

/* Extract high and low order parts of 16 and 32 bit quantity */
#define LOBYTE(w)       ((BYTE)(w))
#define HIBYTE(w)       ((BYTE)(((WORD)(w) >> 8) & 0xff))
#define LOWORD(l)     ((WORD)(l))
#define HIWORD(l)     ((WORD)(((ULONG)(l) >> 16) & 0xffff))

#endif /* defined STDTYP_H */
