;/*++ BUILD Version: 0001    // Increment this if a change has global effects
;
;Copyright (c) 1990  Microsoft Corporation
;
;Module Name:
;
;    msg.h
;
;Abstract:
;
;    This file contains the message definitions for the Win32 Compact
;    utility.
;
;Author:
;
;    Gary Kimura        [garyki]        13-Jan-1994
;
;Revision History:
;
;--*/
;

MessageId=0001 SymbolicName=COMPACT_OK
Language=English
[OK]
.

MessageId=0002 SymbolicName=COMPACT_ERR
Language=English
[ERR]
.

MessageId=0006 SymbolicName=COMPACT_LIST_CDIR
Language=English

 Listing of %1 [Compress new files]

.

MessageId=0007 SymbolicName=COMPACT_LIST_UDIR
Language=English

 Listing of %1 [Do not compress new files]

.

MessageId=0008 SymbolicName=COMPACT_LIST_SUMMARY
Language=English

Of %1 files within %2 directories
%3 are compressed and %4 and not compressed.
%5 bytes are being used to store %6 total bytes of data.
The data occupies %7%% of its uncompressed size.

.

MessageId=0009 SymbolicName=COMPACT_COMPRESS_DIR
Language=English

 Setting the directory %1 to compress new files %0

.

MessageId=0010 SymbolicName=COMPACT_COMPRESS_CDIR
Language=English

 Compress files within %1 [Compress new files]

.

MessageId=0011 SymbolicName=COMPACT_COMPRESS_UDIR
Language=English

 Compress files within %1 [Do not compress new files]

.

MessageId=0012 SymbolicName=COMPACT_COMPRESS_SUMMARY
Language=English

%1 files within %2 directories were compressed.
%3 bytes are being used to store %4 total bytes of data.
The data occupies %5%% of its uncompressed size.

.


MessageId=0013 SymbolicName=COMPACT_UNCOMPRESS_DIR
Language=English

 Setting the directory %1 to not compress new files %0

.

MessageId=0014 SymbolicName=COMPACT_UNCOMPRESS_CDIR
Language=English

 Uncompress files within %1 [Compress new files]

.

MessageId=0015 SymbolicName=COMPACT_UNCOMPRESS_UDIR
Language=English

 Uncompress files within %1 [Do not compress new files]

.

MessageId=0016 SymbolicName=COMPACT_UNCOMPRESS_SUMMARY
Language=English

%1 files within %2 directories were uncompressed.

.

MessageId=0050 SymbolicName=COMPACT_USAGE
Language=English
Displays and alters the compression of files or directories.

COMPACT [/C | /U] [/S] [/I] [/L] [filename [...]]

  /C        Compresses the specified directory or file.
  /U        Uncompress the specified directory or file.
  /S        Performs the specified operation on
            the current directory and all subdirectories.
  /I        Ignore errors.
  /F        Force the operation to compress or uncompress
            the specified directory or file.
  /L        Display the compression state of the specified
            directory or files.
  filename  Specifies the file or directory.

  Used without parameters, COMPACT displays the compression state of
  the current directory. You may use multiple filenames and
  wildcard.

.
