
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
**  File:  w4_main.c
**
**      This is the main source file for the 4D wireframe viewer.  For a
**      description of the input file format, see the header in w4_parse.c
**
**      The command line usage is:
**
**          wire4 < InputFile
**      or:
**          wire4 InputFile
**
**  Revisions:
**
**  1.00  19-Jan-92  Hollasch
**        First public domain release.
**
**  0.02  15-Jul-91  Hollasch
**        If GL_LEVEL is 2 or more, then set up for Gouraud shading while
**        in depthcue mode.  Also removed inclusion of steve.h and unix.h
**        from the wire4 package.
**
**  0.01  91-Jul-07  Hollasch, Marty Ryan
**        Clear both display buffers before starting, call qreset() before
**        event loop.
**
**  0.00  91-Mar-12  Steve R. Hollasch
**        Initial version.
**
****************************************************************************/

#include <stdio.h>
#include <gl.h>
#include <device.h>
#include <vector.h>

#define  DEFINE_GLOBALS
#include "wire4.h"


   /**************************/
   /***  Type Definitions  ***/
   /**************************/

typedef enum
{  EV_NONE,		/* No Mouse or Keyboard Input */
   EV_QUIT,		/* Quit Program */
   EV_CAPTURE,		/* Capture Image */
   EV_DEPTHCUE,		/* Depth-Cue Toggle */
   EV_REFCUBE,		/* 3D Reference Cube Toggle */
   EV_ROTPLANE,		/* Change Rotation Plane */
   EV_ROTSPACE,		/* Change Rotation Space */
   EV_PRJMODE3,		/* Change 3D Projection Mode */
   EV_PRJMODE4		/* Change 4D Projection Mode */
} EventType;


   /*******************************/
   /***  Function Declarations  ***/
   /*******************************/

static EventType  CheckEventQueue  ARGS((void));
static void       EventLoop        ARGS((void));
static void       Initialize       ARGS((void));
static void       Snapshot         ARGS((void));


   /**************************/
   /***  Global Variables  ***/
   /**************************/

      /* 3D Reference Cube Edges */

static Edge RefCubeEdge[] =
{  {0,1,1}, {1,2,1}, {2,3,1}, {3,0,1}, {4,5,1}, {5,6,1}, {6,7,1}, {7,4,1},
   {0,4,1}, {1,5,1}, {2,6,1}, {3,7,1}
};

      /* 3D Reference Cube Vertices */

static Vertex RefCubeVert[] =
{  {{0},{-1,-1,-1}},  {{0},{-1,-1, 1}},  {{0},{-1, 1, 1}},  {{0},{-1, 1,-1}},
   {{0},{ 1,-1,-1}},  {{0},{ 1,-1, 1}},  {{0},{ 1, 1, 1}},  {{0},{ 1, 1,-1}}
};

static ushort  rp[2][6] =	/* Rotation Planes [Axis] [Plane] */
{   { 0, 1, 2, 0, 1, 2 },	/* Plane Axis One */
    { 1, 2, 0, 3, 3, 3 },	/* Plane Axis Two */
};

static char   *rptext[] =	/* Rotation Plane Text */
{   "XY", "YZ", "ZX", "XW", "YW", "ZW"
};

static char  *snapname = "snapshots";



/****************************************************************************
//                          Main Program Entry Point
****************************************************************************/

void  main  (argc, argv)
   int   argc;
   char *argv[];
{
   auto  FILE  *input;		/* Input Stream */

   ginit ();    /* Initialize the graphics interface. */

   /* If the user did not supply a filename on the command line, then read
   // the input from the standard input stream.  */

   if (argc < 2)
      input = stdin;
   else
   {  input = fopen (argv[1], "r");
      if (input == NULL)
	 Halt ("Open failed for input file");
   }

   ReadObject (input);	/* Read in & build the 4D object structure. */
   fclose (input);

   Initialize ();	/* Miscellaneous Initialization */

   CalcV3Matrix ();	/* Calculate the viewing matrices. */
   CalcV4Matrix ();

   ProjectTo3D ();	/* Project the object for the initial display. */
   ProjectToScreen ();

   DrawStatus ();	/* Draw the initial status values. */

   EventLoop ();	/* Process user-interaction. */

   Halt (NULL);
}



/****************************************************************************
//  This routine initializes various fields after reading in the data
//  description.
****************************************************************************/

static void  Initialize  ()
{
   auto  long     ii;           /* Loop Counter */
   auto  short    rgb;		/* RGB Index */
   auto  Vector3  Temp3;        /* Temporary 3-Vector */
   auto  Vector4  Temp4;        /* Temporary 4-Vector */

   Root3 = sqrt((Real)(3.0));

   Tan2Vangle4 = tan (Radians(Vangle4/2.0));
   Tan2Vangle3 = tan (Radians(Vangle3/2.0));

   V3_3Vec (Temp3, =, From3, -, To3);
   Dist3Initial = V3_Norm (Temp3);
   Dist3Auto    = Root3 / Tan2Vangle3;

   /* Find the radius of the 4D data.  The radius of the 4D data is the
   // radius of the smallest enclosing sphere, centered at the To point.
   // Note that during the loop through the vertices, Data4Radius holds
   // the squared radius value.  */

   Data4Radius = 0.00;
   for (ii=OBJ_VSTART;  ii < NumVerts;  ++ii)
   {
      auto  Real  dist;	/* Current Vertex Distance From To4 Point */

      V4_3Vec (Temp4, =, VertList[ii].position, -, To4);
      dist = V4_Dot (Temp4,Temp4);
      if (dist > Data4Radius)
         Data4Radius = dist;
   }

   Data4Radius = sqrt (Data4Radius);

   V4_3Vec (Temp4, =, From4, -, To4);
   Dist4Initial = V4_Norm (Temp4);
   Dist4Auto    = Data4Radius / Tan2Vangle4;

   /* Set up the near and far depthcue distances. */

   if (dc_DepthNear < 0.0)
      dc_DepthNear = Dist4Initial - Data4Radius;

   if (dc_DepthFar  < 0.0)
      dc_DepthFar  = Dist4Initial + Data4Radius;

   dc_DFar_DNear = dc_DepthFar - dc_DepthNear;

   /* Initialize the 3D reference cube vertices and edges. */

   for (ii=0;  ii < OBJ_VSTART;  ++ii)
      VertList[ii] = RefCubeVert[ii];

   for (ii=0;  ii < OBJ_ESTART;  ++ii)
      EdgeList[ii] = RefCubeEdge[ii];

   /* Set up the depthcue color map. */

   for (rgb=0;  rgb < 3;  ++rgb)
   {  auto short range;		/* Color Component Range */

      range = (short) dc_ColorFar[rgb] - (short) dc_ColorNear[rgb];
      if (range < 0)
         --range;
      else
         ++range;

      for (ii=0;  ii < dc_Levels;  ++ii)
         cmap[0x100+ii][rgb] = (short) (dc_ColorNear[rgb])
	 		   + (short) ((ii * range) / dc_Levels);
   }

#  if DB_DEPTHCUE
   {
      register int ii;

      printf ("dc_Levels      %d\n", dc_Levels);
      printf ("dc_ColorNear   %d,%d,%d\n",
	dc_ColorNear[0],dc_ColorNear[1],dc_ColorNear[2]);
      printf ("dc_ColorFar    %d,%d,%d\n",
	dc_ColorFar[0],dc_ColorFar[1],dc_ColorFar[2]);
      printf ("dc_DepthNear   %lg\n", dc_DepthNear);
      printf ("dc_DepthFar    %lg\n", dc_DepthFar);
      printf ("dc_DFar_DNear  %lg\n", dc_DFar_DNear);

      for (ii=0x100;  ii < (0x100+dc_Levels);  ++ii)
         printf ("cmap[%d] is %d,%d,%d\n",
	    ii, cmap[ii][0],cmap[ii][1],cmap[ii][2]);
      
      printf ("\n");
   }
#  endif

   InitDisplay ();
}



/****************************************************************************
//  This routine checks the event queue and returns the type of event it
//  corresponds to.
****************************************************************************/

static EventType  CheckEventQueue  ()
{
   auto   short   data;		/* Event Queue Data */
   auto   long    device;	/* Device Type From the Event Queue */
   static boolean prjmode;	/* Mouse in Projection Selection Mode */

   CHECKQUEUE:

   if (!qtest())
      return EV_NONE;

   device = qread (&data);
   switch (device)
   {
      case KEYBD:
	 switch (data)
	 {
	    case '3':   			return EV_PRJMODE3;
	    case '4':   			return EV_PRJMODE4;
	    case 'c': case 'C':			return EV_CAPTURE;
	    case 'd': case 'D':			return EV_DEPTHCUE;
	    case 'p': case 'P':			return EV_ROTPLANE;
	    case 'q': case 'Q': case 0x1B:	return EV_QUIT;
	    case 'r': case 'R':			return EV_REFCUBE;
	    case 's': case 'S':			return EV_ROTSPACE;

	    default:    return EV_NONE;
	 }
	 break;

      case LEFTMOUSE:
	 if (data == 0)   /* If mouse button released, get next event. */
	    goto CHECKQUEUE;
	 return (prjmode) ? EV_PRJMODE3 : EV_ROTPLANE;

      case MIDDLEMOUSE:
	 if (data == 0)   /* If mouse button released, get next event. */
	    goto CHECKQUEUE;
	 return (prjmode) ? EV_PRJMODE4 : EV_ROTSPACE;

      case RIGHTMOUSE:
	 prjmode = data;
	 goto CHECKQUEUE;
	 break;

      default:
	 printf ("Error:  Unexpected event type (%ld:%d)\n", device,data);
	 return EV_NONE;
   }
}



/****************************************************************************
//  This routine is the main event loop for the wireframe viewer.  It
//  continues to loop on input until the user decides to quit, at which time
//  EventLoop will return.
****************************************************************************/

static void  EventLoop  ()
{
   auto short    data;    /* Mouse Position Data */
   auto short    lastx;   /* Last Mouse Horizontal Position */
   auto boolean  update;  /* If true, then update the screen. */

   qdevice (KEYBD);             /* Queue up keyboard events. */
   qdevice (LEFTMOUSE);         /* Queue up the mouse buttons. */
   qdevice (MIDDLEMOUSE);
   qdevice (RIGHTMOUSE);
   qreset  ();

   setvaluator (MOUSEX, 0, -32767, 32767);
   lastx = getvaluator (MOUSEX);

   update = true;

   for (;;)
   {
      /* Refresh the display, if needed. */

      if (update)
      {  UpdateScreen ();
	 update = false;
      }

      switch (CheckEventQueue())
      {
	 case EV_QUIT:
	    return;

	 case EV_CAPTURE:
	    Snapshot ();
	    break;

	 case EV_DEPTHCUE:
	    DepthCue = !DepthCue;
#	    if (GL_LEVEL >= 2)
	       shademodel (DepthCue ? GOURAUD : FLAT);
#	    endif
	    update = true;
	    break;

	 case EV_REFCUBE:
	    RefCube = !RefCube;
	    ProjectToScreen ();
	    update = true;
	    break;

	 case EV_ROTPLANE:
	    if (++RotPlane >= ((RotSpace == 3) ? 3 : 6))
	       RotPlane = 0;
	    SlapStatus (SB_ROTPLN_X, SB_ROTPLN_Y, rptext[RotPlane]);
	    break;

	 case EV_ROTSPACE:
	    if (RotSpace == 3)
	       RotSpace = 4;
	    else
	    {  RotSpace = 3;
	       if (RotPlane >= 3)
	          RotPlane -= 3;
	    }
	    SlapStatus (SB_ROTSPC_X, SB_ROTSPC_Y, (RotSpace==3)?"3D":"4D");
	    break;

	 case EV_PRJMODE3:
	 {
	    auto Vector3 ViewVector;    /* From To3-Point to From3-Point */

#	    if 0
	       auto Real    ViewScale;     /* View Vector Scale Value */

	       if (Project3 == PARALLEL)
	       {  Project3 = AUTOSIZE;
		  V3_3Vec (ViewVector, =, From3, -, To3);
		  ViewScale = Dist3Auto / Dist3Initial;
		  V3_3Vec (From3, =, To3, +, ViewScale * ViewVector);
	       }
	       else if (Project3 == PERSPECTIVE)
	       {  Project3 = PARALLEL;
	       }
	       else
	       {  Project3 = PERSPECTIVE;
		  V3_3Vec (ViewVector, =, From3, -, To3);
		  ViewScale = Dist3Initial / Dist3Auto;
		  V3_3Vec (From3, =, To3, +, ViewScale * ViewVector);
	       }

	       CalcV3Matrix ();
#	    else
	       if (Project3 == PARALLEL)
		  Project3 = PERSPECTIVE;
	       else
		  Project3 = PARALLEL;
#	    endif

	    ProjectToScreen ();
	    Display3DAxes ();
	    update = true;

	    SlapStatus (SB_3DPROJ_X,SB_3DPROJ_Y,
	       (Project3 == PARALLEL) ? "Parallel" :
	       (Project3 == AUTOSIZE) ? "AutoSize" : "Perspective");
	    break;
	 }

	 case EV_PRJMODE4:
	 {
	    auto Vector4 ViewVector;    /* From To3-Point to From3-Point */

#	    if 0
	       auto Real    ViewScale;     /* View Vector Scale Value */
	       if (Project4 == PARALLEL)
	       {  Project4 = AUTOSIZE;
		  V4_3Vec (ViewVector, =, From4, -, To4);
		  ViewScale = Dist4Auto / Dist4Initial;
		  V4_3Vec (From4, =, To4, +, ViewScale * ViewVector);
	       }
	       else if (Project4 == PERSPECTIVE)
	       {  Project4 = PARALLEL;
	       }
	       else
	       {  Project4 = PERSPECTIVE;
		  V4_3Vec (ViewVector, =, From4, -, To4);
		  ViewScale = Dist4Initial / Dist4Auto;
		  V4_3Vec (From4, =, To4, +, ViewScale * ViewVector);
	       }

	       CalcV4Matrix ();
#	    else
	       if (Project4 == PARALLEL)
		  Project4 = PERSPECTIVE;
	       else
		  Project4 = PARALLEL;
#	    endif

	    ProjectTo3D ();
	    ProjectToScreen ();
	    Display4DAxes ();
	    update = true;

	    SlapStatus (SB_4DPROJ_X,SB_4DPROJ_Y,
	       (Project4 == PARALLEL) ? "Parallel" :
	       (Project4 == AUTOSIZE) ? "AutoSize" : "Perspective");
	    break;
	 }

	 default:
	    break;
      }

      /* Poll the MOUSEX device to see if the mouse has moved horizontally.
      // If so, then we need to rotate the object by the amount the mouse
      // has moved from its last position.  */

      data = getvaluator (MOUSEX);

      if (data != lastx)
      {  auto Real  delta;

	 delta = ((Real)(data - lastx) / (4.0*360.0)) * PI;
	 lastx = data;

	 if (RotSpace == 3)
	 {  Rotate3View (cos(delta), sin(delta),
	       rp[0][RotPlane], rp[1][RotPlane], To3, From3, Up3);
	    Display3DAxes ();
	 }
	 else
	 {  Rotate4View (cos(delta), sin(delta),
	       rp[0][RotPlane], rp[1][RotPlane], To4, From4, Up4, Over4);
	    Display4DAxes ();
	 }

	 setvaluator (MOUSEX, 0, -32767, 32767);
	 lastx = getvaluator (MOUSEX);
	 update = true;
      }
   }
}



/****************************************************************************
//  This procedure halts the program.  If the program is aborting, a non-NULL
//  message is passed as the single parameter and the message is printed.  If
//  the program is terminating normally, then no message is printed and the
//  program exits with a zero return code.
****************************************************************************/

void  Halt  (message)
   char *message;       /* Reason for Program Termination */
{
   if (message) fprintf (stderr, "wire4:  %s.\n", message);

   if (EdgeList)  free (EdgeList);
   if (VertList)  free (VertList);

   setvaluator (MOUSEX, XMAXSCREEN/2, 0, XMAXSCREEN);

   gexit ();
   exit (message ? -1 : 0);
}



/****************************************************************************
//  This procedure appends the current wireframe to the snapshots file.
//  The viewing parameters are written along with the normalized (0 to 1)
//  coordinates of the wireframe image.
****************************************************************************/

static void  Snapshot  ()
{
   register ushort  ii;		/* Loop Variable */
   register FILE   *snap;	/* Snapshots File Handle */

   if (!(snap = fopen (snapname, "a")))
   {  fprintf (stderr, "wire4:  Couldn't open snapshots file (%s).",
         snapname);
      Halt ("Aborting");
   }

   fprintf(snap, "\n");
   fprintf(snap, "# From4    %8.4lf %8.4lf %8.4lf %8.4lf\n", V4_List(From4));
   fprintf(snap, "# To4      %8.4lf %8.4lf %8.4lf %8.4lf\n", V4_List(To4));
   fprintf(snap, "# Up4      %8.4lf %8.4lf %8.4lf %8.4lf\n", V4_List(Up4));
   fprintf(snap, "# Over4    %8.4lf %8.4lf %8.4lf %8.4lf\n", V4_List(Over4));
   fprintf(snap, "# Vangle4  %8.4lf\n", Vangle4);
   fprintf(snap, "# Project4 %s\n",
	(Project4 == PARALLEL) ? "Parallel" : "Perspective");
   fprintf(snap, "# From3    %8.4lf %8.4lf %8.4lf\n", V3_List(From3));
   fprintf(snap, "# To3      %8.4lf %8.4lf %8.4lf\n", V3_List(To3));
   fprintf(snap, "# Up3      %8.4lf %8.4lf %8.4lf\n", V3_List(Up3));
   fprintf(snap, "# Vangle3  %8.4lf\n", Vangle3);
   fprintf(snap, "# Project3 %s\n",
	(Project3 == PARALLEL) ? "Parallel" : "Perspective");
   fprintf(snap, "# RefCube %s\n", RefCube ? "On" : "Off");

   ii = RefCube ? 0 : OBJ_ESTART;
   while (ii < NumEdges)
   {
      fprintf (snap, "M %9.6lf %9.6lf\n",
	 (VertList[EdgeList[ii].v1].x + 1.0) / 2.0,
	 (VertList[EdgeList[ii].v1].y + 1.0) / 2.0
      );
      fprintf (snap, "D %9.6lf %9.6lf\n",
	 (VertList[EdgeList[ii].v2].x + 1.0) / 2.0,
	 (VertList[EdgeList[ii].v2].y + 1.0) / 2.0
      );
      ++ii;
   }

   fclose (snap);
}
