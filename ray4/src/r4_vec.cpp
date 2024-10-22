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
// r4_vec.c
//
// This file contains utility routines for 4D vector operations. See the header in r4_main.c for
// more information on Ray4.
//==================================================================================================

#include "ray4.h"
#include "vector.h"



//==================================================================================================

short V4_Normalize (Vector4 V) {
    // This function attempts to normalize a 4-vector. If the vector is not shorter than some
    // epsilon, it will normalize the vector and return 1. If the vector is shorter than the
    // epsilon, it does not alter the vector and returns 0.

    double norm;  // Norm of the Vector

    norm = V4_Dot (V,V);

    if (norm < 1e-30) return 0;

    V4_Scalar (V, /=, sqrt(norm));

    return 1;
}

//==================================================================================================

void V4_Cross (
    Vector4  result,  // Result Vector
    Vector4  u,       // Source Vectors
    Vector4  v,
    Vector4  w)
{
    // This routine calculates the 4D cross product of the two 4-vectors.

    Real     A, B, C, D, E, F;  // Intermediate Values
    Vector4  temp;              // Intermediate Vector

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
