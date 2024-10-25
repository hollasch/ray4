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
// r4_vector.h
//
// This header file contains the structure and macro definitions for three and four-dimensional
// vectors.
//==================================================================================================

#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>


// Vector Definitions

class Vector4 {
  public:
    double x, y, z, w;

    Vector4() = default;
    Vector4(const Vector4&) = default;
    ~Vector4() = default;
    Vector4& operator= (const Vector4 &other) = default;

    Vector4(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {}

    const double& operator[] (std::size_t index) const;
          double& operator[] (std::size_t index);
};


// 4-Vector Macro Definitions

    // This macro yields a list of the vector components

#define V4_List(V)    V[0],V[1],V[2],V[3]

    // This macro provides for scalar multiplication, division, or assignment with a given 4-vector.

#define V4_Scalar(V,assign,k) \
if (1) { \
    V[0] assign (k); \
    V[1] assign (k); \
    V[2] assign (k); \
    V[3] assign (k); \
}else

    // This macro provides for scalar multiplication or assignment with a given 4-vector, where each
    // coordinate has its own scalar factor. For example: V4_4Scalar (A, /=, x, y, z, w).

#define V4_4Scalar(V,assign,a,b,c,d) \
if (1) { \
    V[0] assign (a); \
    V[1] assign (b); \
    V[2] assign (c); \
    V[3] assign (d); \
}else

    // This macro provides for arithmetic operations of two 4-vectors.
    // For example: V4_2Vec( A, +=, B).

#define V4_2Vec(V,assign,U) \
if (1) { \
    V[0] assign U[0]; \
    V[1] assign U[1]; \
    V[2] assign U[2]; \
    V[3] assign U[3]; \
}else

    // The V4_3Vec macro allows for operations with two source 4-vectors and a destination 4-vector,
    // e.g. V4_3Vec (A, +=, B, -, C).

#define V4_3Vec(V,assign,A,op,B) \
if (1) { \
    V[0] assign A[0] op B[0]; \
    V[1] assign A[1] op B[1]; \
    V[2] assign A[2] op B[2]; \
    V[3] assign A[3] op B[3]; \
}else

    // This macro calculates the dot product of two 4-vectors.

#define V4_Dot(V,U) \
    ( (V[0]*U[0]) + (V[1]*U[1]) + (V[2]*U[2]) + (V[3]*U[3]) )

    // V4_Norm calculates the norm (length) of a 4-vector.

#define V4_Norm(V)   sqrt(V4_Dot(V,V))


// Function Declarations

void  V4_Cross     (Vector4, Vector4, Vector4, Vector4);
short V4_Normalize (Vector4);

#endif
