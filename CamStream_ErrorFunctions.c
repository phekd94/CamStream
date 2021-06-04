
/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2018.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

//=============================================================================

#include <stdarg.h>
#include "CamStream_TLPI.h"
#include "CamStream_ErrorFunctions.h"

//=============================================================================

#ifdef __GNUC__
   __attribute__ ((__noreturn__))
#endif

//=============================================================================

static void
terminate(Boolean useExit3)
{
	char *s;
	s = getenv("EF_DUMPCORE");

	if (s != NULL && *s != '\0') // ??? *s != '\0' ???
		abort();
	else if (useExit3)
		exit(EXIT_FAILURE);
	else
		_exit(EXIT_FAILURE);
}

//-----------------------------------------------------------------------------

static void 
outputError(Boolean useErr, int err, Boolean flushStdout, const char *format, 
	va_list ap)
{
#define   BUF_SIZE   100
	char buf[BUF_SIZE], userMsg[BUF_SIZE], errText[BUF_SIZE];
	
	vsnprintf(userMsg, BUF_SIZE, format, ap);
	if (useErr)
		snprintf(errText, BUF_SIZE, " [err = %d - %s]:", 
			err, strerror(err));
	else
		snprintf(errText, BUF_SIZE, ":");
	snprintf(buf, BUF_SIZE, "ERROR%s %s\n", errText, userMsg);

	if (flushStdout)
		fflush(stdout);
	
	fputs(buf, stderr);
	fflush(stderr);
	
#undef   BUF_SIZE
}

//=============================================================================

void 
errMsg(const char *format, ...)
{
	va_list argList;
	int savedErrno;
	savedErrno = errno; // if errno will change in outputError()
	va_start(argList, format);
	outputError(TRUE, errno, TRUE, format, argList);
	va_end(argList);
	errno = savedErrno; // recovery errno
}

//-----------------------------------------------------------------------------

void 
errExit(const char *format, ...)
{
	va_list argList;
	va_start(argList, format);
	outputError(TRUE, errno, TRUE, format, argList);
	va_end(argList);
	terminate(TRUE);
}

//-----------------------------------------------------------------------------

void 
fatal(const char *format, ...)
{
	va_list argList;
	va_start(argList, format);
	outputError(FALSE, 0, TRUE, format, argList);
	va_end(argList);
	terminate(TRUE);
}

//-----------------------------------------------------------------------------

void 
usageErr(const char *format, ...)
{
	va_list argList;
	fflush(stdout);
	fprintf(stderr, "Usage: ");
	va_start(argList, format);
	vfprintf(stderr, format, argList);
	fputc('\n', stderr);
	va_end(argList);
	fflush(stderr);
	exit(EXIT_FAILURE);
}

//-----------------------------------------------------------------------------

void 
err_exit(const char *format, ...)
{
	va_list argList;
	va_start(argList, format);
	outputError(TRUE, errno, FALSE, format, argList);
	va_end(argList);
	terminate(FALSE);
}

//-----------------------------------------------------------------------------

void 
errExitEN(int errnum, const char *format, ...)
{
	va_list argList;
	va_start(argList, format);
	outputError(TRUE, errnum, TRUE, format, argList);
	va_end(argList);
	terminate(TRUE);
}

//-----------------------------------------------------------------------------

void 
cmdLineErr(const char *format, ...)
{
	va_list argList;
	fflush(stdout);
	fprintf(stderr, "Command-line usage error: ");
	va_start(argList, format);
	vfprintf(stderr, format, argList);
	fputc('\n', stderr);
	va_end(argList);
	fflush(stderr);
	exit(EXIT_FAILURE);
}

//-----------------------------------------------------------------------------
