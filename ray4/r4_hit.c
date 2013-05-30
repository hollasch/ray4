
/***********************************************************************
**
**  "ray4" is Copyright (c) 1991,1992,1993 by Steve R. Hollasch.
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
**  File:  r4_hit.c
**
**        This file contains intersection routines for the geometric
**    primitives in the Ray4 4D raytracer.  See the r4_main.c header for
**    more information on Ray4.
**
**  Revisions:
**
**    1.00  25-Jan-92  Hollasch
**          Released to the public domain.
**
**    0.35  27-May-91  Hollasch
**          Fixed bug in tetrahedron/parallelepiped normal and intersection
**          assignment code; previous version did not ensure that the
**          passed parameter pointers were non-nil.
**
**    0.34  15-May-91  Hollasch
**          Fixed logic flaw in HitSphere t1/t2 selection.
**
**    0.33  12-May-91  Hollasch
**          Added hypersphere debug code.
**
**    0.20  19-Dec-90  Hollasch
**          Altered tetrahedron-intersection routine to accomodate both
**          tetrahedra and parallelepipeds.
**
**    0.13  13-Dec-90  Hollasch
**          Added internal intersection verification code for tetrahedra.
**
**    0.11  20-Nov-90  Hollasch
**          Fixed ray-triangle intersection; moved some variable computations
**          from on-the-fly to precomputed fields in structures for both
**          2D triangles and tetrahedrons.
**
**    0.10  15-Nov-90  Hollasch
**          Added tetrahedron & triangle intersection code, and altered
**          intersection routines to use the new mindist parameter.
**
**    0.00  30-Sep-90  Steve R. Hollasch
**          Initial version.
**
****************************************************************************/

#include <stdio.h>

#include "ray4.h"
#include "r4_globals.h"


	/*** Debugging Flags ***/

	/* 0: No debugging.
	** 1: Internal Validation Checks
	** 2: Output Debug Information */

#define  DB_TETPAR  0	/* Tetrahedra / Parallelepiped Debug Level */
#define  DB_SPHERE  0	/* Hypersphere Debug */


	/*** Defined Constants ***/

#define  MINDIST    1e-7	/* Minimum Intersection Distance (for the
				** elimination of surface acne. */




/****************************************************************************
**
**  All intersection functions must behave in the same manner, since they are
**  called as object-oriented functions that behave generically for different
**  objects.  Each function takes the same parameters and returns a boolean
**  value.  Operation of each function is as follows:
**
**  The functions take the following parameter list:
**
**      objptr  (ObjInfo*):  Pointer to the Object Structure
**      rayO    (Point4  ):  Ray Origin
**      rayD    (Vector4 ):  Ray Direction (Must Be A Unit Vector)
**      mindist (Real* ):    Closest Intersection Distance So Far
**      intr    (Point4  ):  Intersection Point
**      normal  (Vector4 ):  Surface Normal at Intersection Point
**
**  If the ray does not intersect the object, the intersection function
**  returns false and does not alter `mindist', `intr' or `normal'.
**
**  If the mindist parameter is nil, then it's assumed that the calling
**  program is not interested in the specifics of the intersection; it's
**  only interested in whether the ray intersects the object (as in shadow
**  testing).  In this case, if the ray does intersect the object, the
**  function immediately returns true, but does not alter the `mindist',
**  `intr' or `normal' parameters.
**
**  If the ray intersects the object, the intersection function returns true
**  and the `mindist', `intr' and `normal' parameters are all set according
**  to the intersection point if the following are true:
**
**      1) The initial `mindist' value is -1, or
**      2) The intersection point is a positive (albeit small) epsilon
**         distance along the ray and is closer than the initial `mindist'
**         value.
**
**  If these conditions are not met, then even if the ray intersects the
**  object, the intersection function returns false.
**
**  Note that these routines still behave properly if either the `intr' or
**  `normal' parameters are nil.
**
****************************************************************************/




/****************************************************************************
**  This is the intersection function for hyperspheres.
****************************************************************************/

boolean  HitSphere  (objptr, rayO, rayD, mindist, intr, normal)
   ObjInfo *objptr;	/* Sphere to Test */
   Point4   rayO;	/* Ray Origin */
   Vector4  rayD;	/* Ray Direction */
   Real    *mindist;	/* Previous Minimum Distance */
   Point4   intr;	/* Intersection Point */
   Vector4  normal;	/* Surface Normal @ Intersection Point */
{
#  define SPHERE  ((Sphere*)(objptr))   /* Target Sphere */

   auto  Real     bb;		/* Quadratic Equation Parameter */
   auto  Vector4  cdir;		/* Direction from Sphere Center to Eye */
   auto  Real     rad;		/* Radical Value */
   auto  Real     t1,t2;	/* Intersection Ray Parameters */

#  if (DB_SPHERE > 1)
      printf ("HitSphere:  objptr=%08lx, mindist@%08lx=%lg\n", objptr,
      	 mindist, *mindist);
      printf ("HitSphere:  rayO=<%lg %lg %lg %lg>\n", V4_List(rayO));
      printf ("HitSphere:  rayD=<%lg %lg %lg %lg>\n", V4_List(rayD));
#  endif

   V4_3Vec (cdir, =, SPHERE->center, -, rayO);

   bb  = V4_Dot(cdir, rayD);
   rad = (bb * bb) - V4_Dot(cdir,cdir) + SPHERE->rsqrd;

#  if (DB_SPHERE > 1)
      printf ("HitSphere:  Radical is %lg\n", rad);
#  endif

   if (rad < 0.0)
      return false;

   rad = sqrt(rad);
   t2 = bb - rad;
   t1 = bb + rad;

#  if (DB_SPHERE > 1)
      printf ("HitSphere:  t1 is %lg, t2 is %lg\n", t1, t2);
#  endif

   if ((t1 < 0.0) || ((t2 > 0.0) && (t2 < t1)))
      t1 = t2;

   if (t1 <= 0.0)
      return false;

   if (!mindist)
      return true;

   /* Note that the t1 variable is now the length of the vector from the
   ** ray origin to the intersection point, since the direction vector is
   ** a unit vector.  */

   if ((*mindist > 0) && ((t1 < MINDIST) || (t1 > *mindist)))
      return false;      /* Not closer than previous intersection. */

   *mindist = t1;

   if (intr)
      V4_3Vec (intr, =, rayO, +, t1*rayD);

   if (normal)
   {  normal[0] = (intr[0] - SPHERE->center[0]) / SPHERE->radius;
      normal[1] = (intr[1] - SPHERE->center[1]) / SPHERE->radius;
      normal[2] = (intr[2] - SPHERE->center[2]) / SPHERE->radius;
      normal[3] = (intr[3] - SPHERE->center[3]) / SPHERE->radius;
   }

   return true;
}



/****************************************************************************
**  This is the intersection function for 4D tetrahedrons and
**  parallelepipeds.  Note that if the object is a tetrahedron and the
**  conditions are met to set the intersection values, then the barycentric
**  coordinates of the tetrahedron will also be set.  These values may be
**  used later for Phong or Gouraud shading.
****************************************************************************/

boolean  HitTetPar  (objptr, rayO, rayD, mindist, intersect, normal)
   ObjInfo *objptr;	/* Sphere to Test */
   Point4   rayO;	/* Ray Origin */
   Vector4  rayD;	/* Ray Direction */
   Real    *mindist;	/* Previous Minimum Distance */
   Point4   intersect;	/* Intersection Point */
   Vector4  normal;	/* Surface Normal @ Intersection Point */
{
   auto Real    Bc1,Bc2,Bc3;	/* Intersection Barycentric Coordinates */
   auto Point4  intr;		/* Intersection Point */
   auto Real    rayT;		/* Ray Equation Parameter */
   auto TetPar *tp;		/* Tetrahdron/Parallelepiped Data */

   if (objptr->type == O_TETRAHEDRON)
      tp = &(((Tetrahedron*)(objptr))->tp);
   else
      tp = &(((Parallelepiped*)(objptr))->tp);

#  if (DB_TETPAR >= 2)
      print  ("\n");
      printf ("HitTetPar:  rayD = <%lg %lg %lg %lg>\n", V4_List(rayD));
#  endif

   /* Find the ray parameter to intersect the hyperplane. */

   rayT = V4_Dot (tp->normal, rayD);

   if (fabs(rayT) < EPSILON)  /* If the ray is parallel to the hyperplane. */
      return false;

   rayT = (- tp->planeConst - V4_Dot(tp->normal,rayO)) / rayT;

#  if (DB_TETPAR >= 1)
   {
      V4_3Vec (intr, =, rayO, +, rayT * rayD);

#     if (DB_TETPAR >= 2)
         printf ("HitTetPar:  rayT = %6.4lf\n", rayT);
	 if (rayT > 0.0)
            printf ("HitTetPar:  intr = <%6.4lf %6.4lf %6.4lf %6.4lf>\n",
               V4_List(intr));
#     endif

      {  auto  Real    NdotI;	/* Normal Dot Intersection */
         NdotI = V4_Dot (tp->normal, intr);
         if (1e-10 < fabs (NdotI + tp->planeConst))
         {  printf ("HitTetPar:  !! planeConst %10.8lf, N dot I %10.8lf.\n",
               tp->planeConst, NdotI);
            printf ("HitTetPar:  !! Difference is %lg\n",
	       NdotI + tp->planeConst);
            Halt ("Aborting.\n", nil);
         }
      }
   }
#  endif

   if (rayT < 0.0)	/* If the object is behind the ray. */
      return false;

   /* If we're testing for the nearest intersection, then we can trivially
   ** reject those candidate intersection points that occur behind some
   ** other object in the scene.  */

   if (mindist && (*mindist > 0) && ((rayT < MINDIST) || (rayT > *mindist)))
      return false;

   V4_3Vec (intr, =, rayO, +, rayT * rayD);

   /* Now we need to find the barycentric coordinates of the 4D object to
   ** determine if the ray/hyperplane intersection point is inside of the
   ** 4D object.  To simplify the process, project the object to a 3-plane.
   ** In order to assure that we don't `squish' the object in the projection,
   ** select the three axes that are not dominant in the normal vector (the
   ** ax1, ax2 and ax3 fields). */

   /* ----------------------------------------------------------------------
   ** Use Cramer's rule to solve for the barycentric coordinates
   ** ((1-Bc1-Bc2-Bc3), Bc1, Bc2, Bc3) using the intersection point (I) of
   ** the object.  The equation used is as follows:
   **
   **     M0 = (Bc1 * M1) + (Bc2 * M2) + (Bc3 * M3)
   **
   **             +-            -+             +-             -+
   **             |I[ax1]-V0[ax1]|             |V1[ax1]-V0[ax1]|
   ** where M0 is |I[ax2]-V0[ax2]|       M1 is |V1[ax2]-V0[ax2]|
   **             |I[ax3]-V0[ax3]|             |V1[ax3]-V0[ax3]|
   **             +-            -+             +-             -+
   **
   **             +-             -+            +-             -+
   **             |V2[ax1]-V0[ax1]|            |V3[ax1]-V0[ax1]|
   **       M2 is |V2[ax2]-V0[ax2]|  and M3 is |V3[ax2]-V0[ax2]|
   **             |V2[ax3]-V0[ax3]|            |V3[ax3]-V0[ax3]|
   **             +-             -+            +-             -+
   ** --------------------------------------------------------------------*/

   {
      static Real    M01,M02,M03, M11,M12,M13,	/* Matrix Values */
		     M21,M22,M23, M31,M32,M33;

						/* Intermediate Values */
      static Real    M22M33_M23M32, M02M33_M03M32, M12M03_M13M02,
		     M12M33_M13M32, M12M23_M13M22, M02M23_M03M22;

      M01 = intr[tp->ax1] - tp->vert[0][tp->ax1];
      M02 = intr[tp->ax2] - tp->vert[0][tp->ax2];
      M03 = intr[tp->ax3] - tp->vert[0][tp->ax3];

      M11 = tp->vec1[tp->ax1];
      M12 = tp->vec1[tp->ax2];
      M13 = tp->vec1[tp->ax3];

      M21 = tp->vec2[tp->ax1];
      M22 = tp->vec2[tp->ax2];
      M23 = tp->vec2[tp->ax3];

      M31 = tp->vec3[tp->ax1];
      M32 = tp->vec3[tp->ax2];
      M33 = tp->vec3[tp->ax3];

      M22M33_M23M32 = (M22 * M33) - (M23 * M32);
      M02M33_M03M32 = (M02 * M33) - (M03 * M32);
      M12M03_M13M02 = (M12 * M03) - (M13 * M02);
      M12M33_M13M32 = (M12 * M33) - (M13 * M32);
      M12M23_M13M22 = (M12 * M23) - (M13 * M22);
      M02M23_M03M22 = (M02 * M23) - (M03 * M22);

      Bc1 = (  (M01*M22M33_M23M32)
             - (M21*M02M33_M03M32)
             + (M31*M02M23_M03M22)) / tp->CramerDiv;

      if ((Bc1 < 0.0) || (Bc1 > 1.0))
      {
#         if (DB_TETPAR <= 0)
            return false;
#         endif
      }

      Bc2 = (  (M11*M02M33_M03M32)
             - (M01*M12M33_M13M32)
             + (M31*M12M03_M13M02)) / tp->CramerDiv;

      if ((Bc2 < 0.0) || (Bc2 > 1.0))
      {
#         if (DB_TETPAR <= 0)
            return false;
#         endif
      } 

      Bc3 = (- (M11*M02M23_M03M22)
             - (M21*M12M03_M13M02)
             + (M01*M12M23_M13M22)) / tp->CramerDiv;

      if ((Bc3 < 0.0) || (Bc3 > 1.0))
      {
#         if (DB_TETPAR <= 0)
            return false;
#         endif
      }
   }

   /* Test the barycentric coordinates to determine if the intersection
   ** point is within the object.  */

#  if (DB_TETPAR >= 1)
   {
      auto Vector4  Bint;	/* Intersect Point from Barycentric Coords */
      auto Vector4  Diff;	/* Difference Vector, Bint and Intersect */

#     if (DB_TETPAR >= 2)
         printf ("HitTetpar:  Bc1,2,3 = %lg  %lg  %lg\n", Bc1, Bc2, Bc3);
#     endif

      V4_3Vec (Bint,  =, tp->vert[0],    +, Bc1 * tp->vec1);
      V4_3Vec (Bint, +=, Bc2 * tp->vec2, +, Bc3 * tp->vec3);

      V4_3Vec (Diff, =, intr, -, Bint);

      if (V4_Norm(Diff) > 1e-12)
      {  print ("HitTetpar:  !! Intersect != Barycentric intersect.\n");
         printf("  Bc1,2,3 are %lg  %lg  %lg\n", Bc1, Bc2, Bc3);
         printf("  Intersect <%6.4lf %6.4lf %6.4lf %6.4lf>\n",V4_List(intr));
         printf("  Bintersect <%6.4lf %6.4lf %6.4lf %6.4lf>\n",
	    V4_List(Bint));
         printf("  Norm(Diff) is %lg\n", V4_Norm(Diff));
         Halt  ("Aborting.", nil);
      }

      if (  ((Bc1 < 0.0) || (Bc1 > 1.0))
         || ((Bc2 < 0.0) || (Bc2 > 1.0))
         || ((Bc3 < 0.0) || (Bc3 > 1.0))
         )
      {
         return false;
      }
   }
#  endif

   if ((objptr->type == O_TETRAHEDRON) && ((Bc1 + Bc2 + Bc3) > 1.0))
      return false;

   /* At this point we know that the ray intersects the 4D object.  If we're
   ** only testing for shadowing, return true immediately and leave the
   ** specific intersection parameters unaltered.  */

   if (!mindist)
      return true;

   /* We've already tested to see if the intersection point occurs behind
   ** some other object in the scene, so now we just load up the intersection
   ** parameters and return true.  */

   *mindist = rayT;

   if (intersect)
      V4_2Vec (intersect, =, intr);

   if (normal)
      V4_2Vec (normal, =, tp->normal);

   if (objptr->type == O_TETRAHEDRON)
   {  ((Tetrahedron*)(objptr))->Bc1 = Bc1;
      ((Tetrahedron*)(objptr))->Bc2 = Bc2;
      ((Tetrahedron*)(objptr))->Bc3 = Bc3;
   }

   return true;
}



/****************************************************************************
**  This is the intersection routine for 2D triangles in 4-space.  If the ray
**  intersects triangle and the conditions are met to set the intersection
**  specifics, then the barycentric coordinates of the triangle will also be
**  set according to the intersection point.  These values will be used by
**  the shading routines for Phong or Gouraud shading.
****************************************************************************/

boolean  HitTriangle  (objptr, rayO, rayD, mindist, intersect, normal)
   ObjInfo *objptr;	/* Sphere to Test */
   Point4   rayO;	/* Ray Origin */
   Vector4  rayD;	/* Ray Direction */
   Real    *mindist;	/* Previous Minimum Distance */
   Point4   intersect;	/* Intersection Point */
   Vector4  normal;	/* Surface Normal @ Intersection Point */
{
#  define TRI  ((Triangle*)(objptr))

   auto int      ax1, ax2;		/* Dominant Axes, Tri. Projection */
   auto Real     div;			/* Intersection Equation Divisor */
   auto Point4   intr;			/* Ray/Plane Intersection Point */
   auto Vector4  _normal;		/* Internal Normal Vector */
   auto Real     rayT;			/* Ray Equation Real Parameter */
   auto Vector4  vecTemp1, vecTemp2;	/* Temporary Vectors */

   /*------------------------------------------------------------------------
   ** The following segment calculates the intersection point (if one exists)
   ** with the ray and the plane containing the polygon.  The equation for
   ** this is as follows:
   **
   **            +-                       -+
   **            | (V0 - rayO) x vec1,vec2 | . (rayD x vec1,vec2)
   **            +-                       -+
   **     rayT = --------------------------------------------------
   **                                              2
   **                         | rayD x vec1, vec2 |
   **
   ** V0 a vertex of the triangle, vec1 is the vector from V0 to another
   ** vertex, and vec2 is the vector from V0 to the other vertex.
   ------------------------------------------------------------------------*/

   V4_Cross (vecTemp2, rayD, TRI->vec1, TRI->vec2);
   div = V4_Dot (vecTemp2, vecTemp2);
   if (div < EPSILON)
      return false;

   V4_3Vec (vecTemp1, =, TRI->vert[0], -, rayO);
   V4_Cross (vecTemp1, vecTemp1, TRI->vec1, TRI->vec2);

   rayT = V4_Dot (vecTemp1, vecTemp2) / div;

   /* If the intersection point is behind the ray, then no intersection. */

#  if (DB_TRI >= 1)
   {
      auto Vector4  CChk;	/* Vector Cross Product Check */
      auto Vector4  V3;		/* Vec from Vert0 to Intersect */

      V4_3Vec (intr, =, rayO, +, rayT * rayD);
      V4_3Vec (V3, =, intr, -, TRI->vert[0]);
      V4_Cross(CChk, TRI->vec1, TRI->vec2, V3);

      if (EPSILON < V4_Norm(CChk))
      {  print  ("ERROR:  Intersect point not in triangle plane.\n");
         printf ("V0 <%6.4lf %6.4lf %6.4lf %6.4lf>\n",V4_List(TRI->vert[0]));
         printf ("V1 <%6.4lf %6.4lf %6.4lf %6.4lf>\n",V4_List(TRI->vert[1]));
         printf ("V2 <%6.4lf %6.4lf %6.4lf %6.4lf>\n",V4_List(TRI->vert[2]));
         print  ("\n");
         printf ("     RayD = <%lg %lg %lg %lg>\n", V4_List (rayD));
         printf ("(Q-P)xE,F = <%lg %lg %lg %lg>\n", V4_List (vecTemp1));
         printf ("    DxE,F = <%lg %lg %lg %lg>\n", V4_List (vecTemp2));
         printf ("     RayT =  %lg\n", rayT);
         print  ("\n");
         printf ("Intr <%lg %lg %lg %lg>\n", V4_List (intr));
         printf ("Ivec <%lg %lg %lg %lg>\n", V4_List (V3));
         printf ("Cross<%lg %lg %lg %lg>\n", V4_List (CChk));
         printf ("Norm (CChk) is %lg\n", V4_Norm(CChk));
         Halt   ("Aborting.", nil);
      }
   }
#  endif

   if (rayT < 0.0)
      return false;

   /* If we've previously hit something and the current intersection distance
   ** is either less than epsilon or greater then the previous distance, then
   ** return false; we don't have a nearer intersection. */

   if (mindist && (*mindist > 0) && ((rayT < MINDIST) || (rayT > *mindist)))
      return false;

   V4_3Vec (intr, =, rayO, +, rayT * rayD);


   /* Compute the triangle normal vector.  Since the triangle is embedded in
   ** 2-space, we've got an extra degree of freedom floating around, so we
   ** need to do some jazz to pin it down.  To do this I confine the normal
   ** vector to the 3-plane that contains the triangle and the ray.  The
   ** analogous situation is trying to compute the normal vector to a line
   ** in 3-space -- you'd want the normal to be perpendicular to the line
   ** and in the plane defined by the line and the ray.  It turns out that
   ** this is fairly simple to do (although 4D cross products are quite
   ** expensive computationally).  */
   

   {  auto Vector4  Vtemp;	/* Temporary Vector */

      V4_Cross (Vtemp,    rayD,TRI->vec1,TRI->vec2);
      V4_Cross (_normal, Vtemp,TRI->vec1,TRI->vec2);
   }

#  if (DB_TRI >= 1)
   {
      auto Real  NdotV0;	/* Normal dot Vertex0 */
      auto Real  NdotV1;	/* Normal dot Vertex1 */
      auto Real  NdotV2;	/* Normal dot Vertex2 */
      auto Real  NdotI;		/* Normal dot Intersect */

      NdotV0 = V4_Dot (_normal, TRI->vert[0]);
      NdotV1 = V4_Dot (_normal, TRI->vert[1]);
      NdotV2 = V4_Dot (_normal, TRI->vert[2]);
      NdotI  = V4_Dot (_normal, intr);

      if (  (EPSILON < fabs(NdotV0 - NdotV1))
         || (EPSILON < fabs(NdotV0 - NdotV2))
         || (EPSILON < fabs(NdotV0 - NdotI)))
      {
         print  ("ERROR:  V0, V1, V2 & Intersect not in same plane.\n");
         printf ("RayD   <%6.4lf %6.4lf %6.4lf %6.4lf>\n",V4_List(rayD));
         printf ("Normal <%6.4lf %6.4lf %6.4lf %6.4lf>\n",V4_List(_normal));
         printf ("V0 <%6.4lf %6.4lf %6.4lf %6.4lf>\n",V4_List(TRI->vert[0]));
         printf ("V1 <%6.4lf %6.4lf %6.4lf %6.4lf>\n",V4_List(TRI->vert[1]));
         printf ("V2 <%6.4lf %6.4lf %6.4lf %6.4lf>\n",V4_List(TRI->vert[2]));
         printf ("I  <%6.4lf %6.4lf %6.4lf %6.4lf>\n",V4_List(intr));
         printf ("NdotV0 %lg, NdotV1 %lg, NdotV2 %lg\n",
            NdotV0, NdotV1, NdotV2);
         printf ("NdotI  %lg\n", NdotI);
         Halt ("Aborting.", nil);
      }
   }
#  endif

   /* In order to find the barycentric coordinates of the intersection point
   ** in the triangle, we need to find the two minor axes of the normal
   ** vector.  The intersection point and triangle can then be projected to
   ** the plane spanned by the two minor axes without fear of "collapsing"
   ** the triangle in the process.  */

   if (fabs(_normal[0]) < fabs(_normal[1]))	   /* X, Y */
      ax1 = 0,    ax2 = 1;
   else
      ax1 = 1,  ax2 = 0;

   if (fabs(_normal[2]) < fabs(_normal[ax2]))      /* X, Y, Z */
   {  if (fabs(_normal[2]) < fabs(_normal[ax1]))
         ax2 = ax1,  ax1 = 2;
      else
         ax2 = 2;
   }

   if (fabs(_normal[3]) < fabs(_normal[ax2]))      /* X, Y, Z, W */
   {  if (fabs(_normal[3]) < fabs(_normal[ax1]))
         ax2 = ax1,  ax1 = 3;
      else
         ax2 = 3;
   }

   /* ----------------------------------------------------------------------
   ** Now compute the barycentric coordinates of the intersection point
   ** relative to the three vertices of the triangle.  The equation used
   ** here is as follows (I=intersection point, Vx=Triangle vertex):
   **
   ** +-              -+       +-               -+       +-               -+
   ** |I[ax1] - V0[ax1]|       |V1[ax1] - V0[ax1]|       |V2[ax1] - V0[ax1]|
   ** |I[ax2] - V0[ax2]| = Bc1 |V1[ax2] - V0[ax2]| + Bc2 |V2[ax2] - V0[ax2]|
   ** +-              -+       +-               -+       +-               -+
   **
   **                       Bc1 Bc2
   **            +-  -+   +-       -+
   **      or:   | I1 |   | V11 V21 |
   **            | I2 | = | V12 V22 |
   **            +-  -+   +-       -+
   **
   ** where Bc1 and Bc2 are the barycentric coordinates for vertex 1 and
   ** vertex 2.  The barycentric coordinates for vertex 0 is (1-Bc1-Bc2).
   ** --------------------------------------------------------------------*/


   {  auto Real  div;		/* Cramer's Rule Divisor */
      auto Real  I1, I2;	/* Matrix Entries */

      I1  = intr[ax1] - TRI->vert[0][ax1];
      I2  = intr[ax2] - TRI->vert[0][ax2];
      div = (TRI->vec1[ax1] * TRI->vec2[ax2])
          - (TRI->vec1[ax2] * TRI->vec2[ax1]);

      TRI->Bc1 = ((I1 * TRI->vec2[ax2]) - (I2 * TRI->vec2[ax1])) / div;

      if ((TRI->Bc1 < 0.0) || (TRI->Bc1 > 1.0))
      {
#        if (DB_TRI <= 0)
            return false;
#        endif
      }

      TRI->Bc2 = ((TRI->vec1[ax1] * I2) - (TRI->vec1[ax2] * I1)) / div;

      if ((TRI->Bc2 <0.0) || (TRI->Bc2 >1.0) || ((TRI->Bc1+TRI->Bc2) >1.0))
      {
#        if (DB_TRI <= 0)
            return false;
#        endif
      }
   }

#  if (DB_TRI >= 1)
   {
      auto Vector4  Bint;	/* Intersect Point from Barycentric Coords */
      auto Vector4  Diff;	/* Difference Vector, Bint and Intersect */
      auto Real     rtmp;	/* Real-Valued Temporary */

      rtmp = (1.0 - TRI->Bc1 - TRI->Bc2);
      V4_2Vec (Bint,  =, rtmp * TRI->vert[0]);
      V4_3Vec (Bint, +=, TRI->Bc1*TRI->vert[1], +, TRI->Bc2 * TRI->vert[2]);

      V4_3Vec (Diff, =, intr, -, Bint);

      if (V4_Norm(Diff) > EPSILON)
      {  print  ("ERROR:  Intersect != Barycentric intersect.\n");
         printf (" Intersect <%6.4lf %6.4lf %6.4lf %6.4lf>\n",
            V4_List (intr));
         printf ("Bintersect <%6.4lf %6.4lf %6.4lf %6.4lf>\n",
            V4_List (Bint));
         printf ("Norm (Diff) is %lg\n", V4_Norm (Diff));
         Halt   ("Aborting.", nil);
      }
   }
#  endif

   /* At this point we know that the ray intersects the 2D triangle.  If
   ** we're only testing for shadowing, return true immediately and leave
   ** the specific intersection parameters unaltered.  */

   if (!mindist)
      return true;

   /* We've already tested to see if the intersection point occurs behind
   ** some other object in the scene, so now we just load up the intersection
   ** parameters and return true.  */

   *mindist = rayT;

   V4_2Vec (intersect, =, intr);
   V4_2Vec (normal,    =, _normal);

   return true;
}
