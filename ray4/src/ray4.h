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

#include <cmath>
#include <cstddef>
#include <stdlib.h>
#include <stdio.h>



// Basic Type Definitions

using uchar  = unsigned char;
using ulong  = unsigned long;
using ushort = unsigned short;


// Constant Definitions

const double epsilon = 1.0e-15;  // Very Small Number (Effectively Zero)

const double pi=3.14159265358979323846;

const double degreeToRadian = pi / 180.0;
const double radianToDegree = 180.0 / pi;

#define X 0
#define Y 1
#define Z 2
#define W 3

enum class ObjType {    // Object Type ID's
    None,
    Sphere,
    Tetrahedron,
    Triangle,
    Parallelepiped
};


// Inline Utility Functions

inline void print (const char* str) {
    fputs(str, stdout);
}

inline double clamp (double x, double min, double max) {
    if (x < min)
        return min;
    if (x > max)
        return max;
    return x;
}

inline double lerp (double a, double b, double t) {
    return a + t*(b - a);
}

#define NEW(type,num)  (type *) MyAlloc((unsigned long)(num)*sizeof(type))
#define DELETE(addr)   MyFree ((char*)addr)


// Standard Ray4 Includes

#include "r4_color.h"
#include "r4_point.h"
#include "r4_ray.h"
#include "r4_vector.h"


// Structure Definitions

struct Stats {
    ulong  Ncast;     // Number of Rays Cast
    ulong  Nreflect;  // Number of Reflection Rays Cast
    ulong  Nrefract;  // Number of Refraction Rays Cast
    ulong  maxlevel;  // Maximum Ray Tree Level
};

enum class LightType { Point, Directional };

class Light {
  public:
    Light     *next;   // Next Light Source
    Color      color;  // Light Color
    LightType  type;   // Type of Light

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

struct Attributes {
    Attributes *next;      // Link to Next Attributes Structure
    Color       Ka;        // Ambient Illumination Color
    Color       Kd;        // Diffuse Illumination Color
    Color       Ks;        // Specular Illumination Color
    Color       Kt;        // Transparent Illumination Color
    double      shine;     // Phong Specular Reflection Factor
    double      indexref;  // Index of Refraction
    AttrFlag    flags;     // Attribute Flags
};


// Info Flag Definitions

using InfoFlag = unsigned char;
const InfoFlag FL_GOURAUD = (1 << 0);  // Set if the Object is Gouraud Shaded
const InfoFlag FL_PHONG   = (1 << 1);  // Set if the Object is Phong Shaded


struct ObjInfo {
    ObjInfo    *next;       // Pointer to Next Object
    Attributes *attr;       // Object Attributes
    ObjType     type;       // Object Type
    InfoFlag    flags;      // Information Flags
    bool      (*intersect)  // Intersection Function
                (ObjInfo*, const Ray4&, double*, Point4*, Vector4*);
};

struct Sphere {
    ObjInfo info;    // Common Object Fields; Must Be First Field
    Point4  center;  // Sphere Center
    double  radius;  // Sphere Radius
    double  rsqrd;   // Sphere Radius, Squared
};

struct TetPar {            // Tetrahedron/Parallelepiped Common Fields
    Point4  vert[4];         // Vertices
    Vector4 vec1,vec2,vec3;  // Vectors from Vertex 0 to Vertices 1,2,3
    Vector4 normal;          // Hyperplane Normal Vector
    uchar   ax1, ax2, ax3;   // Non-Dominant Normal Vector Axes
    double  planeConst;      // Hyperplane Constant
    double  CramerDiv;       // Cramer's-Rule Divisor for Barycentric Coords
};

struct Tetrahedron {
    ObjInfo info;           // Common Obj Fields; Must Be First Field
    TetPar  tp;             // Tetrahedron/Parallelepiped Data
    double  Bc1, Bc2, Bc3;  // Barycentric Coordinate Values
};

struct Parallelepiped {
    ObjInfo info;  // Common Obj Fields; Must Be First Field
    TetPar  tp;    // Tetrahedron/Parallelepiped Data
};

struct Triangle {
    ObjInfo  info;        // Common Object Fields; Must Be First Field
    Point4   vert[3];     // Triangle Vertices
    Vector4  vec1, vec2;  // vector from Vertex0 to Vertices 1,2.
    double   Bc1, Bc2;    // Barycentric Coordinate Values
};


// Function Declarations

void  CloseInput  (void);
void  CloseOutput (void);
void  Halt        (const char*, ...);
bool  HitSphere   (ObjInfo*, const Ray4&, double*, Point4*, Vector4*);
bool  HitTetPar   (ObjInfo*, const Ray4&, double*, Point4*, Vector4*);
bool  HitTriangle (ObjInfo*, const Ray4&, double*, Point4*, Vector4*);
char *MyAlloc     (size_t);
void  MyFree      (void*);
void  OpenInput   (void);
void  OpenOutput  (void);
void  ParseInput  (void);
void  RayTrace    (const Ray4&, Color&, ulong);
int   ReadChar    (void);
void  UnreadChar  (int);
void  WriteBlock  (void *block, int size);


// Global Variables

#ifdef DEFINE_GLOBALS
    Color       ambient         = { .0, .0, .0 };            // Ambient Light Factor
    Attributes *attrlist        = nullptr;                   // Attributes List
    Color       background      = { .0, .0, .0 };            // Background Color
    double      global_indexref = 1.00;                      // Global Index Refraction
    char       *infile          = nullptr;                   // Input File Name
    Light      *lightlist       = nullptr;                   // Light-Source List
    ushort      maxdepth        = 0;                         // Maximum Recursion Depth
    ObjInfo    *objlist         = nullptr;                   // Object List
    char       *outfile         = nullptr;                   // Output File Name
    Stats       stats           = { 0, 0, 0, 0 };            // Status Information
    double      Vangle          = 45.0;                      // Viewing Angle
    Point4      Vfrom           = { 0.0, 0.0, 0.0, 100.0 };  // Camera Position
    Vector4     Vover           = { 0.0, 0.0, 1.0, 0.0 };    // View Over-Vector
    Point4      Vto             = { 0.0, 0.0, 0.0, 0.0 };    // View Target Point
    Vector4     Vup             = { 0.0, 1.0, 0.0, 0.0 };    // View Up-Vector
#else
    extern Color       ambient;
    extern Attributes *attrlist;
    extern Color       background;
    extern double      global_indexref;
    extern char       *infile;
    extern Light      *lightlist;
    extern ushort      maxdepth;
    extern ObjInfo    *objlist;
    extern char       *outfile;
    extern Stats       stats;
    extern double      Vangle;
    extern Point4      Vfrom;
    extern Vector4     Vover;
    extern Point4      Vto;
    extern Vector4     Vup;
#endif

#endif
