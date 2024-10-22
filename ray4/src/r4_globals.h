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

//==================================================================================================
// File:  ray4.h
//
// This file contains the global variables and structures for the ray4 four-dimensional raytracer.
// If the macro `DEFINE_GLOBALS' is defined, then the global variables will be declared for file
// that includes this header. If the macro `DEFINE_GLOBALS' is not defined, then the global
// variables will be declared as external references.
//==================================================================================================

#ifndef R4_GLOBALS_H
#define R4_GLOBALS_H

#ifdef  DEFINE_GLOBALS
#   define Global(decl,init)    decl = init
#else
#   define Global(decl,init)    extern decl
#endif


Global (Attributes *attrlist,        nullptr );  // Attributes List
Global (Real        global_indexref, 1.00    );  // Global Index Refraction
Global (char       *infile,          nullptr );  // Input File Name
Global (Light      *lightlist,       nullptr );  // Light-Source List
Global (ushort      maxdepth,        0       );  // Maximum Recursion Depth
Global (ObjInfo    *objlist,         nullptr );  // Object List
Global (char       *outfile,         nullptr );  // Output File Name
Global (Real        Vangle,          45.0    );  // Viewing Angle, X-Axis

#ifndef DEFINE_GLOBALS
   extern Color    ambient;     // Ambient Light Factor
   extern Color    background;  // Background Color
   extern Stats    stats;       // Status Information
   extern Point4   Vfrom;       // Camera Position
   extern Vector4  Vover;       // View Over-Vector
   extern Point4   Vto;         // View Target Point
   extern Vector4  Vup;         // View Up-Vector
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
