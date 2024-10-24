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
// ray4.h
//
// This file contains structure, variable, constant and macro declarations and definitions for the
// Ray4 4D raytracer. See the header in r4_main.c for more information on the raytracer.
//
// NOTE: THIS PROGRAM MAKES THE FOLLOWING ASSUMPTIONS:
//     - char's are exactly 8 bits
//     - short integers are exactly 16 bits.
//     - long integers are exactly 32 bits.
//==================================================================================================

#ifndef RAY_H
#define RAY_H

#include <stdlib.h>
#include <stdio.h>



// Basic Type Definitions

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned long  ulong;
typedef double         Real;


#include "r4_vector.h"


// Program Parameters

#define EPSILON 1.0e-15  // Very Small Number (Effectively Zero)


// Constant Definitions

#define PI 3.14159265358979323846

#define DegreeToRadian  (PI/180.0)
#define RadianToDegree  (180.0/PI)

#define X 0
#define Y 1
#define Z 2
#define W 3

typedef enum {    // Object Type ID's
    O_NONE,
    O_SPHERE,
    O_TETRAHEDRON,
    O_TRIANGLE,
    O_PARALLELEPIPED
} ObjType;


// Macro Definitions

#define strsize(s)              (strlen(s)+1)
#define strrel(s1,op,s2)        (strcmp(s1,s2) op 0)
#define print(s)                fputs(s,stdout)
#define ALIMIT(array)           (sizeof(array) / sizeof(array[0]))
#define CLAMP(value,low,high)   \
    (((value) < (low)) ? (low) : (((value) > (high)) ? (high) : (value)))

#define  NEW(type,num)  (type *) MyAlloc((unsigned long)(num)*sizeof(type))
#define  DELETE(addr)   MyFree ((char*)addr)

#define  Color_List(C)  C.r,C.g,C.b

#define  Color_Scale(C,asn,k)   \
(   C.r asn k,  \
    C.g asn k,  \
    C.b asn k   \
)

#define  Color_2Op(A,asn,B) \
(   A.r asn B.r,        \
    A.g asn B.g,        \
    A.b asn B.b  \
)

#define  Color_3Op(A,asn,B,op,C) \
(   A.r asn B.r op C.r, \
    A.g asn B.g op C.g, \
    A.b asn B.b op C.b  \
)


// Structure Definitions

typedef struct S_ATTRIBUTES     Attributes;
typedef struct S_COLOR          Color;
typedef struct S_LIGHT          Light;
typedef struct S_OBJINFO        ObjInfo;
typedef struct S_PARALLELEPIPED Parallelepiped;
typedef struct S_RAY            Ray;
typedef struct S_SPHERE         Sphere;
typedef struct S_STATS          Stats;
typedef struct S_TETRAHEDRON    Tetrahedron;
typedef struct S_TETPAR         TetPar;
typedef struct S_TRIANGLE       Triangle;

struct S_COLOR {   // Color Triple
    Real r, g, b;  // Each color should be in [0,1].
};

struct S_RAY {        // Ray Definition
    Point4   origin;  // Ray Origin
    Vector4  dir;     // Ray Direction
};

struct S_STATS {
    ulong  Ncast;     // Number of Rays Cast
    ulong  Nreflect;  // Number of Reflection Rays Cast
    ulong  Nrefract;  // Number of Refraction Rays Cast
    ulong  maxlevel;  // Maximum Ray Tree Level
};

typedef enum { L_POINT, L_DIRECTIONAL } LType;

struct S_LIGHT {
    Light *next;   // Next Light Source
    Color  color;  // Light Color
    LType  type;   // Type of Light

    union {
        Vector4 dir;  // Direction for Directional Light Source
        Point4  pos;  // Position for Point Light Source
    } u;
};


// Attribute Flag Definitions

using AttrFlag = unsigned char;
const AttrFlag AT_AMBIENT  = (1 << 0);  // Set if Ambient is Non-Zero
const AttrFlag AT_DIFFUSE  = (1 << 1);  // Set if Diffuse is Non-Zero
const AttrFlag AT_SPECULAR = (1 << 2);  // Set if Specular is Non-Zero
const AttrFlag AT_TRANSPAR = (1 << 3);  // Set if Transparency is Non-Zero
const AttrFlag AT_REFLECT  = (1 << 4);  // Set if Object Reflects Light

struct S_ATTRIBUTES {
    Attributes *next;      // Link to Next Attributes Structure
    Color       Ka;        // Ambient Illumination Color
    Color       Kd;        // Diffuse Illumination Color
    Color       Ks;        // Specular Illumination Color
    Color       Kt;        // Transparent Illumination Color
    Real        shine;     // Phong Specular Reflection Factor
    Real        indexref;  // Index of Refraction
    AttrFlag    flags;     // Attribute Flags
};


// Info Flag Definitions

using InfoFlag = unsigned char;
const InfoFlag FL_GOURAUD = (1 << 0);  // Set if the Object is Gouraud Shaded
const InfoFlag FL_PHONG   = (1 << 1);  // Set if the Object is Phong Shaded


struct S_OBJINFO {
    ObjInfo    *next;       // Pointer to Next Object
    Attributes *attr;       // Object Attributes
    uchar       type;       // Object Type
    InfoFlag    flags;      // Information Flags
    bool      (*intersect)  // Intersection Function
                (ObjInfo*, Point4, Vector4, Real*, Point4, Vector4);
};

struct S_SPHERE {
    ObjInfo info;    // Common Object Fields; Must Be First Field
    Point4  center;  // Sphere Center
    Real    radius;  // Sphere Radius
    Real    rsqrd;   // Sphere Radius, Squared
};

struct S_TETPAR {            // Tetrahedron/Parallelepiped Common Fields
    Point4  vert[4];         // Vertices
    Vector4 vec1,vec2,vec3;  // Vectors from Vertex 0 to Vertices 1,2,3
    Vector4 normal;          // Hyperplane Normal Vector
    uchar   ax1, ax2, ax3;   // Non-Dominant Normal Vector Axes
    Real    planeConst;      // Hyperplane Constant
    Real    CramerDiv;       // Cramer's-Rule Divisor for Barycentric Coords
};

struct S_TETRAHEDRON {
    ObjInfo info;           // Common Obj Fields; Must Be First Field
    TetPar  tp;             // Tetrahedron/Parallelepiped Data
    Real    Bc1, Bc2, Bc3;  // Barycentric Coordinate Values
};

struct S_PARALLELEPIPED {
    ObjInfo info;  // Common Obj Fields; Must Be First Field
    TetPar  tp;    // Tetrahedron/Parallelepiped Data
};

struct S_TRIANGLE {
    ObjInfo  info;        // Common Object Fields; Must Be First Field
    Point4   vert[3];     // Triangle Vertices
    Vector4  vec1, vec2;  // vector from Vertex0 to Vertices 1,2.
    Real     Bc1, Bc2;    // Barycentric Coordinate Values
};


// Function Declarations

void  CloseInput  (void);
void  CloseOutput (void);
void  Halt        (char*, ...);
bool  HitSphere   (ObjInfo*, Point4, Vector4, Real*, Point4, Vector4);
bool  HitTetPar   (ObjInfo*, Point4, Vector4, Real*, Point4, Vector4);
bool  HitTriangle (ObjInfo*, Point4, Vector4, Real*, Point4, Vector4);
char *MyAlloc     (size_t);
void  MyFree      (void*);
void  OpenInput   (void);
void  OpenOutput  (void);
void  ParseInput  (void);
void  RayTrace    (Point4, Vector4, Color*, ulong);
int   ReadChar    (void);
void  UnreadChar  (int);
void  WriteBlock  (void *block, int size);

#endif
