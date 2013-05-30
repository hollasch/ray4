
/***********************************************************************
**
**  "wire4" is Copyright (c) 1991 by Steve R. Hollasch.
**
**  All rights reserved.  This software may be freely copied, modified
**  and redistributed, provided that this copyright notice is preserved
**  in all copies.  This software is provided "as is", without express
**  or implied warranty.  You may not include this software in a program
**  or other software product without also supplying the source, or
**  without informing the end-user that the source is available for no
**  extra charge.  If you modify this software, please include a notice
**  detailing the author, date and purpose of the modification.
**
***********************************************************************/

/****************************************************************************
**
**  File:  wire4.h
**
**      This is the main header file for the 4D wireframe display program.
**  It includes the definitions and declarations for the wire4 C source
**  modules.
**
**  Revisions:
**
**  0.02  15-Jul-91  Hollasch, Ferneau, Marty Ryan 
**        Removed the SUBPIXEL macro and just dummied the subpixel()
**        function.  This now keys off of the GL_LEVEL compile macro.
**
**  0.01  12-Jul-91  Hollasch, Mark Ferneau
**        Added the SUBPIXEL() macro definition.  Changed transendental
**        function macros from keying off the "sgi" define to the "SVR0"
**        define.
**
**  0.00  12-Mar-91  Steve R. Hollasch
**
****************************************************************************/


#if defined (FLOAT_TRAN)		/* Workaround for improper     */
#  define acos(x)   _lacos(x)		/* (single precision)          */
#  define asin(x)   _lasin(x)		/* transendental function      */
#  define atan(x)   _latan(x)		/* parameter and return types. */
#  define atof(s)   _latof(s)
#  define ceil(x)   _lceil(x)
#  define cos(x)    _lcos(x)
#  define exp(x)    _lexp(x)
#  define fabs(x)   _lfabs(x)
#  define floor(x)  _lfloor(x)
#  define log(x)    _llog(x)
#  define pow(x,y)  _lpow(x,y)
#  define sin(x)    _lsin(x)
#  define sqrt(x)   _lsqrt(x)
#  define tan(x)    _ltan(x)
#endif


   /************************/
   /***  Debug Switches  ***/
   /************************/

#ifndef DB_DEPTHCUE		/* Depthcue Debugging Output Switch */
#  define DB_DEPTHCUE 0
#endif


   /****************************/
   /***  Program Parameters  ***/
   /****************************/

#define  EPSILON  (1e-10)       /* Small Number for Floating-Point Calcs */


   /***************************/
   /***  Defined Constants  ***/
   /***************************/

#define PI	3.14159265358979323846

#define true	(boolean)(1)
#define false	(boolean)(0)


        /* Status Box Dimensions */

#define  AXES_WIDTH     136
#define  SBOX_WIDTH     136
#define  SBOX_HEIGHT    40


        /* ViewMode Values */

#define  PERSPECTIVE  1		/* Perspective-Projection */
#define  PARALLEL     2		/* Parallel-Projection */
#define  AUTOSIZE     3		/* Automatic-Sized Perspective Projection */

#define  XMAX  XMAXSCREEN
#define  YMAX  YMAXSCREEN

#if (XMAX > YMAX)		/* MAXSCREEN is the largest box */
#  define MAXSCREEN	YMAX	/* (in pixels) that can be	*/
#else				/* displayed on the screen.	*/
#  define MAXSCREEN	XMAX
#endif

#define XCENTER ((XMAXSCREEN+1-SBOX_WIDTH)/2) /* Screen Center Coordinates */
#define YCENTER ((YMAXSCREEN+1)/2)

#define OBJ_VSTART   8	/* Start of Object Vertices */
#define OBJ_ESTART  12	/* Start of Object Edges */


      /* Axes Display Box Constants and Macros */

#define  AXES_INTERIOR(x,y)	(x),(y),(x)+AXES_WIDTH-1,(y)+AXES_WIDTH-1
#define  AXES_OUTLINE(x,y)	(x)-1,(y)-1,(x)+AXES_WIDTH,(y)+AXES_WIDTH

#define  AXES3D_X	(XMAX-1-AXES_WIDTH)	/* 4D Axes Display Box */
#define  AXES3D_Y	(YMAX-1-AXES_WIDTH)

#define  AXES4D_X	(XMAX-1-AXES_WIDTH)	/* 3D Axes Display Box */
#define  AXES4D_Y	(AXES3D_Y-AXES_WIDTH-10)


      /* Status Display Box Constants and Macros */

#define  SBOX_INTERIOR(x,y)   (x),(y),(x)+SBOX_WIDTH-1,(y)+SBOX_HEIGHT-1
#define  SBOX_OUTLINE(x,y)    (x)-1,(y)-1,(x)+SBOX_WIDTH,(y)+SBOX_HEIGHT

#define  SB_3DPROJ_X	(XMAX-SBOX_WIDTH)	/* 4D Projection Status */
#define  SB_3DPROJ_Y	(AXES4D_Y-25-SBOX_HEIGHT)

#define  SB_4DPROJ_X	(XMAX-SBOX_WIDTH)	/* 3D Projection Status */
#define  SB_4DPROJ_Y	(SB_3DPROJ_Y-10-SBOX_HEIGHT)

#define  SB_ROTSPC_X	(XMAX-SBOX_WIDTH)    /* Rotation Dimension Status */
#define  SB_ROTSPC_Y	(SB_4DPROJ_Y-10-SBOX_HEIGHT)

#define  SB_ROTPLN_X	(XMAX-SBOX_WIDTH)    /* Rotation Plane Status */
#define  SB_ROTPLN_Y	(SB_ROTSPC_Y-10-SBOX_HEIGHT)


      /* Keyboard Menu Location */

#define  KEYMENU_X	(XMAX-SBOX_WIDTH)	/* Key Menu Left Side */
#define  KEYMENU_Y	1			/* Key Menu Bottom Side */


   /***************************/
   /***  Macro Definitions  ***/
   /***************************/

#define  Radians(degrees)	(((degrees) / 180.0) * PI)

#define  streqic(s1,s2)		(strcmpic(s1,s2) == 0)

#define  MAX(x,y)		((x < y) ? (y) : (x))
#define  ABS(value)		(((value) > 0) ? (value) :-(value))

#ifdef _STDC_
#   define ARGS(arglist)	arglist
#else
#   define ARGS(arglist)	()
#endif

#if (GL_LEVEL < 1)
#   define  subpixel(x)		/* empty macro */
#endif


	/**************************/
	/***  Type Definitions  ***/
	/**************************/

#ifndef _AIX
   typedef unsigned char   uchar;	/* Unsigned  8-bit Integer */
   typedef unsigned short  ushort;	/* Unsigned 16-bit Integer */
   typedef unsigned long   ulong;	/* Unsigned 32-bit Integer */
#endif

typedef short  boolean;	/* Boolean; `true'(1) or `false'(0) */

#if !defined (Real)
#   define Real  double
#endif


   /*************************/
   /***  Data Structures  ***/
   /*************************/

typedef struct		/* Edge Between Two Vertices */
{  int         v1, v2;	/* Vertex Endpoints */
   Colorindex  color;	/* Color of the Edge */
} Edge;

typedef struct
{  Vector4  position;	/* 4D Vertex Position */
   Vector3  project3;	/* 3D Projection of the 4D Vertex */
   Real     x, y;	/* XY Screen Coordinates, Normalized */
   Real     depth;	/* 4D W Eye-Coordinate Depth (for Depthcueing) */
} Vertex;


   /**************************/
   /***  Global Variables  ***/
   /**************************/

#ifdef DEFINE_GLOBALS
#  define GLOBAL
#  define INIT(x)   = x
#else
#  define GLOBAL    extern
#  define INIT(x)
#endif


GLOBAL Real     Data4Radius;		/* 4D Data Radius */
GLOBAL boolean  DepthCue;		/* DepthCue Switch */
GLOBAL RGBvalue dc_ColorFar[3];		/* Far DepthCue Color */
GLOBAL RGBvalue dc_ColorNear[3];	/* Near DepthCue Color */
GLOBAL Real     dc_DepthFar;		/* Far DepthCue Distance */
GLOBAL Real     dc_DepthNear;		/* Near DepthCue Distance */
GLOBAL Real     dc_DFar_DNear;		/* Depthcue Range */
GLOBAL short    dc_Levels;		/* Number of DepthCue Levels */
GLOBAL Real     Dist3Auto;		/* Auto-Size View Distance, 3D */
GLOBAL Real     Dist3Initial;		/* Initial View Distance, 3D */
GLOBAL Real     Dist4Auto;		/* Auto-Size View Distance, 4D */
GLOBAL Real     Dist4Initial;		/* Initial View Distance, 4D */
GLOBAL Edge    *EdgeList;		/* Object Edge Array */
GLOBAL ulong    NumEdges;		/* Number of Edges in EdgeList */
GLOBAL ulong    NumVerts;		/* Number of Vertices in VertList */
GLOBAL boolean  RefCube;		/* 3D Reference Cube Switch */
GLOBAL Real     Root3;			/* Square Root of Three */
GLOBAL ushort   RotSpace INIT(3);	/* Dimension of Rotation (3 or 4) */
GLOBAL ushort   RotPlane;		/* Rotation Plane, Index to `rp' */
GLOBAL Real     Tan2Vangle3;		/* Tangent of Vangle3/2 */
GLOBAL Real     Tan2Vangle4;		/* Tangent of Vangle4/2 */
GLOBAL Vector3  Va, Vb, Vc;		/* 3x3 Viewing Matrix */
GLOBAL Real     Vangle3  INIT(45.0);	/* 3D View Angle */
GLOBAL Real     Vangle4  INIT(45.0);	/* 4D View Angle */
GLOBAL Vertex  *VertList;		/* Object Vertex Array */
GLOBAL uchar    Project3    INIT(1);	/* 3D View Type */
GLOBAL uchar    Project4    INIT(1);	/* 4D View Type */
GLOBAL Vector4  Wa, Wb, Wc, Wd;		/* 4x4 Viewing Matrix */

#ifndef DEFINE_GLOBALS
   extern RGBvalue cmap[512][3];	/* Color Map */
   extern Vector3  From3;		/* 3D Viewpoint */
   extern Vector4  From4;		/* 4D Viewpoint */
   extern Vector4  Over4;		/* 4D Other Up View Vector */
   extern Vector3  To3;			/* 3D Target Point */
   extern Vector4  To4;			/* 4D Target Point */
   extern Vector3  Up3;			/* 3D Up Vector */
   extern Vector4  Up4;			/* 4D Up View Vector */
#else
   RGBvalue cmap[512][3] =
   {  {  0,  0,  0},   {120,120,120},   {  0,255,  0},   {255,255,  0},
      {  0,  0,255},   {255,  0,255},   {  0,255,255},   {255,255,255},
      {255,  0,  0},   {200,200,200},   {100,100,100},   {188,255,  0},
      {  0,  0,128},   {  0,128,128},   {  0,256,128},   {128,  0,128},
      {128,128,128},   {128,256,128},   {256,128,128},   {256,256,128},
      {  0,128,  0},   {  0,128,256},   {128,128,  0},   {128,128,256},
      {256,128,256},   {128,  0,  0},   {128,  0,256},   {128,256,  0},
      {128,256,256}
   };

   Vector3  From3 = {10., 11., 13.};      /* 3D Viewpoint */
   Vector4  From4 = {0., 0., 0., 100.};   /* 4D Viewpoint */
   Vector4  Over4 = {0., 1., 0.,   0.};   /* 4D Other Up View Vector */
   Vector3  To3   = { 0.,  0.,  0.};      /* 3D Target Point */
   Vector4  To4   = {0., 0., 0.,   0.};   /* 4D Target Point */
   Vector3  Up3   = { 0.,  0.,  1.};      /* 3D Up Vector */
   Vector4  Up4   = {0., 0., 1.,   0.};   /* 4D Up View Vector */
#endif


   /*******************************/
   /***  Function Declarations  ***/
   /*******************************/

void    CalcV3Matrix    ARGS((void));
void    CalcV4Matrix    ARGS((void));
void    Display3DAxes   ARGS((void));
void    Display4DAxes   ARGS((void));
void    DrawStatus      ARGS((void));
void    Halt            ARGS((char*));
void    InitDisplay     ARGS((void));
void    ProjectToScreen ARGS((void));
void    ProjectTo3D     ARGS((void));
void    ReadObject      ARGS((FILE*));
void    Rotate3View     ARGS((Real, Real, int, int, Vector3, Vector3,
                                Vector3));
void    Rotate4View     ARGS((Real, Real, int, int, Vector4, Vector4,
                                Vector4, Vector4));
void    SlapStatus      ARGS((int, int, char*));
void    UpdateScreen    ARGS((void));
