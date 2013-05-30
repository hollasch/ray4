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

File:  r4_vec.c

    This file contains utility routines for 4D vector operations.  See the
header in r4_main.c for more information on Ray4.


Revisions:

92-01-25  Hollasch
    Released to the public domain.

91-05-27  Hollasch
    Fixed bug in tetrahedron/parallelepiped normal and intersection assignment
    code; previous version did not ensure that the passed parameter pointers
    were non-nil.

91-05-15  Hollasch
    Fixed logic flaw in HitSphere t1/t2 selection.

91-05-12  Hollasch
    Added hypersphere debug code.

90-12-19  Hollasch
    Altered tetrahedron-intersection routine to accomodate both tetrahedra
    and parallelepipeds.

90-12-13  Hollasch
    Added internal intersection verification code for tetrahedra.

90-11-20  Hollasch
    Fixed ray-triangle intersection; moved some variable computations from
    on-the-fly to precomputed fields in structures for both 2D triangles and
    tetrahedrons.

90-11-15  Hollasch
    Added tetrahedron & triangle intersection code, and altered intersection
    routines to use the new mindist parameter.

90-09-30  Steve R. Hollasch
    Initial version.

*******************************************************************************/

#include "ray4.h"
#include "vector.h"



/*****************************************************************************
This procedure normalizes a 4-vector.  If the vector is shorter than epsilon,
this routine will return 0, and will not modify the vector.  If the vector has
a non-zero length greater than epsilon, the vector will be normalized and this
routine will return 1.
*****************************************************************************/

short V4_Normalize (Vector4 V)
{
    double norm;    /* Norm of the Vector */

    norm = V4_Dot (V,V);

    if (norm < 1e-30) return 0;

    V4_Scalar (V, /=, norm);

    return 1;
}



/*****************************************************************************
This routine calculates the 4D cross product of the two 4-vectors.
*****************************************************************************/

void  V4_Cross  (
    Vector4  result,  /* Result Vector */
    Vector4  u,       /* Source Vectors */
    Vector4  v,
    Vector4  w)
{
    Real     A, B, C, D, E, F;   /* Intermediate Values */
    Vector4  temp;               /* Intermediate Vector */

    A = (v[0] * w[1]) - (v[1] * w[0]);
    B = (v[0] * w[2]) - (v[2] * w[0]);
    C = (v[0] * w[3]) - (v[3] * w[0]);
    D = (v[1] * w[2]) - (v[2] * w[1]);
    E = (v[1] * w[3]) - (v[3] * w[1]);
    F = (v[2] * w[3]) - (v[3] * w[2]);

    temp[0] =   (u[1] * F) - (u[2] * E) + (u[3] * D);
    temp[1] = - (u[0] * F) + (u[2] * C) - (u[3] * B);
    temp[2] =   (u[0] * E) - (u[1] * C) + (u[3] * A);
    temp[3] = - (u[0] * D) + (u[1] * B) - (u[2] * A);

    V4_2Vec (result,=,temp);
}
