/*******************************************************************************
"ray4" is Copyright (c) 1991,1992,1993,1996 by Steve R. Hollasch.

All rights reserved.  This software may be freely copied, modified and
redistributed, provided that this copyright notice is preserved in all copies.
This software is provided "as is", without express or implied warranty.  You
may not include this software in a program or other software product without
also supplying the source, except by express agreement with the author (Steve
Hollasch).  If you modify this software, please include a notice detailing the
author, date and purpose of the modification.
*******************************************************************************/

/*******************************************************************************

File:  r4_trace.c

    This file contains the procedures that spawn reflection and refraction
rays.  It also contains the procedures responsible for shading and
illumination.

*******************************************************************************/

#include "ray4.h"
#include "r4_globals.h"


Color black = { 0.000, 0.000, 0.000 };  /* Used to Zero Out Colors */


/*****************************************************************************
This routine is the heart of the raytracer; it takes the ray, determines which
objects are hit, picks the closest one, determines the appropriate shade at the
surface, and then may or may not fire a reflection ray and or a refraction ray.
*****************************************************************************/

void  RayTrace  (
    Point4   rayO,       /* Ray Origin */
    Vector4  rayD,       /* Ray Direction */
    Color   *color,      /* Resulting Color */
    ulong    level)      /* Raytrace Level */
{
    Real        ftemp;      /* Scratch Real Value */
    Point4      intr_out;   /* Intersection Outside The Surface */
    Light      *lptr;       /* Light Pointer */
    Attributes *nearattr;   /* Nearest Object's Attributes */
    ObjInfo    *nearobj;    /* Nearest Object */
    Point4      nearintr;   /* Nearest Object Intersection */
    Vector4     nearnormal; /* Nearest Object Normal */
    Real        NdotD;      /* Normal dot rayD */
    ObjInfo    *optr;       /* Object Pointer */

    ++ stats.Ncast;
    ++ level;

    if (level > stats.maxlevel)
        stats.maxlevel = level;

    /* Move the ray origin a bit along the ray direction to eliminate
    ** surface acne, where floating-point roundoff erroneously puts the
    ** point inside a surface.  */

    V4_3Vec (rayO, =, rayO, +, 1e-10 * rayD);

    /* Find the nearest object intersection. */

    {   Real  mindist;       /* Nearest Object Distance */

        mindist = -1.0;
        nearobj = nil;

        for (optr = objlist;  optr;  optr = optr->next)
        {
            if ((*optr->intersect)
                (optr, rayO,rayD, &mindist, nearintr,nearnormal))
            nearobj = optr;
        }
    }

    /* If the ray hit nothing, assign the background color to it.  If the
    ** hit an object, then determine the shade at the intersection. */

    if (!nearobj)
    {   *color = background;
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
    ** flip the normal vector. */

    NdotD = V4_Dot (nearnormal, rayD);

    if (NdotD > 0.0)
    {   V4_2Vec (nearnormal, =, -nearnormal);
        NdotD = -NdotD;
    }

    /* To avoid surface acne, move the intersection point just outside
    ** the object surface to prevent that same surface from erroneously
    ** shadowing itself.  */

    V4_3Vec (intr_out, =, nearintr, +, 1e-10 * nearnormal);

    /* Add illumation to the point from all visible lights. */

    for (lptr=lightlist;  lptr;  lptr=lptr->next)
    {
        Color    lcolor;         /* Light Color */
        Vector4  ldir;           /* Light Direction */
        Real     mindist;        /* Nearest Object Distance */
        ObjInfo *optr;           /* Object List Traversal Pointer */
        Vector4  Refl;           /* Reflection Vector */

        if (lptr->type == L_DIRECTIONAL)
        {   V4_2Vec (ldir, =, lptr->u.dir);
            mindist = -1.0;
        }
        else
        {   Real norm;       /* Vector Norm */

            V4_3Vec (ldir, =, lptr->u.pos, -, intr_out);

            /* Normalize the light-direction vector.  If the (point) light
            ** source is VERY close to the intersection point, then just set
            ** the light-direction vector to the surface normal. */

            mindist = norm = V4_Norm (ldir);
            if (norm < EPSILON)
                V4_2Vec (ldir, =, nearnormal);
            else
                V4_Scalar (ldir, /=, norm);
        }

        /* Determine if the light is obscurred by any other object.  If the
        ** light is obscurred by a transparent object, then ignore
        ** refraction, but color the light we're receiving based on the
        ** object's transparent color. */

        lcolor = lptr->color;

        for (optr=objlist;  optr;  optr=optr->next)
        {
            Real minsave=mindist;     /* Nearest Object Distance (saved) */

            if ((*optr->intersect)(optr, intr_out, ldir, &mindist, nil, nil))
            {
                if (!(optr->attr->flags & AT_TRANSPAR))
                    break;

                Color_2Op (lcolor, *=, optr->attr->Kt);
                mindist = minsave;
            }
        }

        /* If an opaque object shadows us, then skip this light source.
        ** Also, if the maximum amount of light transmitted through
        ** transparent objects is less than 1/256, then this light source
        ** can add nothing significant, so skip it.  */

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
        {   ftemp = pow (ftemp, nearattr->shine);
            Color_3Op ((*color), +=, ftemp * nearattr->Ks, *, lcolor);
        }
    }

    REFTRAN:

    /* If we're at the mamximum raytrace depth now, don't go any deeper. */

    if (maxdepth && (level == maxdepth))
        return;

    /* Find the contribution from the refraction vector, if applicable. */

    if (nearattr->flags & AT_TRANSPAR)
    {
        Vector4  RefrD;    /* Refracted Direction Vector */
        Vector4  T;        /* Temporary Vector */
        Color    Tcolor;   /* Transparent Color */

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
        Color    Rcolor;     /* Reflected Color */
        Vector4  ReflD;      /* Reflection Direction Vector */

        ftemp = 2.0 * NdotD;
        V4_3Vec (ReflD, =, rayD, -, ftemp * nearnormal);

        RayTrace (nearintr, ReflD, &Rcolor, level);

        color->r += Rcolor.r * nearattr->Ks.r;
        color->g += Rcolor.g * nearattr->Ks.g;
        color->b += Rcolor.b * nearattr->Ks.b;
    }
}
