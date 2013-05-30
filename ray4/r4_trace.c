
/***********************************************************************
//
//  "ray4" is Copyright (c) 1991 by Steve R. Hollasch.
//
//  All rights reserved.  This software may be freely copied, modified
//  and redistributed, provided that this copyright notice is preserved
//  in all copies.  This software is provided "as is", without express
//  or implied warranty.  You may not include this software in a program
//  or other software product without also supplying the source, or
//  without informing the end-user that the source is available for no
//  extra charge.  If you modify this software, please include a notice
//  detailing the author, date and purpose of the modification.
//
***********************************************************************/

/****************************************************************************
//
//  File:  r4_trace.c
//
//      This file contains the procedures that spawn reflection and
//    refraction rays.  It also contains the procedures responsible for
//    shading and illumination.
//
//  Revisions:
//
//    1.00  25-Jan-92  Hollasch
//          Released to the public domain.
//
//    0.33  12-May-91  Hollasch
//          Fixed bug in shadow ray intersection routine call; `mindist' was
//          incorrectly passed by value, rather than by reference.
//
//    0.32  04-Mar-91  Hollasch
//          Fixed error in light-source shadow code; prior code allowed
//          objects behind point light sources to cast shadows.
//
//    0.30  05-Jan-91  Hollasch
//          Implemented new attribute list references.
//
//    0.21  20-Dec-90  Hollasch
//          Fixed illumination by flipping normal vector if sight vector
//            approaches an object from `behind'.
//          Implemented refracted rays.
//          Added code to filter light through transparent objects for
//            illumination calculations (ignoring refraction).
//
//    0.13  13-Dec-90  Hollasch
//          Implemented reflection rays.
//
//    0.10  15-Nov-90  Hollasch
//          Changed code to reflect new mindist parameter in intersection
//          routines.
//
//    0.00  30-Sep-90  Steve R. Hollasch
//          Initial version.
//
****************************************************************************/

#include <stdio.h>

#define  DB_TRACE  0	/* Debug Raytrace Loop */

#include "ray4.h"
#include "r4_globals.h"


Color black = { 0.000, 0.000, 0.000 };	/* Used to Zero Out Colors */


/****************************************************************************
**  This routine is the heart of the raytracer; it takes the ray, determines
**  which objects are hit, picks the closest one, determines the appropriate
**  shade at the surface, and then may or may not fire a reflection ray and
**  or a refraction ray.
****************************************************************************/

void  RayTrace  (rayO, rayD, color, level)
   Point4   rayO;	/* Ray Origin */
   Vector4  rayD;	/* Ray Direction */
   Color   *color;	/* Resulting Color */
   ulong    level;	/* Raytrace Level */
{
   static   Real        ftemp;	    /* Scratch Real Value */
   auto     Point4      intr_out;   /* Intersection Outside The Surface */
   register Light      *lptr;	    /* Light Pointer */
   auto     Attributes *nearattr;   /* Nearest Object's Attributes */
   auto     ObjInfo    *nearobj;    /* Nearest Object */
   auto     Point4      nearintr;   /* Nearest Object Intersection */
   auto     Vector4     nearnormal; /* Nearest Object Normal */
   auto     Real        NdotD;      /* Normal dot rayD */
   register ObjInfo    *optr;	    /* Object Pointer */

   ++ stats.Ncast;
   ++ level;

#  if (DB_TRACE)
      printf ("\nRayTrace:  ray %ld, level %lu\n", stats.Ncast, level);
#  endif

   if (level > stats.maxlevel)
      stats.maxlevel = level;

   /* Move the ray origin a bit along the ray direction to eliminate
   // surface acne, where floating-point roundoff erroneously puts the
   // point inside a surface.  */

   V4_3Vec (rayO, =, rayO, +, 1e-10 * rayD);

   /* Find the nearest object intersection. */

   {  auto Real  mindist;	/* Nearest Object Distance */

      mindist = -1.0;
      nearobj = nil;

      for (optr = objlist;  optr;  optr = optr->next)
      {
#        if (DB_TRACE)
	    print ("RayTrace:  Testing for ray-object intersection.\n");
#        endif

	 if ((*optr->intersect)
	       (optr, rayO,rayD, &mindist, nearintr,nearnormal))
	    nearobj = optr;

#        if (DB_TRACE)
	    printf ("RayTrace:  Ray did %shit object.\n",
	       nearobj == optr ? "" : "not ");
#        endif
      }
   }

   /* If the ray hit nothing, assign the background color to it.  If the
   // hit an object, then determine the shade at the intersection. */

   if (!nearobj)
   {
#     if (DB_TRACE)
         print ("RayTrace:  No objects intersected.\n");
#     endif

      *color = background;
      return;
   }

   nearattr = nearobj->attr;

   if (nearattr->flags & AT_AMBIENT)
      Color_3Op ((*color), =, ambient, *, nearattr->Ka);
   else
      *color = black;

   /* If the object has no diffuse or specular reflection, skip this
   ** part of the code.  */

   if (!(nearattr->flags & (AT_DIFFUSE | AT_SPECULAR)))
      goto REFTRAN;

   /* If we're looking at the object from `behind' (or inside), then
   // flip the normal vector. */

   NdotD = V4_Dot (nearnormal, rayD);

   if (NdotD > 0.0)
   {  V4_2Vec (nearnormal, =, -nearnormal);
      NdotD = -NdotD;
   }

   /* To avoid surface acne, move the intersection point just outside
   // the object surface to prevent that same surface from erroneously
   // shadowing itself.  */

   V4_3Vec (intr_out, =, nearintr, +, 1e-10 * nearnormal);

   /* Add illumation to the point from all visible lights. */

#  if (DB_TRACE)
      print ("RayTrace:  Calculating surface illumination.\n");
#  endif

   for (lptr=lightlist;  lptr;  lptr=lptr->next)
   {
      auto     Color    lcolor;		/* Light Color */
      auto     Vector4  ldir;		/* Light Direction */
      auto     Real     mindist;	/* Nearest Object Distance */
      register ObjInfo *optr;		/* Object List Traversal Pointer */
      auto     Vector4  Refl;		/* Reflection Vector */

      if (lptr->type == L_DIRECTIONAL)
      {  V4_2Vec (ldir, =, lptr->u.dir);
	 mindist = -1.0;
      }
      else
      {  auto Real  norm;	/* Vector Norm */

	 V4_3Vec (ldir, =, lptr->u.pos, -, intr_out);

	 /* Normalize the light-direction vector.  If the (point) light
	 // source is VERY close to the intersection point, then just set
	 // the light-direction vector to the surface normal. */

	 mindist = norm = V4_Norm (ldir);
	 if (norm < EPSILON)
	    V4_2Vec (ldir, =, nearnormal);
	 else
	    V4_Scalar (ldir, /=, norm);
      }

      /* Determine if the light is obscurred by any other object.  If the
      // light is obscurred by a transparent object, then ignore
      // refraction, but color the light we're receiving based on the
      // object's transparent color. */

      lcolor = lptr->color;

      for (optr=objlist;  optr;  optr=optr->next)
      {
	 auto Real minsave=mindist;	/* Nearest Object Distance (saved) */

#	 if (DB_TRACE)
	    print ("RayTrace:  Testing shadow ray intersection.\n");
#	 endif

	 if ((*optr->intersect)(optr, intr_out, ldir, &mindist, nil, nil))
	 {
	    if (!(optr->attr->flags & AT_TRANSPAR))
	       break;

	    Color_2Op (lcolor, *=, optr->attr->Kt);
	    mindist = minsave;
	 }

#	 if (DB_TRACE)
	    print ("RayTrace:  Finished shadow ray intersection test.\n");
#	 endif
      }

      /* If an opaque object shadows us, then skip this light source.
      // Also, if the maximum amount of light transmitted through
      // transparent objects is less than 1/256, then this light source
      // can add nothing significant, so skip it.  */

      if ((optr) || ((lcolor.r + lcolor.g + lcolor.b) < 0.001))
	 continue;

      /* If surface normal is turned from light, skip this light. */

      ftemp = V4_Dot (nearnormal, ldir);
      if (ftemp <= 0.0)  continue;

      /* Add the diffuse component of the light. */

      Color_3Op ((*color), +=, ftemp * nearattr->Kd, *, lcolor);

      /* If this object has no specular reflection, do next light. */

      if (!(nearattr->flags & AT_SPECULAR))  continue;

      /* Calculate the light reflection vector. */

      ftemp *= 2.0;
      V4_3Vec (Refl, =, -ldir, +, ftemp * nearnormal);

      /* Calculate the cosine of the sight & reflection vectors, raised
      ** to the Phong power, for the specular reflection. */

      ftemp = V4_Dot (-rayD, Refl);
      if (ftemp > 0.0)
      {  ftemp = pow (ftemp, nearattr->shine);
	 Color_3Op ((*color), +=, ftemp * nearattr->Ks, *, lcolor);
      }
   }


   REFTRAN:

#  if (DB_TRACE)
      print ("RayTrace:  Finished calculating surface illumination.\n");
#  endif

   /* If we're at the mamximum raytrace depth now, don't go any deeper. */

   if (maxdepth && (level == maxdepth))
      return;

   /* Find the contribution from the refraction vector, if applicable. */

   if (nearattr->flags & AT_TRANSPAR)
   {
      auto   Vector4  RefrD;	/* Refracted Direction Vector */
      static Vector4  T;	/* Temporary Vector */
      auto   Color    Tcolor;	/* Transparent Color */

#     if (DB_TRACE)
         print ("RayTrace:  Firing refraction ray.\n");
#     endif

      V4_2Vec (T, =, NdotD * nearnormal);
      ftemp = global_indexref / nearattr->indexref;

      RefrD[0] = T[0] + (ftemp * (rayD[0] - T[0]));
      RefrD[1] = T[1] + (ftemp * (rayD[1] - T[1]));
      RefrD[2] = T[2] + (ftemp * (rayD[2] - T[2]));
      RefrD[3] = T[3] + (ftemp * (rayD[3] - T[3]));

      RayTrace (nearintr, RefrD, &Tcolor, level);

      color->r += Tcolor.r * nearattr->Kt.r;
      color->g += Tcolor.g * nearattr->Kt.g;
      color->b += Tcolor.b * nearattr->Kt.b;
   }


   /* Find the contribution from the reflection vector, if applicable. */

   if (nearattr->flags & AT_REFLECT)
   {
      auto Color    Rcolor;	/* Reflected Color */
      auto Vector4  ReflD;	/* Reflection Direction Vector */

#     if (DB_TRACE)
         print ("RayTrace:  Firing reflection ray.\n");
#     endif

      ftemp = 2.0 * NdotD;
      V4_3Vec (ReflD, =, rayD, -, ftemp * nearnormal);

      RayTrace (nearintr, ReflD, &Rcolor, level);

      color->r += Rcolor.r * nearattr->Ks.r;
      color->g += Rcolor.g * nearattr->Ks.g;
      color->b += Rcolor.b * nearattr->Ks.b;
   }
}
