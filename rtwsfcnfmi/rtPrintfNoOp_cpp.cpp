/*****************************************************************
 *  Copyright (c) Dassault Systemes. All rights reserved.        *
 *  This file is part of FMIKit. See LICENSE.txt in the project  *
 *  root for license information.                                *
 *****************************************************************/

/*
-----------------------------------------------------------
	C++ implementation of rtPrintfNoOp function to
	enable proper linking with C++ S-functions.
-----------------------------------------------------------
*/

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

extern "C" {
    extern int rtPrintfNoOp_C(const char *fmt, ...);
}

/* FMU mapping of ssPrintf for child C++ source S-functions (through rtPrintfNoOp) */
int rtPrintfNoOp(const char *fmt, ...)
{
	char buf[4096];
	va_list ap;
    int capacity;

	va_start(ap, fmt);
    capacity = sizeof(buf) - 1;
#if defined(_MSC_VER) && _MSC_VER>=1400
	vsnprintf_s(buf, capacity, _TRUNCATE, fmt, ap);
#else
    buf[capacity]=0;
	vsnprintf(buf, capacity, fmt, ap);
#endif
	va_end(ap);
	rtPrintfNoOp_C(buf);

    return 0;
}
