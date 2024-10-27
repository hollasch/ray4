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
#ifndef R4_VECTOR_H
#define R4_VECTOR_H

#include <cstddef>


class Vector4 {
    // Represents a four-dimenisonal vector.

  public:
    double x, y, z, w;

    Vector4() = default;
    Vector4(const Vector4&) = default;
    ~Vector4() = default;
    Vector4& operator= (const Vector4 &other) = default;

    Vector4(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {}

    const double& operator[] (std::size_t index) const;
          double& operator[] (std::size_t index);

    Vector4 operator- () const;
    Vector4 operator- (const Vector4& other) const;
    Vector4 operator+ (const Vector4& other) const;
    Vector4& operator*= (double scale);
    Vector4& operator/= (double divisor);

    // If the vector length is longer than a certain tolerance, the vector is resized to a length of
    // one, and the function returns true. Otherwise, the vector is considered to have a length of
    // zero, is left unchanged, and the function returns false.
    bool normalize();

    double normSquared() const;
    double norm() const;
};

Vector4 operator* (double s, const Vector4& v);
Vector4 operator* (const Vector4& v, double s);

double  dot   (const Vector4&, const Vector4&);
Vector4 cross (const Vector4&, const Vector4&, const Vector4&);

#endif
