// $Revision: 17348 $ $Date: 2017-10-11 11:44:01 -0700 (Wed, 11 Oct 2017) $
// Copyright (c) Bullseye Testing Technology
// This source file contains confidential proprietary information.
//
// BullseyeCoverage small footprint run-time porting layer
// http://www.bullseye.com/help/env-embedded.html
//
// Implementations should conform to the Single UNIX Specification
// http://www.opengroup.org/onlinepubs/009695399/

#if defined(_BullseyeCoverage)
	#pragma BullseyeCoverage off


//---------------------------------------------------------------------------
// NULL
// Change the line below to "#if 0" if you do not have stdlib.h
#if 1
	#include <stdlib.h>
#else
	#define NULL 0
#endif

#define BSPTESTLOG printk

//---------------------------------------------------------------------------
// open, close, write
// Change the line below to "#if 0" if you do not have these headers
#if 0
	#include <fcntl.h>
	#include <sys/stat.h>
	#include <unistd.h>
	#if !defined(O_CREAT)
		#error O_CREAT not defined
	#endif
	#if !defined(S_IRUSR)
		#error S_IRUSR not defined
	#endif
#else
	// http://pubs.opengroup.org/onlinepubs/009695399/functions/open.html
	// http://pubs.opengroup.org/onlinepubs/009695399/functions/close.html
	// http://pubs.opengroup.org/onlinepubs/009695399/functions/write.html
	#define O_CREAT 0x0100
	#define O_TRUNC 0x0200
	#define O_WRONLY 1
	#define S_IRUSR 0x0100
	#define S_IWUSR 0x0080

        static int s_linenum=0;

	int bullseye_getpid(void)
	{
		// If you have multiple instrumented programs running simultaneously,
		// this function should return a different value for each of them.
		// Otherwise, return any value.
		return 1;
	}

	int bullseye_open(const char* path, int oflag, int mode)
	{
                s_linenum++;
		// Insert your code here. Successful return is > 2
                BSPTESTLOG("[BULLSEYE:%08d:KERNEL] Open Bullseye logging %s\n", s_linenum, path);
		return 3;
	}

	int bullseye_close(int fildes)
	{
                s_linenum++;
		// Insert your code here. The return value is not used.
                BSPTESTLOG("[BULLSEYE:%08d:KERNEL] Close Bullseye logging\n",s_linenum);
		return 0;
	}

        #define OUT_BUF_SIZE 128 /* max size by guess. */
	int bullseye_write(int fildes, const void* buf, unsigned nbyte)
	{
                static char str[OUT_BUF_SIZE];
                s_linenum++;
		if (fildes == 2) {
			// Insert your code here to report the error message in buf.
			// This is critical. Implement this first.
			// The message ends with a newline.
			// A null character follows the buffer (buf[nbyte] == '\0').
                        if (nbyte>OUT_BUF_SIZE) nbyte=OUT_BUF_SIZE;
                        memcpy(str, buf, nbyte);
                        str[nbyte] = 0;
                        BSPTESTLOG("[BULLSEYE:%08d:KERNEL] %s\n", s_linenum, str);
		} else {
			// Insert your code here to write data
                        if (nbyte < OUT_BUF_SIZE){
                                memcpy(str, buf, nbyte);
                                str[nbyte] = 0;
                                BSPTESTLOG("[BULLSEYE:%08d:KERNEL] %s", s_linenum, str);
                                return nbyte;
                        }else{
                                BSPTESTLOG("[BULLSEYE:%08d:KERNEL] Error in Bullseye logging. Length > 128.\n", s_linenum);
                                return -1;
                        }
		}
		// Successful return is number of bytes written >= 0
		return -1;
	}
#endif

//---------------------------------------------------------------------------
// BullseyeCoverage run-time code
#include "atomic.h"
#include "libcov.h"
#include "version.h"
#include "libcov-core-small.h"

#endif
