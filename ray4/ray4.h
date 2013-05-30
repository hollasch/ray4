/****************************************************************************
**
**  File:  ray4.h                                 Author:  Steve Hollasch
**
**          This file contains structure, variable, constant and macro
**      declarations and definitions for the Ray4 4D raytracer.  See the
**      header in r4_main.c for more information on the raytracer.
**
**      NOTE:  THIS PROGRAM MAKES THE FOLLOWING ASSUMPTIONS:
**             - char's are 8 bits exactly.
**             - short integers are 16 bits exactly.
**             - long integers are 32 bits exactly.
**
**  Revisions:
**
**    1.??  11-Apr-91  Hollasch
**          Created the Ray structure (currently unused).
**
**    1.20  01-Jan-91  Hollasch
**          Defined attribute structure and created linked attribute list
**          structure.
**
**    1.10  17-Dec-90  Hollasch
**          Added Parallelepiped structure.  Removed
**          tetrahedron/parallelepiped common fields to new `TetPar'
**          structure.
**
**    1.01  20-Nov-90  Hollasch
**          Moved some variable computations from on-the-fly to precomputed
**          fields in structures for both 2D triangles and tetrahedrons.
**
**    1.00  15-Nov-90  Hollasch
**          Added tetrahedron and triangle code.
**
**    0.00  30-Sep-90  Steve R. Hollasch
**          Initial version.
**
****************************************************************************/

#ifndef RAY_H
#define RAY_H


#ifndef STEVE_H
#  include <steve.h>
#endif

#ifndef VECTOR_H
#  include <vector.h>
#endif

#ifdef sgi					/* Grrrrrr... */
#  define acos(x)	_lacos(x)
#  define asin(x)	_lasin(x)
#  define atan(x)	_latan(x)
#  define atof(s)	_latof(s)
#  define ceil(x)	_lceil(x)
#  define cos(x)	_lcos(x)
#  define exp(x)	_lexp(x)
#  define fabs(x)	_lfabs(x)
#  define floor(x)	_lfloor(x)
#  define log(x)	_llog(x)
#  define pow(x,y)	_lpow(x,y)
#  define sin(x)	_lsin(x)
#  define sqrt(x)	_lsqrt(x)
#  define tan(x)	_ltan(x)
#endif


	/*======================*/
	/*  Program Parameters  */
	/*======================*/

#define EPSILON	 1.0e-15	/* Very Small Number (Effectively Zero) */


	/*========================*/
	/*  Constant Definitions  */
	/*========================*/

#define  X  0
#define  Y  1
#define  Z  2
#define  W  3

#define  O_SPHERE         1		/* Object Type ID's */
#define  O_TETRAHEDRON    2
#define  O_TRIANGLE       3
#define  O_PARALLELEPIPED 4


	/*=====================*/
	/*  Macro Definitions  */
	/*=====================*/

#define  NEW(type,num)  (type *) MyAlloc((unsigned long)(num)*sizeof(type))
#define  DELETE(addr)	MyFree ((char*)addr)

#define  Color_List(C)	C.r,C.g,C.b

#define  Color_Scale(C,asn,k)	\
(  C.r asn k,	\
   C.g asn k,	\
   C.b asn k	\
)

#define  Color_2Op(A,asn,B) \
(  A.r asn B.r,	\
   A.g asn B.g,	\
   A.b asn B.b  \
)

#define  Color_3Op(A,asn,B,op,C) \
(  A.r asn B.r op C.r,	\
   A.g asn B.g op C.g,	\
   A.b asn B.b op C.b	\
)


	/*=========================*/
	/*  Structure Definitions  */
	/*=========================*/

typedef struct S_ATTRIBUTES	Attributes;
typedef struct S_COLOR		Color;
typedef struct S_LIGHT		Light;
typedef struct S_OBJINFO	ObjInfo;
typedef struct S_PARALLELEPIPED	Parallelepiped;
typedef struct S_RAY            Ray;
typedef struct S_SPHERE		Sphere;
typedef struct S_STATS		Stats;
typedef struct S_TETRAHEDRON	Tetrahedron;
typedef struct S_TETPAR		TetPar;
typedef struct S_TRIANGLE	Triangle;

struct S_COLOR		/* Color Triple */
{  Lfloat r, g, b;	/* Each color should be in [0,1]. */
};

struct S_RAY		/* Ray Definition */
{  Point4   origin;	/* Ray Origin */
   Vector4  dir;	/* Ray Direction */
};

struct S_STATS
{  ulong  Ncast;	/* Number of Rays Cast */
   ulong  Nreflect;	/* Number of Reflection Rays Cast */
   ulong  Nrefract;	/* Number of Refraction Rays Cast */
   ulong  maxlevel;	/* Maximum Ray Tree Level */
};

typedef enum { L_POINT, L_DIRECTIONAL } LType;

struct S_LIGHT
{  Light  *next;	/* Next Light Source */
   Color   color;	/* Light Color */
   LType   type;	/* Type of Light */
   union
   {  Vector4  dir;	/* Direction for Directional Light Source */
      Point4   pos;	/* Position for Point Light Source */
   } u;
};

	/* Attribute Flag Definitions */

#define  AT_AMBIENT  (1<<0)	/* Set if Ambient is Non-Zero */
#define  AT_DIFFUSE  (1<<1)	/* Set if Diffuse is Non-Zero */
#define  AT_SPECULAR (1<<2)	/* Set if Specular is Non-Zero */
#define  AT_TRANSPAR (1<<3)	/* Set if Transparency is Non-Zero */
#define  AT_REFLECT  (1<<4)	/* Set if Object Reflects Light */

struct S_ATTRIBUTES
{  Attributes *next;		/* Link to Next Attributes Structure */
   Color       Ka;		/* Ambient Illumination Color */
   Color       Kd;		/* Diffuse Illumination Color */
   Color       Ks;		/* Specular Illumination Color */
   Color       Kt;		/* Transparent Illumination Color */
   Lfloat      shine;		/* Phong Specular Reflection Factor */
   Lfloat      indexref;	/* Index of Refraction */
   uchar       flags;		/* Attribute Flags */
};

	/* Info Flag Definitions */

#define  FL_GOURAUD  (1<<0)	/* Set if the Object is Gouraud Shaded */
#define  FL_PHONG    (1<<1)	/* Set if the Object is Phong Shaded */


struct S_OBJINFO
{  ObjInfo    *next;		/* Pointer to Next Object */
   Attributes *attr;		/* Object Attributes */
   uchar       type;		/* Object Type */
   uchar       flags;		/* Information Flags */
   boolean   (*intersect)	/* Intersection Function */
		ARGS((ObjInfo*, Point4, Vector4, Lfloat*, Point4, Vector4));
};

struct S_SPHERE
{  ObjInfo  info;	/* Common Object Fields; Must Be First Field */
   Point4   center;	/* Sphere Center */
   Lfloat   radius;	/* Sphere Radius */
   Lfloat   rsqrd;	/* Sphere Radius, Squared */
};

struct S_TETPAR		   /* Tetrahedron/Parallelepiped Common Fields */
{  Point4   vert[4];	   /* Vertices */
   Vector4  vec1,vec2,vec3;/* Vectors from Vertex 0 to Vertices 1,2,3 */
   Vector4  normal;	   /* Hyperplane Normal Vector */
   uchar    ax1, ax2, ax3; /* Non-Dominant Normal Vector Axes */
   Lfloat   planeConst;	   /* Hyperplane Constant */
   Lfloat   CramerDiv;	   /* Cramer's-Rule Divisor for Barycentric Coords */
};

struct S_TETRAHEDRON
{  ObjInfo  info;		/* Common Obj Fields; Must Be First Field */
   TetPar   tp;			/* Tetrahedron/Parallelepiped Data */
   Lfloat   Bc1, Bc2, Bc3;	/* Barycentric Coordinate Values */
};

struct S_PARALLELEPIPED
{  ObjInfo  info;	/* Common Obj Fields; Must Be First Field */
   TetPar   tp;		/* Tetrahedron/Parallelepiped Data */
};

struct S_TRIANGLE
{  ObjInfo  info;	/* Common Object Fields; Must Be First Field */
   Point4   vert[3];	/* Triangle Vertices */
   Vector4  vec1, vec2;	/* vector from Vertex0 to Vertices 1,2. */
   Lfloat   Bc1, Bc2;	/* Barycentric Coordinate Values */
};



	/*=========================*/
	/*  Function Declarations  */
	/*=========================*/

void     CloseInput   ARGS((void));
void     CloseOutput  ARGS((void));
void     Halt         ARGS((char *));
boolean  HitSphere    ARGS((ObjInfo*,Point4,Vector4,Lfloat*,Point4,Vector4));
boolean  HitTetPar    ARGS((ObjInfo*,Point4,Vector4,Lfloat*,Point4,Vector4));
boolean  HitTriangle  ARGS((ObjInfo*,Point4,Vector4,Lfloat*,Point4,Vector4));
char    *MyAlloc      ARGS((unsigned long));
void     MyFree       ARGS((char *));
void     OpenInput    ARGS((void));
void     OpenOutput   ARGS((void));
void     ParseInput   ARGS((void));
void     RayTrace     ARGS((Point4, Vector4, Color*, ulong));
short    ReadChar     ARGS((void));
void     WriteBlock   ARGS((char *, ulong));

#endif
