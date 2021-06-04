
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

#ifndef ERROR_FUNCTIONS_H
#define ERROR_FUNCTIONS_H

//=============================================================================

void errMsg(const char *format, ...);

//=============================================================================

#ifdef __GNUC__
   #define   NORETURN   __attribute__ ((__noreturn__))
#else
   #define   NORETURN 
#endif

//=============================================================================

void errExit(const char *format, ...) NORETURN;
void fatal(const char *format, ...) NORETURN;
void usageErr(const char *format, ...) NORETURN;
void cmdLineErr(const char *format, ...) NORETURN;
void err_exit(const char *format, ...) NORETURN;
void errExitEN(int errnum, const char *format, ...) NORETURN;

//=============================================================================

#endif // ERROR_FUNCTIONS_H