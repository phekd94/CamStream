
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

#ifndef TLPI_HDR_H
#define TLPI_HDR_H

// TLPI = The Linux Programming Interface

//=============================================================================

#include <sys/types.h>		// many types
#include <stdio.h>		// in/out 
#include <stdlib.h>		// EXIT_SUCCESS, EXIT_FAILURE
#include <unistd.h>		// prototype many system calls
#include <errno.h>
#include <string.h>

//=============================================================================

//#include "get_num.h"
#include "CamStream_ErrorFunctions.h"

//=============================================================================

typedef   enum {FALSE, TRUE}   Boolean;

//=============================================================================

#define   max(m,n)   ((m) > (n) ? (m) : (n))
#define   min(m,n)   ((m) < (n) ? (m) : (n))

//=============================================================================

#endif // TLPI_HDR_H