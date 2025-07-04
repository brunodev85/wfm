/* config.h for Visual Studio compilation  */

/* compiler does lsbf in struct bitfields */
#undef BITFIELD_LSBF

/* Define 1 if you are compiling using cygwin */
#undef CYGWIN

/* what to put between the brackets for empty arrays */
#define EMPTY_ARRAY_SIZE

/* Define 1 if you have BSDI-type CD-ROM support */
#undef HAVE_BSDI_CDROM

/* Define to 1 if you have the `bzero' function. */
#undef HAVE_BZERO

/* Define this if you have libcddb installed */
#undef HAVE_CDDB

/* Define to 1 if you have the <CoreFoundation/CFBase.h> header file. */
#undef HAVE_COREFOUNDATION_CFBASE_H

/* Define 1 if you have Darwin OS X-type CD-ROM support */
#undef HAVE_DARWIN_CDROM

/* Define if time.h defines extern long timezone and int daylight vars. */
#undef HAVE_DAYLIGHT

/* Define to 1 if you have the <dlfcn.h> header file. */
#undef HAVE_DLFCN_H

/* Define to 1 if you have the <dvd.h> header file. */
#undef HAVE_DVD_H

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define 1 if you have FreeBSD CD-ROM support */
#undef HAVE_FREEBSD_CDROM

/* Define to 1 if you have the <glob.h> header file. */
#undef HAVE_GLOB_H

/* Define if you have the iconv() function. */
#undef HAVE_ICONV

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <IOKit/IOKitLib.h> header file. */
#undef HAVE_IOKIT_IOKITLIB_H

/* Supports ISO _Pragma() macro */
#undef HAVE_ISOC99_PRAGMA

/* Define 1 if you want ISO-9660 Joliet extension support. You must have also
   libiconv installed to get Joliet extension support. */
#define HAVE_JOLIET 1

/* Define if you have <langinfo.h> and nl_langinfo(CODESET). */
#undef HAVE_LANGINFO_CODESET

/* Define to 1 if you have the `nsl' library (-lnsl). */
#undef HAVE_LIBNSL

/* Define to 1 if you have the `socket' library (-lsocket). */
#undef HAVE_LIBSOCKET

/* Define 1 if you have Linux-type CD-ROM support */
#undef HAVE_LINUX_CDROM

/* Define to 1 if you have the <linux/cdrom.h> header file. */
#undef HAVE_LINUX_CDROM_H

/* Define 1 if timeout is in cdrom_generic_command struct */
#undef HAVE_LINUX_CDROM_TIMEOUT

/* Define to 1 if you have the <linux/version.h> header file. */
#undef HAVE_LINUX_VERSION_H

/* Define to 1 if you have the `memcpy' function. */
#define HAVE_MEMCPY 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `memset' function. */
#define HAVE_MEMSET 1

/* Define to 1 if you have the Windows SCSI layer. */
#define HAVE_NTDDSCSI_H 1

/* Define to 1 if you have the Windows CD-ROM layer. */
#define HAVE_NTDDCDRM_H 1

/* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF 1

/* Define 1 if you have Solaris CD-ROM support */
#undef HAVE_SOLARIS_CDROM

/* Define to 1 if you have the <stdbool.h> header file. */
#define HAVE_STDBOOL_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdio.h> header file. */
#define HAVE_STDIO_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <stdarg.h> header file. */
#define HAVE_STDARG_H 1

/* Define to 1 if you have the <strings.h> header file. */
#undef HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <windows.h> header file. */
#define HAVE_WINDOWS_H 1

/* Define to 1 if you have the <sys/cdio.h> header file. */
#undef HAVE_SYS_CDIO_H

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define if struct tm has the tm_gmtoff member. */
#undef HAVE_TM_GMTOFF

/* Define if time.h defines extern extern char *tzname[2] variable */
#undef HAVE_TZNAME

/* Define to 1 if you have the `tzset' function. */
#undef HAVE_TZSET

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define this if you have libvcdinfo installed */
#undef HAVE_VCDINFO

/* Define to 1 if you have the `vsnprintf' function. */
#define HAVE_VSNPRINTF 1

/* Define as const if the declaration of iconv() needs const. */
#undef ICONV_CONST 

/* Define 1 if you are compiling using MinGW */
#define MINGW32 1

/* Name of package */
#define PACKAGE "libcdio"

/* Define to the address where bug reports for this package should be sent. */
#undef PACKAGE_BUGREPORT

/* Define to the full name of this package. */
#define PACKAGE_NAME "libcdio"

/* Define to the full name and version of this package. */
#undef PACKAGE_STRING

/* Define to the one symbol short name of this package. */
#undef PACKAGE_TARNAME

/* Define to the version of this package. */
#define PACKAGE_VERSION 1

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION "1"

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
#undef WORDS_BIGENDIAN
