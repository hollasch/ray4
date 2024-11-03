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
// r4_trace.cpp
//
// This file contains the procedures that spawn reflection and refraction rays. It also contains the
// procedures responsible for shading and illumination.
//==================================================================================================

#include "ray4.h"

Color black { 0, 0, 0 };  // Used to zero out colors.



//__________________________________________________________________________________________________

void RayTrace (
    const Ray4 &rayIn,  // Trace Ray
    Color      &color,  // Resulting Color
    int         level)  // Raytrace Level
{
    // This routine is the heart of the raytracer; it takes the ray, determines which objects are
    // hit, picks the closest one, determines the appropriate shade at the surface, and then may or
    // may not fire a reflection ray and or a refraction ray.

    ++ stats.Ncast;
    ++ level;

    if (level > stats.maxlevel)
        stats.maxlevel = level;

    // Move the ray origin a bit along the ray direction to eliminate surface acne, where
    // floating-point roundoff erroneously puts the point inside a surface.

    Ray4 ray = rayIn;
    ray.origin = ray(1e-10);

    // Find the nearest object intersection.

    ObjInfo *nearobj = nullptr;  // Nearest Object
    Point4   nearintr{0,0,0,0};  // Nearest Object Intersection
    Vector4  nearnormal;         // Nearest Object Normal
    ObjInfo *optr = nullptr;     // Object List Traversal Pointer

    {
        double mindist;  // Nearest Object Distance

        mindist = -1.0;

        for (optr = objlist;  optr;  optr = optr->next) {
            if ((*optr->intersect)(optr, ray, &mindist, &nearintr, &nearnormal))
                nearobj = optr;
        }
    }

    // If the ray hit nothing, assign the background color to it. If the hit an object, then
    // determine the shade at the intersection.

    if (!nearobj) {
        color = background;
        return;
    }

    auto nearattr = nearobj->attr;  // Nearest Object's Attributes

    if (nearattr->flags & AT_AMBIENT)
        color = ambient * nearattr->Ka;
    else
        color = black;

    // If the object has no diffuse or specular reflection, skip this part of the code.

    double NdotD = 0;  // Normal dot ray direction

    if (!(nearattr->flags & (AT_DIFFUSE | AT_SPECULAR)))
        goto REFTRAN;

    // If we're looking at the object from `behind' (or inside), then flip the normal vector.

    NdotD = dot(nearnormal, ray.direction);

    if (NdotD > 0.0) {
        nearnormal = -nearnormal;
        NdotD = -NdotD;
    }

    // To avoid surface acne, move the intersection point just outside the object surface to prevent
    // that same surface from erroneously shadowing itself.

    Point4 intr_out = nearintr + (1e-10 * nearnormal);  // Intersection Outside The Surface

    // Add illumation to the point from all visible lights.

    for (auto *light = lightlist;  light;  light=light->next) {
        Vector4 ldir;     // Light Direction
        double  mindist;  // Nearest Object Distance

        if (light->type == LightType::Directional) {
            ldir = light->direction;
            mindist = -1.0;
        } else {
            double norm;  // Vector Norm

            ldir = light->position - intr_out;

            // Normalize the light-direction vector. If the (point) light source is VERY close to
            // the intersection point, then just set the light-direction vector to the surface
            // normal.

            mindist = norm = ldir.norm();
            if (norm < epsilon)
                ldir = nearnormal;
            else
                ldir /= norm;
        }

        // Determine if the light is obscurred by any other object. If the light is obscurred by a
        // transparent object, then ignore refraction, but color the light we're receiving based on
        // the object's transparent color.

        auto lcolor = light->color;  // Light Color

        for (optr=objlist;  optr;  optr=optr->next) {
            double minsave=mindist;  // Nearest Object Distance (saved)

            if ((*optr->intersect)(optr, Ray4(intr_out, ldir), &mindist, nullptr, nullptr)) {
                if (!(optr->attr->flags & AT_TRANSPAR))
                    break;

                lcolor *= optr->attr->Kt;
                mindist = minsave;
            }
        }

        // If an opaque object shadows us, then skip this light source. Also, if the maximum amount
        // of light transmitted through transparent objects is less than 1/256, then this light
        // source can add nothing significant, so skip it.

        if ((optr) || ((lcolor.r + lcolor.g + lcolor.b) < 0.001))
            continue;

        // If surface normal is turned from light, skip this light.

        double ftemp = dot(nearnormal, ldir);  // Scratch Real Value
        if (ftemp <= 0.0)
            continue;

        // Add the diffuse component of the light.

        color += ftemp * nearattr->Kd * lcolor;

        // If this object has no specular reflection, do next light.

        if (!(nearattr->flags & AT_SPECULAR))  continue;

        // Calculate the light reflection vector.

        ftemp *= 2.0;
        Vector4 Refl = -ldir + (ftemp * nearnormal);  // Reflection Vector

        // Calculate the cosine of the sight & reflection vectors, raised to the Phong power, for
        // the specular reflection.

        ftemp = dot(-ray.direction, Refl);
        if (ftemp > 0.0) {
            ftemp = pow (ftemp, nearattr->shine);
            color += ftemp * nearattr->Ks * lcolor;
        }
    }

    REFTRAN:

    // If we're at the mamximum raytrace depth now, don't go any deeper.

    if (maxdepth && (level == maxdepth))
        return;

    // Find the contribution from the refraction vector, if applicable.

    if (nearattr->flags & AT_TRANSPAR) {
        Vector4 T = NdotD * nearnormal;
        double ftemp = global_indexref / nearattr->indexref;

        Vector4 RefrD = T + (ftemp * (ray.direction - T));  // Refracted Direction Vector

        Color Tcolor;  // Transparent Color
        RayTrace (Ray4(nearintr, RefrD), Tcolor, level);

        color += nearattr->Kt * Tcolor;
    }

    // Find the contribution from the reflection vector, if applicable.

    if (nearattr->flags & AT_REFLECT) {
        double ftemp = 2.0 * NdotD;
        Vector4 ReflD = ray.direction - (ftemp * nearnormal);

        Color Rcolor;  // Reflected Color
        RayTrace (Ray4(nearintr, ReflD), Rcolor, level);

        color += nearattr->Ks * Rcolor;
    }
}
