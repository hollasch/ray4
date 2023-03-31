/*******************************************************************************
Ray4 is Copyright (C) Steve Hollasch, 1991-1996

All rights reserved.  This software may be freely copied, modified and
redistributed, provided that this copyright notice is preserved in all copies.
This software is provided "as is", without express or implied warranty.  You
may not include this software in a program or other software product without
also supplying the source, except by express agreement with the author (Steve
Hollasch).  If you modify this software, please include a notice detailing the
author, date and purpose of the modification.
*******************************************************************************/

/*******************************************************************************

File:  ray4.h

    This file contains the global variables and structures for the ray4 four-
dimensional raytracer.  If the macro `DEFINE_GLOBALS' is defined, then the
global variables will be declared for file that includes this header.  If the
macro `DEFINE_GLOBALS' is not defined, then the global variables will be
declared as external references.

*******************************************************************************/

#ifndef R4_GLOBALS_H
#define R4_GLOBALS_H

#ifdef  DEFINE_GLOBALS
#   define Global(decl,init)    decl = init
#else
#   define Global(decl,init)    extern decl
#endif


Global (Attributes *attrlist,        nil  );  /* Attributes List */
Global (Real        global_indexref, 1.00 );  /* Global Index Refraction */
Global (char       *infile,          nil  );  /* Input File Name */
Global (Light      *lightlist,       nil  );  /* Light-Source List */
Global (ushort      maxdepth,        0    );  /* Maximum Recursion Depth */
Global (ObjInfo    *objlist,         nil  );  /* Object List */
Global (char       *outfile,         nil  );  /* Output File Name */
Global (Real        Vangle,          45.0 );  /* Viewing Angle, X-Axis */

#ifndef DEFINE_GLOBALS
   extern Color    ambient;     /* Ambient Light Factor */
   extern Color    background;  /* Background Color */
   extern Stats    stats;       /* Status Information */
   extern Point4   Vfrom;       /* Camera Position */
   extern Vector4  Vover;       /* View Over-Vector */
   extern Point4   Vto;         /* View Target Point */
   extern Vector4  Vup;         /* View Up-Vector */
#else
   Color    ambient     = { .0, .0, .0 };
   Color    background  = { .0, .0, .0 };
   Stats    stats       = { 0, 0, 0, 0 };
   Point4   Vfrom       = { 0.0, 0.0, 0.0, 100.0 };
   Vector4  Vover       = { 0.0, 0.0, 1.0, 0.0 };
   Point4   Vto         = { 0.0, 0.0, 0.0, 0.0 };
   Vector4  Vup         = { 0.0, 1.0, 0.0, 0.0 };
#endif

#endif
