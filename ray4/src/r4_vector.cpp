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
// r4_vector.cpp
//
// This file contains utility routines for 4D vector operations. See the header in r4_main.c for
// more information on Ray4.
//==================================================================================================

#include "r4_vector.h"

#include <cmath>



//==================================================================================================
const double& Vector4::operator[] (std::size_t index) const {
    if (index <= 0)
        return x;
    if (index <= 1)
        return y;
    if (index <= 2)
        return z;
    return w;
}

//==================================================================================================
double& Vector4::operator[] (std::size_t index) {
    if (index <= 0)
        return x;
    if (index <= 1)
        return y;
    if (index <= 2)
        return z;
    return w;
}

//==================================================================================================
bool Vector4::operator== (const Vector4& other) const {
    return x == other.x && y == other.y && z == other.z && w == other.w;
}

//==================================================================================================
bool Vector4::operator!= (const Vector4& other) const {
    return !(*this == other);
}

//==================================================================================================
Vector4 Vector4::operator- () const {
    return {-x, -y, -z, -w};
}

//==================================================================================================
Vector4 Vector4::operator+ (const Vector4& other) const {
    return {x+other.x, y+other.y, z+other.z, w+other.w};
}

//==================================================================================================
Vector4 Vector4::operator- (const Vector4& other) const {
    return {x-other.x, y-other.y, z-other.z, w-other.w};
}

//==================================================================================================
Vector4 Vector4::operator* (double s) const {
    return {s*x, s*y, s*z, s*w};
}

//==================================================================================================
Vector4 Vector4::operator/ (double d) const {
    return {x/d, y/d, z/d, w/d};
}

//==================================================================================================
Vector4& Vector4::operator*= (double scale) {
    x *= scale;
    y *= scale;
    z *= scale;
    w *= scale;
    return *this;
}

//==================================================================================================
Vector4& Vector4::operator/= (double divisor) {
    *this *= 1/divisor;
    return *this;
}

//==================================================================================================
double Vector4::normSquared () const {
    return x*x + y*y + z*z + w*w;
}

//==================================================================================================
double Vector4::norm () const {
    return std::sqrt(normSquared());
}

//==================================================================================================
bool Vector4::normalize () {
    // This function attempts to normalize the 4-vector. If the vector is larger than some
    // epsilon, it will normalize the vector, otherwise it will leave it unchanged.

    const auto zeroLengthThreshold = 1.0e-15;

    auto vecNormSquared = normSquared();

    if (vecNormSquared < zeroLengthThreshold)
        return false;

    *this /= sqrt(vecNormSquared);
    return true;
}

//==================================================================================================
double dot(const Vector4& a, const Vector4& b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

//==================================================================================================
Vector4 cross(const Vector4& u, const Vector4& v,const Vector4& w) {
    // This routine calculates the 4D cross product of three 4-vectors.

    double A, B, C, D, E, F;  // Intermediate Values

    A = (v.x * w.y) - (v.y * w.x);
    B = (v.x * w.z) - (v.z * w.x);
    C = (v.x * w.w) - (v.w * w.x);
    D = (v.y * w.z) - (v.z * w.y);
    E = (v.y * w.w) - (v.w * w.y);
    F = (v.z * w.w) - (v.w * w.z);

    return {
          (u.y * F) - (u.z * E) + (u.w * D),
        - (u.x * F) + (u.z * C) - (u.w * B),
          (u.x * E) - (u.y * C) + (u.w * A),
        - (u.x * D) + (u.y * B) - (u.z * A)
    };
}
