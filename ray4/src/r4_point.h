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
#ifndef R4_POINT_H
#define R4_POINT_H

#include <cstddef>
#include "r4_vector.h"


class Point4 {
    // Represents a four-dimensional point.

  public:
    double x, y, z, w;

    Point4() = default;
    Point4(const Point4&) = default;
    ~Point4() = default;
    Point4& operator= (const Point4 &other) = default;

    Point4(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {}

    bool operator== (const Point4& other) const;
    bool operator!= (const Point4& other) const;

    const double& operator[] (std::size_t index) const;
          double& operator[] (std::size_t index);

    Point4& operator+= (const Vector4&);
    Point4& operator-= (const Vector4&);

    // Returns the vector from the origin to the point. In other words, the vector with all
    // coordinates equal to the point coordinates.
    Vector4 toVector() const;

    Vector4 operator- (const Point4&) const;
    Point4 operator+ (const Vector4&) const;
    Point4 operator- (const Vector4&) const;
};

inline Point4 operator+ (const Vector4& v, const Point4& p) {
    return p + v;
}

#endif
