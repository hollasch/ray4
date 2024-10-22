//**************************************************************************************************
//  Copyright (c) 1991-2024 Steven R Hollasch
//
//  MIT License
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
//  and associated documentation files (the "Software"), to deal in the Software without
//  restriction, including without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies or
//  substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
//  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//**************************************************************************************************

/****************************************************************************
**
**  File:  amiga.h                            Author:  Steve Hollasch
**
**      This file contains defined constants, macros, data structures and
**  function definitions for the amiga portion of the ray4 package.
**
****************************************************************************/

#ifndef AMIGA_H
#define AMIGA_H

#ifndef LIBRARIES_DOS_H
#  include <libraries/dos.h>
#endif

#ifndef INTUITION_INTUITION_H
#  include <intuition/intuition.h>
#endif

    /***  Macro Definitions  ***/

#define nil ((void*)(0))

#define true   1
#define false  0

#define print(s)  fputs(s,stdout)

#define ABS(x)    (((x) < 0) ? -(x) : (x))
#define MIN(x,y)  (((x) < (y)) ? (x) : (y))
#define MAX(x,y)  (((x) > (y)) ? (x) : (y))


    /***  Type Definitions  ***/

#ifndef ulong
#  define uchar   unsigned char    /* Unsigned  8-bit Integer */
#  define ushort  unsigned short   /* Unsigned 16-bit Integer */
#  define ulong   unsigned long    /* Unsigned 32-bit Integer */
#endif

typedef short boolean;  /* Boolean; `true'(1) or `false'(0) */


    /***  Function Declarations  ***/

short  WriteILBM  (/* char*, struct Screen* */);

#endif
