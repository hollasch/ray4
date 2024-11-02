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

#include "r4_point.h"



//__________________________________________________________________________________________________

bool Point4::operator== (const Point4& other) const {
    return x == other.x && y == other.y && z == other.z && w == other.w;
}

//__________________________________________________________________________________________________

bool Point4::operator!= (const Point4& other) const {
    return !(*this == other);
}

//__________________________________________________________________________________________________

const double& Point4::operator[] (std::size_t index) const {
    if (index <= 0)
        return x;
    if (index <= 1)
        return y;
    if (index <= 2)
        return z;
    return w;
}

//__________________________________________________________________________________________________

double& Point4::operator[] (std::size_t index) {
    if (index <= 0)
        return x;
    if (index <= 1)
        return y;
    if (index <= 2)
        return z;
    return w;
}

//__________________________________________________________________________________________________

Point4& Point4::operator+= (const Vector4& v) {
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;
    return *this;
}

//__________________________________________________________________________________________________

Point4& Point4::operator-= (const Vector4& v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;
    return *this;
}

//__________________________________________________________________________________________________

Vector4 Point4::toVector () const {
    return {x, y, z, w};
}

//__________________________________________________________________________________________________

Vector4 Point4::operator- (const Point4& other) const {
    return { x - other.x, y - other.y, z - other.z, w - other.w };
}

//__________________________________________________________________________________________________

Point4 Point4::operator+ (const Vector4& v) const {
    return { x + v.x, y + v.y, z + v.z, w + v.w };
}

//__________________________________________________________________________________________________

Point4 Point4::operator- (const Vector4& v) const {
    return { x - v.x, y - v.y, z - v.z, w - v.w };
}
