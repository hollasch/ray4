
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
**  File:  w4_display.c
**
**      This file contains the display procedures for the wire4 4D wireframe
**      display program.
**
**  Revisions:
**
**  1.00  19-Jan-92  Hollasch
**        First public domain release.
**
**  0.02  17-Jul-91  Hollasch
**        Changed SUBPIXEL() call to subpixel().  Rewrote the line-drawing
**        routines to use the more advanced GL functions based on the
**        defined value of GL_LEVEL.
**
**  0.01   7-Jul-91  Hollasch
**        Cleared both display buffers in the initialization routine.
**        Added SUBPIXEL() call for VGX and other workstations.
**
**  0.00  12-Mar-91  Steve R. Hollasch
**        Initial version.
**
****************************************************************************/

#include <stdio.h>
#include <gl.h>
#include <device.h>

#include <vector.h>
#include "wire4.h"


   /***************************/
   /***  Defined Constants  ***/
   /***************************/

      /* Write Masks */

#define  MASK_NORMAL   0x1FF    /* Normal Image Writemask */
#define  MASK_STATUS   0xE00    /* Status Colors Writemask */

      /* Status Display Colors */

#define  COLOR_BLACK        0,  0,  0   /* Black */
#define  COLOR_OUTLINE    230,175,100   /* Status Box Outline */
#define  COLOR_AXES        50,220, 65   /* Coordinate Axes */
#define  COLOR_HILIGHT    230, 50, 20   /* High Light Color */
#define  COLOR_BACKGRND   100,  0, 60   /* Status Box Background */
#define  COLOR_TEXT       110,110,110   /* Status Text */
#define  COLOR_OTHER      200, 50,200   /* Other (unused) */

      /* Status Pens */

#define  PEN_BLACK        0x200
#define  PEN_OUTLINE      0x400
#define  PEN_AXES         0x600
#define  PEN_HILIGHT      0x800
#define  PEN_BACKGRND     0xA00
#define  PEN_TEXT         0xC00
#define  PEN_OTHER        0xE00


   /***************************/
   /***  Macro Definitions  ***/
   /***************************/

#define SCREENX(V)	((long)(XCENTER + ((V)->x * (long)(MAXSCREEN/2))))
#define SCREENY(V)	((long)(YCENTER + ((V)->y * (long)(MAXSCREEN/2))))


   /*************************************/
   /***  Local Function Declarations  ***/
   /*************************************/

static void  LineDepthCue ARGS((Vertex*, Vertex*));
static void  LineRegular  ARGS((Vertex*, Vertex*));


   /**************************/
   /***  Global Variables  ***/
   /**************************/

static char   *coordch[] =	/* Coordinate Characters */
{ "X", "Y", "Z", "W" };



/****************************************************************************
//  This routine initializes the Silicon Graphics Iris display.
****************************************************************************/

void  InitDisplay  ()
{
   register short ii;	/* Loop Variable */

   /* Initialize the graphics display. */

   cursoff ();		/* Turn the cursor display off. */
   doublebuffer ();	/* Initialize for double-buffered display. */
   onemap ();		/* Use a common color map. */
   gconfig ();		/* Now use the current configuration. */
   subpixel(TRUE);	/* For increased VGX performance. */

   /* Set up the color maps for the status palette. */

   for (ii=0;  ii < 0x1FF;  ++ii)
   {  mapcolor (ii | PEN_BLACK,   COLOR_BLACK);
      mapcolor (ii | PEN_OUTLINE, COLOR_OUTLINE);
      mapcolor (ii | PEN_AXES,    COLOR_AXES);
      mapcolor (ii | PEN_HILIGHT, COLOR_HILIGHT);
      mapcolor (ii | PEN_BACKGRND,COLOR_BACKGRND);
      mapcolor (ii | PEN_TEXT,    COLOR_TEXT);
      mapcolor (ii | PEN_OTHER,   COLOR_OTHER);
   }

   /* Set up the wireframe object color map. */

   for (ii=0;  ii < 512;  ++ii)
      mapcolor (ii, cmap[ii][0], cmap[ii][1], cmap[ii][2]);

   /* Clear both display buffers. */

   color (BLACK);  clear ();  swapbuffers ();
   color (BLACK);  clear ();
}



/****************************************************************************
//  This routine draws the axis display boxes, the status boxes, and the
//  keyboard menu box on the current screen buffer.
****************************************************************************/

void  DrawStatus  ()
{
   /* Set up the drawing modes to write the status colors, and to write to
   // both the foreground and the background buffers.  */

   writemask (MASK_STATUS);
   frontbuffer (1);

   /* Draw the axes' display boxes. */

   color  (PEN_BLACK);
   rectfi (AXES_INTERIOR (AXES3D_X,AXES3D_Y));
   rectfi (AXES_INTERIOR (AXES4D_X,AXES4D_Y));

   color (PEN_OUTLINE);
   recti (AXES_OUTLINE (AXES3D_X,AXES3D_Y));
   recti (AXES_OUTLINE (AXES4D_X,AXES4D_Y));

   Display4DAxes ();
   Display3DAxes ();

   /* Draw the status boxes. */

   color  (PEN_BACKGRND);
   rectfi (SBOX_INTERIOR (SB_4DPROJ_X, SB_4DPROJ_Y));
   rectfi (SBOX_INTERIOR (SB_3DPROJ_X, SB_3DPROJ_Y));
   rectfi (SBOX_INTERIOR (SB_ROTSPC_X, SB_ROTSPC_Y));
   rectfi (SBOX_INTERIOR (SB_ROTPLN_X, SB_ROTPLN_Y));

   color (PEN_TEXT);
   recti (SBOX_OUTLINE (SB_4DPROJ_X, SB_4DPROJ_Y));
   recti (SBOX_OUTLINE (SB_3DPROJ_X, SB_3DPROJ_Y));
   recti (SBOX_OUTLINE (SB_ROTSPC_X, SB_ROTSPC_Y));
   recti (SBOX_OUTLINE (SB_ROTPLN_X, SB_ROTPLN_Y));

   /* Draw the status box headers. */

   cmov2i (SB_4DPROJ_X+6, SB_4DPROJ_Y+22);    charstr ("4D Projection");
   cmov2i (SB_3DPROJ_X+6, SB_3DPROJ_Y+22);    charstr ("3D Projection");
   cmov2i (SB_ROTSPC_X+6, SB_ROTSPC_Y+22);    charstr ("Rotation Space");
   cmov2i (SB_ROTPLN_X+6, SB_ROTPLN_Y+22);    charstr ("Rotation Plane");

   /* Draw the status box values. */

   SlapStatus (SB_4DPROJ_X, SB_4DPROJ_Y, "Perspective");
   SlapStatus (SB_3DPROJ_X, SB_3DPROJ_Y, "Perspective");
   SlapStatus (SB_ROTSPC_X, SB_ROTSPC_Y, "3D");
   SlapStatus (SB_ROTPLN_X, SB_ROTPLN_Y, "XY");

   /* Draw the keyboard menu box. */

#  define NUM_MENU  8	/* Number of Keyboard Menu Lines */

   color  (PEN_BACKGRND);
   rectfi (KEYMENU_X, KEYMENU_Y,
   	   KEYMENU_X + SBOX_WIDTH - 1, KEYMENU_Y + 16*(NUM_MENU+1) - 1);
   color  (PEN_TEXT);
   recti  (KEYMENU_X-1, KEYMENU_Y-1,
	   KEYMENU_X + SBOX_WIDTH, KEYMENU_Y + 16*(NUM_MENU+1));

   /* Draw the keyboard menu items. */

   cmov2i (KEYMENU_X+14, KEYMENU_Y+8 + (7*16));    charstr ("3: 3D Proj.");
   cmov2i (KEYMENU_X+14, KEYMENU_Y+8 + (6*16));    charstr ("4: 4D Proj.");
   cmov2i (KEYMENU_X+14, KEYMENU_Y+8 + (5*16));    charstr ("D: Depth Cue");
   cmov2i (KEYMENU_X+14, KEYMENU_Y+8 + (4*16));    charstr ("R: Ref Cube");
   cmov2i (KEYMENU_X+14, KEYMENU_Y+8 + (3*16));    charstr ("P: Rot Plane");
   cmov2i (KEYMENU_X+14, KEYMENU_Y+8 + (2*16));    charstr ("S: Rot Space");
   cmov2i (KEYMENU_X+14, KEYMENU_Y+8 + (1*16));    charstr ("C: Capture");
   cmov2i (KEYMENU_X+14, KEYMENU_Y+8 + (0*16));    charstr ("Q: Quit");
}



/****************************************************************************
//  This subroutine displays the given string in the status box given by
//  the X and Y coordinates.  The string is displayed in the status box
//  status area, and is highlighted & centered.
****************************************************************************/

void  SlapStatus  (sbx, sby, text)
   int   sbx, sby;              /* Status Box Lower Left Corner Coords */
   char *text;                  /* New Text */
{
   /* Update the status box in both buffers (front & back). */

   frontbuffer (1);
   writemask   (MASK_STATUS);

   /* Erase the current button status */

   color (PEN_BACKGRND);
   rectfi (sbx, sby + 4, sbx + SBOX_WIDTH - 1, sby + 15);

   /* Write the new status, centered. */

   color (PEN_HILIGHT);
   cmov2i (sbx + ((SBOX_WIDTH - (strlen(text)*9)) / 2), sby + 6);
   charstr (text);
}



/****************************************************************************
//  This procedure displays the object on the IRIS screen from the vertex
//  screen coordinates.
****************************************************************************/

void  UpdateScreen  ()
{
   register int  ii;           	/* Index Counter */
   register void (*LineFunc)();	/* Line-Drawing Function */

   /* Clear the screen before drawing. */

   writemask (MASK_NORMAL);
   frontbuffer (0);
   color (BLACK);
   clear ();

   /* Display each edge of the object. */

   if (RefCube)
   {  for (ii=0;  ii < OBJ_ESTART;  ++ii)
      {  color (EdgeList[ii].color);
         LineRegular (VertList+EdgeList[ii].v1, VertList+EdgeList[ii].v2);
      }
   }

   /* Set the line-drawing functions to either depthcued lines or to
   // solid-shaded lines. */

   if (DepthCue)
      LineFunc = LineDepthCue;
   else
      LineFunc = LineRegular;

   /* Draw each edge of the wireframe. */

   for (ii=OBJ_ESTART;  ii < NumEdges;  ++ii)
   {  color (EdgeList[ii].color);
      (*LineFunc) (VertList + EdgeList[ii].v1, VertList + EdgeList[ii].v2);
   }

   /* Finally, slap the current display buffer to the screen. */

   swapbuffers ();
}



/****************************************************************************
//  This procedure displays the 3D coordinate axes in the status box.
//  This function leaves the frontbuffer enbabled and the writemask set to
//  the status palette.
****************************************************************************/

void  Display3DAxes  ()
{
   auto     Vector3 Axis;       /* Coordinate Axis */
   register int     ii;         /* Index Variable */
   auto     Real    pconst;     /* Projection Constant */
   auto     Real    rtemp;      /* Temporary Real */
   auto     Real    x,y;        /* Screen Coordinates of Axes */

   /* Write into both the front and the back buffers. */

   frontbuffer(1);
   writemask (MASK_STATUS);

   /* Clear the 3D axes display box. */

   color  (PEN_BLACK);
   rectfi (AXES_INTERIOR (AXES3D_X, AXES3D_Y));

   pconst = (Real)(AXES_WIDTH/2 - 5) / tan (Radians(Vangle3/2.0));

   /* The following loop draws each of the three 3D basis axes. */

   for (ii=0;  ii < 3;  ++ii)
   {
      V3_2Vec (Axis, =, -From3);
      Axis[ii] += 1.0;
      rtemp = pconst / V3_Dot (Axis, Vc);
      x = rtemp * V3_Dot(Axis,Va);      /* Calculate screen coords. */
      y = rtemp * V3_Dot(Axis,Vb);

      /* If the axis vector is more than (AXES_WIDTH/2-9) pixels long, then
      // scale it down to that length. */

      rtemp = MAX(ABS(x),ABS(y));
      if (rtemp > (AXES_WIDTH/2 - 12))
      {  x *= (AXES_WIDTH/2 - 12) / rtemp;
         y *= (AXES_WIDTH/2 - 12) / rtemp;
      }

      /* Display the axis. */

      color (PEN_AXES);
      move2i (AXES3D_X + (AXES_WIDTH/2), AXES3D_Y + (AXES_WIDTH/2));
      rdr2i ((int)(x), (int)(y));

      if (x < 0.0)  x -= 10.0;  else x += 1.0;
      if (y < 0.0)  y -= 10.0;  else y += 1.0;

      color (PEN_HILIGHT);
      cmov2i (AXES3D_X + (AXES_WIDTH/2) + (int)(x),
	      AXES3D_Y + (AXES_WIDTH/2) + (int)(y));
      charstr(coordch[ii]);
   }
}



/****************************************************************************
//  This procedure displays the 4D coordinate axes in the status box.
//  This function leaves the frontbuffer enbabled and the writemask set to
//  the status palette.
****************************************************************************/

void  Display4DAxes  ()
{
   auto      Vector3 Axis3;     /* Coordinate Axis; 3D Projection */
   auto      Vector4 Axis4;     /* Coordinate Axis; 4D */
   register  int     ii;        /* Index Variable */
   auto      Real    pconst3;   /* 3D Projection Constant */
   auto      Real    pconst4;   /* 4D Projection Constant */
   auto      Real    rtemp;     /* Temporary Real */
   auto      Real    x,y;       /* Screen Coordinates of Axes */

   /* Write into both the front and back buffers. */

   frontbuffer(1);
   writemask (MASK_STATUS);

   /* Clear the 4D axes display box. */

   color  (PEN_BLACK);
   rectfi (AXES_INTERIOR (AXES4D_X, AXES4D_Y));

   pconst4 =  1.0 / tan (Radians(Vangle4/2.0));
   pconst3 = 27.0 / tan (Radians(Vangle3/2.0));

   /* The following loop draws each of the four 4D basis axes. */

   for (ii=0;  ii < 4;  ++ii)
   {
      V4_2Vec (Axis4, =, -From4);
      Axis4[ii] += 1.0;
      rtemp = pconst4 / V4_Dot (Axis4, Wd);
      Axis3[0] = V4_Dot (Axis4, Wa) * rtemp;
      Axis3[1] = V4_Dot (Axis4, Wb) * rtemp;
      Axis3[2] = V4_Dot (Axis4, Wc) * rtemp;

      V3_2Vec (Axis3, -=, From3);
      rtemp = pconst3 / V3_Dot (Axis3, Vc);
      x = rtemp * V3_Dot(Axis3,Va);     /* Calculate screen coords. */
      y = rtemp * V3_Dot(Axis3,Vb);

      /* If the axis vector is too long, then scale it down to fit in
      // the box, along with the identifying character. */

      rtemp = MAX(ABS(x),ABS(y));
      if (rtemp > (AXES_WIDTH/2 - 12))
      {  x *= (AXES_WIDTH/2 - 12) / rtemp;
         y *= (AXES_WIDTH/2 - 12) / rtemp;
      }

      /* Display the axis.  Move to the center of the axis box, and then
      // draw the axis.  */

      color (PEN_AXES);
      move2i (AXES4D_X + (AXES_WIDTH/2), AXES4D_Y + (AXES_WIDTH/2));
      rdr2i ((int)(x), (int)(y));

      if (x < 0.0)  x -= 10.0;  else x += 1.0;
      if (y < 0.0)  y -= 10.0;  else y += 1.0;

      color (PEN_HILIGHT);
      cmov2i (AXES4D_X + (AXES_WIDTH/2) + (int)(x),
	      AXES4D_Y + (AXES_WIDTH/2) + (int)(y));
      charstr(coordch[ii]);
   }
}



/****************************************************************************
//  This function draws lines between two vertices using constant intensity
//  lines of the current color.
****************************************************************************/

static void  LineRegular  (P, Q)
   Vertex *P, *Q;		/* Endpoint Vertices */
{
   /* If the GL implementation level is greater than zero, then use the
   // bgnline()...v2i()...v2i()...endline() drawing sequence, otherwise
   // use the move2i()-draw2i() functions. */

#  if (GL_LEVEL < 1)
      move2i ((short) SCREENX(P), (short) SCREENY(P));
      draw2i ((short) SCREENX(Q), (short) SCREENY(Q));
#  else
      auto  long  pos[2];	/* Vertex Position */

      bgnline ();
      pos[0] = SCREENX(P);   pos[1] = SCREENY(P);   v2i(pos);
      pos[0] = SCREENX(Q);   pos[1] = SCREENY(Q);   v2i(pos);
      endline ();
#  endif
}


/****************************************************************************
//  This function draws lines between two vertices using depthcueing.  The
//  main approach used is to subdivide the edge into dc_Levels subsegments.
//  Each edge is shaded according to the depth of the subsegment midpoint.
****************************************************************************/

static void  LineDepthCue  (P, Q)
   Vertex *P, *Q;		/* Endpoint Vertices */
{
   auto     Real  dscale;	/* Depth Scale Factor */
   auto     long  Ps[2], Qs[2];	/* Screen Coordinates of Q & P */
   auto     long  Q_P[2];	/* X and Y components of Q - P */

   Ps[0]  = SCREENX (P);     Ps[1]  = SCREENY (P);
   Qs[0]  = SCREENX (Q);     Qs[1]  = SCREENY (Q);
   Q_P[0] = Qs[0] - Ps[0];   Q_P[1] = Qs[1] - Ps[1];
   dscale = (Real)(dc_Levels) / dc_DFar_DNear;

#  if (GL_LEVEL >= 2)
   {
      /* Draw the line segment with Gouraud shading, interpolating from
      // the depthcued color of the first vertex to the depthcued color
      // of the second vertex. */

      bgnline ();

      if (P->depth < dc_DepthNear)
	 color ((Colorindex) (0x100));
      else if (P->depth > dc_DepthFar)
	 color ((Colorindex) (0x100 + dc_Levels - 1));
      else
	 color ((Colorindex) (0x100 + (dscale * (P->depth - dc_DepthNear))));
      v2i (Ps);

      if (Q->depth < dc_DepthNear)
	 color ((Colorindex) (0x100));
      else if (Q->depth > dc_DepthFar)
	 color ((Colorindex) (0x100 + dc_Levels - 1));
      else
	 color ((Colorindex) (0x100 + (dscale * (Q->depth - dc_DepthNear))));
      v2i (Qs);

      endline ();
   }
#  else
   {
      auto     Real   QPdepth;	/* Qdepth - Pdepth */
      auto     Real   segdepth;	/* Segment Midpoint Depth */
      register short  tt;	/* Segment Index */

      QPdepth = Q->depth - P->depth;

#     if (GL_LEVEL <= 0)
         move2i ((short) Ps[0], (short) Ps[1]);
#     else
         bgnline ();
         v2i (Ps);
#     endif

      /* The following loop draws the edge in `dc_Levels' number of
      // individual segments, since the GL_LEVEL setting indicates that
      // the current platform cannot support Gouraud (blended) shading
      // from one vertex to the next. */

      for (tt = 0;  tt <= dc_Levels;  ++tt)
      {
         auto  long  seg[2];

	 /* Find the eye-depth of the current edge segment. */

         segdepth = P->depth
	       + (QPdepth * ((Real)((2*tt)-1) / (Real)(2*dc_Levels)));

	 /* Set the color of the current edge segment. */

	 if (segdepth < dc_DepthNear)
	    color ((Colorindex) (0x100));
	 else if (segdepth > dc_DepthFar)
	    color ((Colorindex) (0x100 + dc_Levels - 1));
	 else
	    color ((Colorindex) (0x100 + (dscale * (segdepth-dc_DepthNear))));

	 /* Set the endpoint coordinates of the current edge segment. */

         seg[0] = Ps[0] + ((tt * Q_P[0]) / dc_Levels);
         seg[1] = Ps[1] + ((tt * Q_P[1]) / dc_Levels);

	 /* Draw the current edge segment. */

#	 if (GL_LEVEL <= 0)
            draw2i ((short) seg[0], (short) seg[1]);
#	 else
	    v2i (seg);
#	 endif
      }

#     if (GL_LEVEL >= 1)
         endline ();
#     endif
   }
#  endif
}
