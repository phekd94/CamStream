
//=============================================================================

#include "CamStream_TLPI.h"

#include "CamStream_Print.h"

#include <stdarg.h>

//=============================================================================

static FILE			*out			= NULL;

//=============================================================================

void 
CamStream_Print(Boolean err, const char *format, ...)
{
 #define	BUF_SIZE	100
	
	char buf[BUF_SIZE], userMsg[BUF_SIZE];
	va_list argList;

	va_start(argList, format);
	vsprintf(userMsg, format, argList);
	va_end(argList);

	sprintf(buf, "[CamS]: %s", 
		(err == TRUE) ? "[error]: " ANSI_COLOR_RED : "");

	fprintf((out == NULL) ? stdout : out, 
		"%s%s" ANSI_COLOR_RESET "\n", buf, userMsg);

 #undef		BUF_SIZE
}

//=============================================================================

void 
CamStream_PrintSetOut(FILE *new_out)
{
	out = new_out;
}

//=============================================================================
