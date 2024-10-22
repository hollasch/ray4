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
**  File:  w4_geometry.c
**
**      This source file contains the procedures and variables involved in
**      the geometry of the 4D wireframe viewer.
**
**  Revisions:
**
**  1.00  19-Jan-92  Hollasch
**        First public domain release.
**
**  0.00  91-Mar-12  Steve R. Hollasch
**
****************************************************************************/

#include <stdio.h>
#include <gl.h>
#include <vector.h>

#include "wire4.h"



/****************************************************************************
//  This function computes the three basis vectors for the 3D viewing matrix,
//  Va, Vb, and Vc.  This function uses the following global variables:
****************************************************************************/

void  CalcV3Matrix  ()
{
   auto Real norm;    /* Vector Scalar Magnitude */

   /* Get the Normalized Vc Vector */

   V3_3Vec (Vc,=,To3,-,From3);
   norm = V3_Norm (Vc);
   if (norm == 0.0)  Halt ("To Point and From Point are the same");
   V3_Scalar (Vc, /=, norm);

   /* Calculate the Va Vector */

   V3_Cross (Va, Vc, Up3);
   norm = V3_Norm (Va);
   if (norm == 0.0)  Halt ("Invalid Up Vector");
   V3_Scalar (Va, /=, norm);

   /* Calculate the Vb Vector.  Note that since Va and Vc are unit vectors,
   // Vb will also be a unit vector.  */

   V3_Cross (Vb, Va,Vc);
}



/****************************************************************************
//  This procedure computes the four basis vectors for the 4D viewing matrix,
//  Ua,Ub,Uc, and Ud.  Note that the Up vector transforms to Wb, the Over
//  vector transforms to Wc, and the line of sight transforms to Wd.  The
//  Wa vector is then computed from Wb,Wc and Wd.
****************************************************************************/

void  CalcV4Matrix  ()
{
   auto Real norm;    /* Vector Scalar Magnitude */

   /* Calculate Wd, the 4th coordinate basis vector and line-of-sight. */

   V4_3Vec (Wd, =, To4, -, From4);
   norm = V4_Norm (Wd);
   if (norm < EPSILON)
      Halt ("4D To Point and From Point are the same");
   V4_Scalar (Wd, /=, norm);

   /* Calculate Wa, the X-axis basis vector. */

   V4_Cross (Wa, Up4,Over4,Wd);
   norm = V4_Norm (Wd);
   if (norm < EPSILON)
      Halt ("4D up, over and view vectors are not perpendicular");
   V4_Scalar (Wa, /=, norm);

   /* Calculate Wb, the perpendicularized Up vector. */

   V4_Cross (Wb, Over4,Wd,Wa);
   norm = V4_Norm (Wb);
   if (norm < EPSILON)
      Halt ("Invalid 4D over vector");
   V4_Scalar (Wb, /=, norm);

   /* Calculate Wc, the perpendicularized Over vector.  Note that the
   // resulting vector is already normalized, since Wa, Wb and Wd are all
   // unit vectors.  */

   V4_Cross (Wc, Wd,Wa,Wb);
}



/****************************************************************************
//  This procedure takes the 3D vertex projections and projects them to the
//  the screen coordinates.
****************************************************************************/

void  ProjectToScreen  ()
{
   register int      ii;        /* Loop Index Variable */
   auto     Real     pconst;    /* Projection Constant For All Verts */
   auto     Real     rtemp;     /* Scratch Real-Value */
   auto     Vector3  TempV;     /* Scratch Vector */
   register Vertex  *vert;      /* Vertex List Traversal Pointer */

   if (Project3 == PARALLEL)
      rtemp  = 1.0 / Root3;
   else
      pconst = 1.0 / Tan2Vangle3;

   ii   = RefCube ? 0 : OBJ_VSTART;
   vert = &VertList[ii];

   while (ii < NumVerts)
   {
      V3_3Vec (TempV, =, vert->project3, -, From3);

      if (Project3 != PARALLEL)
         rtemp = pconst / V3_Dot (TempV, Vc);

      vert->x = rtemp * V3_Dot (TempV, Va);
      vert->y = rtemp * V3_Dot (TempV, Vb);

      ++vert; ++ii;
   }
}



/****************************************************************************
//  This procecure projects the 4D vertices to the 3D unit cube via the 4D
//  viewing parameters.
****************************************************************************/

void  ProjectTo3D  ()
{
   auto     Real     pconst;    /* Projection Constant */
   register int      ii;        /* Loop Index Variable */
   auto     Real     rtemp;     /* Real-Valued Temporary Variable */
   auto     Vector4  TempV;     /* Scratch Vector */
   register Vertex  *vert;      /* Vertex List Traversal Pointer */

   if (Project4 == PARALLEL)
      rtemp  = 1.0 / Data4Radius;
   else
      pconst = 1.0 / Tan2Vangle4;

   ii   = OBJ_VSTART;
   vert = &VertList[ii];

   while (ii < NumVerts)
   {
      /* Transform the vertices from world coordinates to eye coordinates. */

      V4_3Vec (TempV, =, vert->position, -, From4);

      vert->depth = V4_Dot (TempV, Wd);

      if (Project4 != PARALLEL)
         rtemp = pconst / vert->depth;

      vert->project3[0] = rtemp * V4_Dot (TempV, Wa);
      vert->project3[1] = rtemp * V4_Dot (TempV, Wb);
      vert->project3[2] = rtemp * V4_Dot (TempV, Wc);

      ++vert; ++ii;
   }
}



/****************************************************************************
//  This procedure rotates the 3D viewpoint and the up vector.
****************************************************************************/

void  Rotate3View  (Cos, Sin, rp1, rp2, To, From, Up)
   Real     Cos, Sin;   /* Cosine and Sine of the Rotation Angle */
   int      rp1, rp2;   /* The Coordinate Indices of the Rotation Plane */
   Vector3  To;         /* To Point (Rotate About This Point) */
   Vector3  From, Up;   /* The 3D From and Up Vectors */
{
   auto Real    t1, t2; /* Temporary Real Values */

   /* Rotate the from-vector. */

   t1 = Cos * (From[rp1]-To[rp1])  +  Sin * (From[rp2]-To[rp2]);
   t2 = Cos * (From[rp2]-To[rp2])  -  Sin * (From[rp1]-To[rp1]);
   From[rp1] = t1 + To[rp1];
   From[rp2] = t2 + To[rp2];

   /* Rotate the Up Vector */

   t1 = Cos * Up[rp1]  +  Sin * Up[rp2];
   t2 = Cos * Up[rp2]  -  Sin * Up[rp1];
   Up[rp1] = t1;
   Up[rp2] = t2;

   CalcV3Matrix ();
   ProjectToScreen ();
}



/****************************************************************************
//  This procedure rotates the 4D viewpoint, 4D Up vector and 4D over vector.
****************************************************************************/

void  Rotate4View  (Cos, Sin, rp1, rp2, To, From, Up, Over)
   Real     Cos, Sin;           /* Cosine and Sine of the Rotation Angle */
   int      rp1, rp2;           /* Coordinate Indices of Rotation Plane */
   Vector4  To;                 /* To Point (Rotate About This Point) */
   Vector4  From, Up, Over;     /* The 4D From, Up and Over Vectors */
{
   auto Real  t1, t2;         /* Temporary Real Values */

   /* Rotate the from-vector. */

   t1 = Cos * (From[rp1]-To[rp1])  +  Sin * (From[rp2]-To[rp2]);
   t2 = Cos * (From[rp2]-To[rp2])  -  Sin * (From[rp1]-To[rp1]);
   From[rp1] = t1 + To[rp1];
   From[rp2] = t2 + To[rp2];

   /* Rotate the Up Vector. */

   t1 = Cos * Up[rp1]  +  Sin * Up[rp2];
   t2 = Cos * Up[rp2]  -  Sin * Up[rp1];
   Up[rp1] = t1;
   Up[rp2] = t2;

   /* Rotate the Over Vector */

   t1 = Cos * Over[rp1]  +  Sin * Over[rp2];
   t2 = Cos * Over[rp2]  -  Sin * Over[rp1];
   Over[rp1] = t1;
   Over[rp2] = t2;

   CalcV4Matrix ();
   ProjectTo3D ();
   ProjectToScreen ();
}
