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
