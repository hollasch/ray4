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
// r4_hit.cpp
//
// This file contains intersection routines for the geometric primitives in the Ray4 4D raytracer.
// See the r4_main.cpp header comment for more information on Ray4.
//==================================================================================================

#include <stdio.h>

#include "ray4.h"


#define MINDIST 1e-7  // Minimum Intersection Distance (for the elimination of surface acne)



    //==============================================================================================
    // All intersection functions must behave in the same manner, since they are called as
    // object-oriented functions that behave generically for different objects. Each function takes
    // the same parameters and returns a boolean value. Operation of each function is as follows:
    //
    // The functions take the following parameter list:
    //
    //     ObjInfo* objptr : Pointer to the Object Structure
    //     Ray4     ray    : Ray Origin
    //     double  *mindist: Closest Intersection Distance So Far
    //     Point4  *intr   : Intersection Point
    //     Vector4 *normal : Surface Normal at Intersection Point
    //
    // If the ray does not intersect the object, the intersection function returns false and does
    // not alter `mindist', `intr' or `normal'.
    //
    // If the mindist parameter is null, then it's assumed that the calling program is not
    // interested in the specifics of the intersection; it's only interested in whether the ray
    // intersects the object (as in shadow testing). In this case, if the ray does intersect the
    // object, the function immediately returns true, but does not alter the `mindist', `intr' or
    // `normal' parameters.
    //
    // If the ray intersects the object, the intersection function returns true and the `mindist',
    // `intr' and `normal' parameters are all set according to the intersection point if the
    // following are true:
    //
    //     1) The initial `mindist' value is -1, or
    //     2) The intersection point is a positive (albeit small) epsilon distance along the ray and
    //        is closer than the initial `mindist' value.
    //
    // If these conditions are not met, then even if the ray intersects the object, the intersection
    // function returns false.
    //
    // Note that these routines still behave properly if either the `intr' or `normal' parameters
    // are null.
    //==============================================================================================



//__________________________________________________________________________________________________

bool HitSphere (
    ObjInfo    *objptr,    // Sphere to Test
    const Ray4 &ray,       // Trace Ray
    double     *mindist,   // Previous Minimum Distance
    Point4     *intr,      // Intersection Point
    Vector4    *normal)    // Surface Normal @ Intersection Point
{
    // This is the intersection function for hyperspheres.

    const auto& sphere = *reinterpret_cast<Sphere*>(objptr);

    Vector4 cdir = sphere.center - ray.origin;   // Direction from Sphere Center to Eye

    double bb  = dot(cdir, ray.direction);                    // Quadratic Equation Parameter
    double rad = (bb * bb) - dot(cdir, cdir) + sphere.rsqrd;  // Radical Value

    if (rad < 0.0)
        return false;

    // Compute the ray intersection parameters
    rad = sqrt(rad);
    double t1 = bb + rad;
    double t2 = bb - rad;

    if ((t1 < 0.0) || ((t2 > 0.0) && (t2 < t1)))
        t1 = t2;

    if (t1 <= 0.0)
        return false;

    if (!mindist)
        return true;

    // Note that the t1 variable is now the length of the vector from the ray origin to the
    // intersection point, since the direction vector is a unit vector.

    if ((*mindist > 0) && ((t1 < MINDIST) || (t1 > *mindist)))
        return false;      // Not closer than previous intersection.

    *mindist = t1;

    if (intr)
        (*intr) = ray(t1);

    if (normal)
        *normal = (*intr - sphere.center) / sphere.radius;

    return true;
}

//__________________________________________________________________________________________________

bool HitTetPar (
    ObjInfo    *objptr,     // Sphere to Test
    const Ray4 &ray,       // Trace Ray
    double     *mindist,    // Previous Minimum Distance
    Point4     *intersect,  // Intersection Point
    Vector4    *normal)     // Surface Normal @ Intersection Point
{
    // This is the intersection function for 4D tetrahedrons and parallelepipeds. Note that if the
    // object is a tetrahedron and the conditions are met to set the intersection values, then the
    // barycentric coordinates of the tetrahedron will also be set. These values may be used later
    // for Phong or Gouraud shading.

    TetPar *tp;  // Tetrahdron/Parallelepiped Data

    if (objptr->type == ObjType::Tetrahedron)
        tp = &((reinterpret_cast<Tetrahedron*>(objptr))->tp);
    else
        tp = &((reinterpret_cast<Parallelepiped*>(objptr))->tp);

    // Find the ray parameter to intersect the hyperplane.

    double rayT = dot(tp->normal, ray.direction);  // Ray Equation Parameter

    if (fabs(rayT) < epsilon)  // If the ray is parallel to the hyperplane.
        return false;

    rayT = (-tp->planeConst - dot(tp->normal, ray.origin.toVector())) / rayT;

    if (rayT < 0.0)      // If the object is behind the ray.
        return false;

    // If we're testing for the nearest intersection, then we can trivially reject those candidate
    // intersection points that occur behind some other object in the scene.

    if (mindist && (*mindist > 0) && ((rayT < MINDIST) || (rayT > *mindist)))
        return false;

    Point4 intr = ray(rayT);  // Intersection Point

    // Now we need to find the barycentric coordinates of the 4D object to determine if the
    // ray/hyperplane intersection point is inside of the 4D object. To simplify the process,
    // project the object to a 3-plane. In order to assure that we don't `squish' the object in the
    // projection, select the three axes that are not dominant in the normal vector (the ax1, ax2
    // and ax3 fields).

    // Use Cramer's rule to solve for the barycentric coordinates ((1-Bc1-Bc2-Bc3), Bc1, Bc2, Bc3)
    // using the intersection point (I) of the object. The equation used is as follows:
    //
    //     M0 = (Bc1 * M1) + (Bc2 * M2) + (Bc3 * M3)
    //
    //             +-            -+             +-             -+
    //             |I[ax1]-V0[ax1]|             |V1[ax1]-V0[ax1]|
    // where M0 is |I[ax2]-V0[ax2]|       M1 is |V1[ax2]-V0[ax2]|
    //             |I[ax3]-V0[ax3]|             |V1[ax3]-V0[ax3]|
    //             +-            -+             +-             -+
    //
    //             +-             -+            +-             -+
    //             |V2[ax1]-V0[ax1]|            |V3[ax1]-V0[ax1]|
    //       M2 is |V2[ax2]-V0[ax2]|  and M3 is |V3[ax2]-V0[ax2]|
    //             |V2[ax3]-V0[ax3]|            |V3[ax3]-V0[ax3]|
    //             +-             -+            +-             -+

    double Bc1,Bc2,Bc3;  // Intersection Barycentric Coordinates
    {
        // Intermediate Values

        double M01 = intr[tp->ax1] - tp->vert[0][tp->ax1];
        double M02 = intr[tp->ax2] - tp->vert[0][tp->ax2];
        double M03 = intr[tp->ax3] - tp->vert[0][tp->ax3];

        double M11 = tp->vec1[tp->ax1];
        double M12 = tp->vec1[tp->ax2];
        double M13 = tp->vec1[tp->ax3];

        double M21 = tp->vec2[tp->ax1];
        double M22 = tp->vec2[tp->ax2];
        double M23 = tp->vec2[tp->ax3];

        double M31 = tp->vec3[tp->ax1];
        double M32 = tp->vec3[tp->ax2];
        double M33 = tp->vec3[tp->ax3];

        double M22M33_M23M32 = (M22 * M33) - (M23 * M32);
        double M02M33_M03M32 = (M02 * M33) - (M03 * M32);
        double M12M03_M13M02 = (M12 * M03) - (M13 * M02);
        double M12M33_M13M32 = (M12 * M33) - (M13 * M32);
        double M12M23_M13M22 = (M12 * M23) - (M13 * M22);
        double M02M23_M03M22 = (M02 * M23) - (M03 * M22);

        Bc1 = ((M01*M22M33_M23M32) - (M21*M02M33_M03M32) + (M31*M02M23_M03M22)) / tp->CramerDiv;
        if ((Bc1 < 0.0) || (Bc1 > 1.0))
            return false;

        Bc2 = ((M11*M02M33_M03M32) - (M01*M12M33_M13M32) + (M31*M12M03_M13M02)) / tp->CramerDiv;
        if ((Bc2 < 0.0) || (Bc2 > 1.0))
            return false;

        Bc3 = (- (M11*M02M23_M03M22) - (M21*M12M03_M13M02) + (M01*M12M23_M13M22)) / tp->CramerDiv;
        if ((Bc3 < 0.0) || (Bc3 > 1.0))
            return false;
    }

    // Test the barycentric coordinates to determine if the intersection point is within the object.

    if ((objptr->type == ObjType::Tetrahedron) && ((Bc1 + Bc2 + Bc3) > 1.0))
        return false;

    // At this point we know that the ray intersects the 4D object. If we're only testing for
    // shadowing, return true immediately and leave the specific intersection parameters unaltered.

    if (!mindist)
        return true;

    // We've already tested to see if the intersection point occurs behind some other object in the
    // scene, so now we just load up the intersection parameters and return true.

    *mindist = rayT;

    if (intersect)
        (*intersect) = intr;

    if (normal)
        (*normal) = tp->normal;

    if (objptr->type == ObjType::Tetrahedron) {
        ((Tetrahedron*)(objptr))->Bc1 = Bc1;
        ((Tetrahedron*)(objptr))->Bc2 = Bc2;
        ((Tetrahedron*)(objptr))->Bc3 = Bc3;
    }

    return true;
}

//__________________________________________________________________________________________________

bool HitTriangle (
    ObjInfo    *objptr,     // Sphere to Test
    const Ray4 &ray,       // Trace Ray
    double     *mindist,    // Previous Minimum Distance
    Point4     *intersect,  // Intersection Point
    Vector4    *normal)     // Surface Normal @ Intersection Point
{
    // This is the intersection routine for 2D triangles in 4-space. If the ray intersects triangle
    // and the conditions are met to set the intersection specifics, then the barycentric
    // coordinates of the triangle will also be set according to the intersection point. These
    // values will be used by the shading routines for Phong or Gouraud shading.

    auto &tri = *reinterpret_cast<Triangle*>(objptr);

    // The following segment calculates the intersection point (if one exists) with the ray and the
    // plane containing the polygon. The equation for this is as follows:
    //
    //            +-                       -+
    //            | (V0 - rayO) x vec1,vec2 | . (rayD x vec1,vec2)
    //            +-                       -+
    //     rayT = --------------------------------------------------
    //                                              2
    //                         | rayD x vec1, vec2 |
    //
    // V0 a vertex of the triangle, vec1 is the vector from V0 to another vertex, and vec2 is the
    // vector from V0 to the other vertex.

    auto vecTemp2 = cross(ray.direction, tri.vec1, tri.vec2);
    double div = vecTemp2.normSquared();  // Intersection Equation Divisor
    if (div < epsilon)
        return false;

    auto vecTemp1 = cross(tri.vert[0] - ray.origin, tri.vec1, tri.vec2);

    double rayT = dot(vecTemp1, vecTemp2) / div;  // Ray Equation Real Parameter

    // If the intersection point is behind the ray, then no intersection.

    if (rayT < 0.0)
        return false;

    // If we've previously hit something and the current intersection distance is either less than
    // epsilon or greater then the previous distance, then return false; we don't have a nearer
    // intersection.

    if (mindist && (*mindist > 0) && ((rayT < MINDIST) || (rayT > *mindist)))
        return false;

    Point4 intr = ray(rayT);  // Ray/Plane Intersection Point

    // Compute the triangle normal vector. Since the triangle is embedded in 2-space, we've got an
    // extra degree of freedom floating around, so we need to do some jazz to pin it down. To do
    // this I confine the normal vector to the 3-plane that contains the triangle and the ray. The
    // analogous situation is trying to compute the normal vector to a line in 3-space -- you'd want
    // the normal to be perpendicular to the line and in the plane defined by the line and the ray.
    // It turns out that this is fairly simple to do (although 4D cross products are quite expensive
    // computationally).

    Vector4 _normal;  // Internal Normal Vector
    {
        auto Vtemp = cross(ray.direction, tri.vec1, tri.vec2);
        _normal = cross(Vtemp, tri.vec1, tri.vec2);
    }

    // In order to find the barycentric coordinates of the intersection point in the triangle, we
    // need to find the two minor axes of the normal vector. The intersection point and triangle can
    // then be projected to the plane spanned by the two minor axes without fear of "collapsing" the
    // triangle in the process.

    int ax1, ax2;  // Dominant Axes, tri. Projection
    if (fabs(_normal[0]) < fabs(_normal[1])) {      // X, Y
        ax1 = 0;
        ax2 = 1;
    } else {
        ax1 = 1;
        ax2 = 0;
    }

    if (fabs(_normal[2]) < fabs(_normal[ax2])) {    // X, Y, Z
        if (fabs(_normal[2]) < fabs(_normal[ax1])) {
            ax2 = ax1;
            ax1 = 2;
        } else {
            ax2 = 2;
        }
    }

    if (fabs(_normal[3]) < fabs(_normal[ax2])) {    // X, Y, Z, W
        if (fabs(_normal[3]) < fabs(_normal[ax1])) {
            ax2 = ax1;
            ax1 = 3;
        } else {
            ax2 = 3;
        }
    }

    // Now compute the barycentric coordinates of the intersection point relative to the three
    // vertices of the triangle. The equation used here is as follows (I=intersection point,
    // Vx=Triangle vertex):
    //
    // +-              -+       +-               -+       +-               -+
    // |I[ax1] - V0[ax1]|       |V1[ax1] - V0[ax1]|       |V2[ax1] - V0[ax1]|
    // |I[ax2] - V0[ax2]| = Bc1 |V1[ax2] - V0[ax2]| + Bc2 |V2[ax2] - V0[ax2]|
    // +-              -+       +-               -+       +-               -+
    //
    //                       Bc1 Bc2
    //            +-  -+   +-       -+
    //      or:   | I1 |   | V11 V21 |
    //            | I2 | = | V12 V22 |
    //            +-  -+   +-       -+
    //
    // where Bc1 and Bc2 are the barycentric coordinates for vertex 1 and vertex 2. The barycentric
    // coordinates for vertex 0 is (1-Bc1-Bc2).

    {
        double cramer_div;  // Cramer's Rule Divisor
        double I1, I2;      // Matrix Entries

        I1  = intr[ax1] - tri.vert[0][ax1];
        I2  = intr[ax2] - tri.vert[0][ax2];
        cramer_div = (tri.vec1[ax1] * tri.vec2[ax2])
                   - (tri.vec1[ax2] * tri.vec2[ax1]);

        tri.Bc1 = ((I1 * tri.vec2[ax2]) - (I2 * tri.vec2[ax1])) / cramer_div;

        if ((tri.Bc1 < 0.0) || (tri.Bc1 > 1.0))
            return false;

        tri.Bc2 = ((tri.vec1[ax1] * I2) - (tri.vec1[ax2] * I1)) / cramer_div;

        if ((tri.Bc2 <0.0) || (tri.Bc2 >1.0) || ((tri.Bc1+tri.Bc2) >1.0))
            return false;
    }

    // At this point we know that the ray intersects the 2D triangle. If we're only testing for
    // shadowing, return true immediately and leave the specific intersection parameters unaltered.

    if (!mindist)
        return true;

    // We've already tested to see if the intersection point occurs behind some other object in the
    // scene, so now we just load up the intersection parameters and return true.

    *mindist = rayT;

    (*intersect) = intr;
    (*normal)    = _normal;

    return true;
}
