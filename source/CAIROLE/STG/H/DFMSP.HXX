//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 2000 - 2000
//
//  File:       dfmsp.hxx
//
//  Contents:   DocFile and MultiStream shared private definitions
//
//  History:    01-Apr-92       DrewB   Created
//
//---------------------------------------------------------------

#ifndef __DFMSP_HXX__
#define __DFMSP_HXX__

#ifndef REF
#include <debnot.h>
#else
#include <ref.hxx>
#endif //!REF
#include <wchar.h>
#ifndef REF
#include <valid.h>
#endif //!REF
#include <string.h>

// Target-dependent things

//
//      x86 16-bit build optimizations
//
//      Some function parameters are always stack based pointers,
//      so we can let the compiler use near addressing via ss by
//      declaring the parameter stack based.
//

#ifndef REF
#if defined(_M_I286)
#define STACKBASED __based(__segname("_STACK"))
#else
#define STACKBASED
#endif
#else
#define STACKBASED
#endif //!REF

//
//      x86 16-bit retail build optimizations
//
//      For the retail build, we group the code segments,
//      allowing us to make many calls near.
//

#ifndef REF
#if defined(_M_I286) && DBG == 0 && defined(USE_NEAR)
#define DFBASED
#define DIR_CLASS __near
#define FAT_CLASS __near
#define MSTREAM_CLASS __near
#define VECT_CLASS __near
#else
#define DFBASED
#define DIR_CLASS
#define FAT_CLASS
#define MSTREAM_CLASS
#define VECT_CLASS
#endif
#else
#define DFBASED
#define DIR_CLASS
#define FAT_CLASS
#define MSTREAM_CLASS
#define VECT_CLASS
#endif //!REF

#ifndef REF
// Compiler pragma define
// Currently defined for C7 and C8
// Unused as of 1/18/93
#if ((_MSC_VER == 700) || (_MSC_VER == 800))
#define MS_COMPILER
#endif
#endif //!REF

// Segmented memory model definitions
#ifndef REF
#if !defined(HUGEP)
#if defined(_M_I286)
#define HUGEP __huge
#else
#define HUGEP
#endif
#endif
#else
#define HUGEP
#endif //!REF

#ifndef LISet32
#define LISet32(li, v) \
    ((li).HighPart = ((LONG)(v)) < 0 ? -1 : 0, (li).LowPart = (v))
#endif
#ifndef ULISet32
#define ULISet32(li, v) ((li).HighPart = 0, (li).LowPart = (v))
#endif
#define LISetLow(li, v) ((li).LowPart = (v))
#define LISetHigh(li, v) ((li).HighPart = (v))
#define ULISetLow(li, v) ((li).LowPart = (v))
#define ULISetHigh(li, v) ((li).HighPart = (v))
#define LIGetLow(li) ((li).LowPart)
#define LIGetHigh(li) ((li).HighPart)
#define ULIGetLow(li) ((li).LowPart)
#define ULIGetHigh(li) ((li).HighPart)

// Fast safe increment/decrement
#ifndef REF
#ifdef FLAT

//  Win32 specific functions
//  Should use #ifdef WIN32 instead of FLAT, but there isn't a WIN32

#define AtomicInc(lp) InterlockedIncrement(lp)
#define AtomicDec(lp) InterlockedDecrement(lp)

#else
#define AtomicInc(lp) (++*(lp))
#define AtomicDec(lp) (--*(lp))

#endif
#else
#define AtomicInc(lp) (++*(lp))
#define AtomicDec(lp) (--*(lp))

#endif //!REF

// Switchable ANSI/Unicode support for TCHAR
// Conversion routines assume null termination before max characters
#ifdef UNICODE

#define ATOT(a, t, max) mbstowcs(t, a, max)
#define TTOA(t, a, max) wcstombs(a, t, max)
#define WTOT(w, t, max) wcscpy(t, w)
#define TTOW(t, w, max) wcscpy(w, t)

#define tcscpy(t, f) wcscpy(t, f)
#define tcslen(t) wcslen(t)

#define TSTR(s) L##s

// printf format string
#define TFMT "%ws"

#else

#define ATOT(a, t, max) strcpy(t, a)
#define TTOA(t, a, max) strcpy(a, t)
#define WTOT(w, t, max) wcstombs(t, w, max)
#define TTOW(t, w, max) mbstowcs(w, t, max)

#define tcscpy(t, f) strcpy(t, f)
#define tcslen(t) strlen(t)

#define TSTR(s) s

// printf format string
#define TFMT "%s"

#endif

// Switchable ANSI/Unicode support for OLECHAR
// Conversion routines assume null termination before max characters
#ifdef WIN32

#define OLEWIDECHAR

#ifndef OLECHAR
#define LPOLESTR    	LPWSTR
#define LPCOLESTR   	LPCWSTR
#define OLECHAR     	WCHAR
#define OLESTR(str)  	L##str
#endif //OLECHAR

#define _OLESTDMETHODIMP STDMETHODIMP
#define _OLEAPIDECL STDAPI
#define _OLERETURN(sc) ResultFromScode(sc)
#define _OLEAPI(name) name

#define ATOOLE(a, t, max) mbstowcs(t, a, max)
#define OLETOA(t, a, max) wcstombs(a, t, max)
#define WTOOLE(w, t, max) wcscpy(t, w)
#define OLETOW(t, w, max) wcscpy(w, t)

#define olecscpy(t, f) wcscpy(t, f)
#define olecslen(t) wcslen(t)

#define OLESTR(s) L##s

// printf format string
#define OLEFMT "%ws"

#else

#ifndef OLECHAR
#define LPOLESTR    	LPSTR
#define LPCOLESTR   	LPCSTR
#define OLECHAR     	char
#define OLESTR(str)	str
#endif

#define _OLESTDMETHODIMP SCODE
#define _OLEAPIDECL SCODE
#define _OLERETURN(sc) (sc)
#define _OLEAPI(name) name##W
#define ATOOLE(a, t, max) strcpy(t, a)
#define OLETOA(t, a, max) strcpy(a, t)
#define WTOOLE(w, t, max) wcstombs(t, w, max)
#define OLETOW(t, w, max) mbstowcs(w, t, max)

#define olecscpy(t, f) strcpy(t, f)
#define olecslen(t) strlen(t)

#define OLESTR(s) s

// printf format string
#define OLEFMT "%s"

#endif

// Security is only enabled for Cairo builds
#if WIN32 == 300
typedef LPSECURITY_ATTRIBUTES LPSTGSECURITY;
#else
typedef DWORD LPSTGSECURITY;
#endif

// In Cairo, the reserved field in STATSTG is dwStgFmt
#if WIN32 == 300
#define STATSTG_reserved dwStgFmt
#else
#define STATSTG_reserved reserved
#endif

// For NT 1.0a OLE we need to use based pointers for shared memory objects
// since they might not be mapped at the same address in every process
#if WIN32 == 100 || WIN32 > 200

#define DFBASEPTR pvDfSharedMemBase

extern void *DFBASEPTR;

void DfInitSharedMemBase(void);

#pragma warning(error: 4795 4796)

#undef DFBASED
#define DFBASED __based(DFBASEPTR)

#define P_TO_BP(t, p) ((t)((p) ? (int)(t)(char *)(p) : 0))

#define BP_TO_P(t, bp) (t)((bp) != 0 ? (bp) : 0)

#else
#define DfInitSharedMemBase()
#endif

//----------------------------------------------------------------------------

// The name of this function might change, so encapsulate it
#define DfGetScode(hr) GetScode(hr)

#ifndef REF
// Buffer/pointer validation macros
#define AssertMsg(s) _Win4Assert(__FILE__, __LINE__, s)
#endif //!REF

#ifndef REF
// MAC - We use Windows functions if available
#if (WIN32 == 100 || WIN32 >= 300)
#define IsValidHugePtrIn(pv, n)  (((pv) == NULL) || !IsBadHugeReadPtr(pv, n))
#define IsValidHugePtrOut(pv, n) (!IsBadHugeWritePtr(pv, n))
#else
#define IsValidHugePtrIn(pv, n)  1
#define IsValidHugePtrOut(pv, n) ((pv) != NULL)
#endif
#endif //!REF

#ifndef REF
#define ValidateBuffer(pv, n) \
    (((pv) == NULL || !IsValidPtrIn(pv, n)) ? STG_E_INVALIDPOINTER : S_OK)

#define ValidatePtrBuffer(pv) \
    ValidateBuffer(pv, sizeof(void *))

#define ValidateHugeBuffer(pv, n) \
    (((pv) == NULL || !IsValidHugePtrIn(pv, n)) ? STG_E_INVALIDPOINTER : S_OK)

#define ValidateOutBuffer(pv, n) \
    (!IsValidPtrOut(pv, n) ? STG_E_INVALIDPOINTER : S_OK)

#define ValidateOutPtrBuffer(pv) \
    ValidateOutBuffer(pv, sizeof(void *))

#define ValidateHugeOutBuffer(pv, n) \
    (!IsValidHugePtrOut(pv, n) ? STG_E_INVALIDPOINTER : S_OK)

#define ValidateIid(riid) \
    (!IsValidIid(riid) ? STG_E_INVALIDPOINTER : S_OK)

#define ValidateInterface(punk, riid) \
    (!IsValidInterface(punk) ? STG_E_INVALIDPOINTER : S_OK)

#if (WIN32 == 100 || WIN32 >= 300)
#define ValidateWcs(pwcs, cwcMax) \
    (IsBadStringPtrW(pwcs, cwcMax) ? STG_E_INVALIDPOINTER : S_OK)
#else
// OLE should provide a helper function for this
#define ValidateWcs(pwcs, cwcMax) \
    (!IsValidPtrIn(pwcs, sizeof(WCHAR)) ? STG_E_INVALIDPOINTER : S_OK)
#endif

#if defined(_WINDOWS_)
# define ValidateSz(psz, cchMax) \
    (IsBadStringPtrA(psz, cchMax) ? STG_E_INVALIDPOINTER : S_OK)
#elif defined(_INC_WINDOWS)
# define ValidateSz(psz, cchMax) \
    (IsBadStringPtr(psz, cchMax) ? STG_E_INVALIDPOINTER : S_OK)
#else
// MAC - OLE doesn't provide sufficient helper functions
# define ValidateSz(psz, cchMax) \
    (!IsValidPtrIn(psz, 1) ? STG_E_INVALIDPOINTER : S_OK)
#endif

#if defined(OLEWIDECHAR)
#if defined(_WINDOWS_)
#define ValidateNameW(pwcs, cchMax) \
    (IsBadStringPtrW(pwcs, cchMax) ? STG_E_INVALIDNAME : S_OK)
#else
#define ValidateNameW(pwcs, cchMax) \
    (!IsValidPtrIn(pwcs, sizeof(WCHAR)) ? STG_E_INVALIDNAME : S_OK)
#endif
#else
//  For non-Unicode builds, we verify all names before converting them
//  to wide character names, so there's no need to recheck.
# define ValidateNameW(pwcs, cchMax) \
    S_OK
#endif

#if defined(_WINDOWS_)
# define ValidateNameA(psz, cchMax) \
    (IsBadStringPtrA(psz, cchMax) ? STG_E_INVALIDNAME : S_OK)
#elif defined(_INC_WINDOWS)
# define ValidateNameA(psz, cchMax) \
    (IsBadStringPtr(psz, cchMax) ? STG_E_INVALIDNAME : S_OK)
#else
# define ValidateNameA(psz, cchMax) \
    (!IsValidPtrIn(psz, 1) ? STG_E_INVALIDNAME : S_OK)
#endif

#else
#define ValidateBuffer(pv, n) S_OK
#define ValidatePtrBuffer(pv) S_OK
#define ValidateHugeBuffer(pv, n) S_OK
#define ValidateOutBuffer(pv, n) S_OK
#define ValidateOutPtrBuffer(pv) S_OK
#define ValidateHugeOutBuffer(pv, n) S_OK
#define ValidateIid(riid) S_OK
#define ValidateInterface(punk,riid) S_OK
#define ValidateWcs(pwcs, cwcMax) S_OK
#define ValidateSz(psz, cchMax) S_OK
#define ValidateNameW(pwcs, cchMax) S_OK
#define ValidateNameA(psz, cchMax) S_OK
#endif //!REF

// Enumeration for Get/SetTime
enum WHICHTIME
{
    WT_CREATION,
    WT_MODIFICATION,
    WT_ACCESS
};

// Time type
typedef FILETIME TIME_T;

// Signature for transactioning
typedef DWORD DFSIGNATURE;
#define DF_INVALIDSIGNATURE ((DFSIGNATURE)-1)

// Convenience macros for signature creation
#define LONGSIG(c1, c2, c3, c4) \
    (((ULONG) (BYTE) (c1)) | \
    (((ULONG) (BYTE) (c2)) << 8) | \
    (((ULONG) (BYTE) (c3)) << 16) | \
    (((ULONG) (BYTE) (c4)) << 24))

#ifndef min
#define min(a, b) ((a)<(b) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((a)>(b) ? (a) : (b))
#endif

SCODE DfGetTOD(TIME_T *ptm);

// Shared signature validation routine
SCODE CheckSignature(BYTE *pb);

// Docfile locally unique identity
// Every entry in a multistream has a LUID generated and stored for it
typedef DWORD DFLUID;
#define DF_NOLUID 0

#ifndef OLEWIDECHAR
typedef WCHAR **SNBW;

typedef struct
{
    WCHAR *pwcsName;
    DWORD type;
    ULARGE_INTEGER cbSize;
    FILETIME mtime;
    FILETIME ctime;
    FILETIME atime;
    DWORD grfMode;
    DWORD grfLocksSupported;
    CLSID clsid;
    DWORD grfStateBits;
    DWORD reserved;
} STATSTGW;

#else
typedef SNB SNBW;
typedef STATSTG STATSTGW;
#endif

#define CBSTORAGENAME (CWCSTORAGENAME*sizeof(WCHAR))

// A Unicode case-insensitive compare
// No such thing really exists so we use our own
#define dfwcsnicmp(wcsa, wcsb, len) wcsnicmp(wcsa, wcsb, len)

#include <dfname.hxx>

// Fast, fixed space iterator structure
struct SIterBuffer
{
    CDfName dfnName;
    DWORD type;
};

//SID is a Stream Identifier
#ifndef REF
#ifdef FLAT
#define SID DFSID
#endif
#endif //!REF
typedef ULONG SID;

// IsEntry entry information
struct SEntryBuffer
{
    DFLUID luid;
    DWORD  dwType;
    SID    sid;
};

// Destroy flags
#define DESTROY_FROM_HANDLE     0
#define DESTROY_FROM_ENTRY      1
#define DESTROY_FROM            0x01
#define DESTROY_SELF            0x40
#define DESTROY_RECURSIVE       0x80

#define DESTROY_HANDLE          (DESTROY_FROM_HANDLE | DESTROY_SELF)
#define DESTROY_ENTRY           (DESTROY_FROM_ENTRY | DESTROY_SELF)

// Root startup flags
#define RSF_OPEN                0x00
#define RSF_CONVERT             0x01
#define RSF_TRUNCATE            0x02
#define RSF_CREATE              0x04
#define RSF_DELAY               0x08
#define RSF_DELETEONRELEASE     0x10
#define RSF_OPENCREATE          0x20

#define RSF_CREATEFLAGS (RSF_CREATE | RSF_TRUNCATE | RSF_OPENCREATE)

// Stream copy buffer size
ULONG const STREAMBUFFERSIZE = 8192;

// ILockBytes copy buffer size
ULONG const LOCKBYTESBUFFERSIZE = 16384;

// Docfile flags for permissions and other information kept
// on streams and docfiles
typedef WORD DFLAGS;

#define DF_TRANSACTEDSELF       0x0001

#define DF_TRANSACTED           0x0002
#define DF_DIRECT               0x0000

#define DF_INDEPENDENT          0x0004
#define DF_DEPENDENT            0x0000

#define DF_COMMIT               0x0008
#define DF_ABORT                0x0000

#define DF_INVALID              0x0010

#define DF_REVERTED             0x0020
#define DF_NOTREVERTED          0x0000

#define DF_READ                 0x0040
#define DF_WRITE                0x0080
#define DF_READWRITE            (DF_READ | DF_WRITE)

#define DF_DENYREAD             0x0100
#define DF_DENYWRITE            0x0200
#define DF_DENYALL              (DF_DENYREAD | DF_DENYWRITE)

#define DF_PRIORITY             0x0400
#define DF_CREATE               0x0800
#define DF_CACHE                0x1000
#define DF_NOUPDATE             0x2000

// Shift required to translate from DF_READWRITE to DF_DENYALL
#define DF_DENIALSHIFT          2

// Permission abstraction macros
// These only work with DF_* flags
#define P_READ(f)       ((f) & DF_READ)
#define P_WRITE(f)      ((f) & DF_WRITE)
#define P_READWRITE(f)  (((f) & (DF_READ | DF_WRITE)) == (DF_READ | DF_WRITE))
#define P_DENYREAD(f)   ((f) & DF_DENYREAD)
#define P_DENYWRITE(f)  ((f) & DF_DENYWRITE)
#define P_DENYALL(f)    (((f) & (DF_DENYREAD | DF_DENYWRITE)) == \
			 (DF_DENYREAD | DF_DENYWRITE))
#define P_PRIORITY(f)   ((f) & DF_PRIORITY)
#define P_TRANSACTED(f) ((f) & DF_TRANSACTED)
#define P_DIRECT(f)     (!P_TRANSACTED(f))
#define P_INDEPENDENT(f) ((f) & DF_INDEPENDENT)
#define P_DEPENDENT(f)  (!P_INDEPENDENT(f))
#define P_TSELF(f)      ((f) & DF_TRANSACTEDSELF)
#define P_INVALID(f)    ((f) & DF_INVALID)
#define P_REVERTED(f)   ((f) & DF_REVERTED)
#define P_COMMIT(f)     ((f) & DF_COMMIT)
#define P_ABORT(f)      (!P_COMMIT(f))
#define P_CREATE(f)     ((f) & DF_CREATE)
#define P_CACHE(f)      ((f) & DF_CACHE)
#define P_NOUPDATE(f)   ((f) & DF_NOUPDATE)

// Translation functions
DFLAGS ModeToDFlags(DWORD const dwModeFlags);
DWORD DFlagsToMode(DFLAGS const df);

// Flags for what state has been dirtied
#define DIRTY_CREATETIME  0x0001
#define DIRTY_MODIFYTIME  0x0002
#define DIRTY_ACCESSTIME  0x0004
#define DIRTY_CLASS       0x0008
#define DIRTY_STATEBITS   0x0010
#ifdef PROPS
#define DIRTY_PROPTYPE    0x0020
#endif

// Allow text in asserts
#define aMsg(s) ((char *)(s) != NULL)

// Indicate that something is a property value
// This must not conflict with official STGTY_* flags
#define STGTY_PROPFLAG 0x80
#define STGTY_REAL (STGTY_STORAGE | STGTY_STREAM | STGTY_LOCKBYTES)

#ifdef PROPS
#define REAL_STGTY(f) ((f) & STGTY_REAL)
#else
#define REAL_STGTY(f) (f)
#endif

typedef WORD DFPROPTYPE;
#ifdef PROPS
typedef VARIANT DFPROPVAL;
#endif

// Buffer management
#define CB_LARGEBUFFER 32768
#define CB_PAGEBUFFER 4096
#define CB_SMALLBUFFER 512

extern SCODE GetBuffer(USHORT cbMin, USHORT cbMax, BYTE **ppb,
                       USHORT *pcbActual);
extern void GetSafeBuffer(USHORT cbMin, USHORT cbMax, BYTE **ppb,
                          USHORT *pcbActual);
extern void FreeBuffer(BYTE *pb);

#include <dfmem.hxx>

#ifndef REF
#define DfAllocWC(cwc, ppwcs) (*ppwcs = (WCHAR *)\
         TaskMemAlloc((cwc)*sizeof(WCHAR)),\
          (*ppwcs != NULL) ? S_OK: STG_E_INSUFFICIENTMEMORY)
#else
#define DfAllocWC(cwc, ppwcs) (*ppwcs = new WCHAR[cwc],\
        (*ppwcs != NULL) ? S_OK: STG_E_INSUFFICIENTMEMORY)
#endif //!REF

#define DfAllocWCS(pwcs, ppwcs) DfAllocWC(wcslen(pwcs)+1, ppwcs)

#ifndef WIN32
#define STG_SCODE(err) \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, SCODE_CODE(err))
#else
SCODE Win32ErrorToScode(DWORD dwErr);
#define STG_SCODE(err) Win32ErrorToScode(err)

#endif

#endif // #ifndef __DFMSP_HXX__
