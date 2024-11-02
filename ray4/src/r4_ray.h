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
#ifndef R4_RAY_H
#define R4_RAY_H

#include "r4_vector.h"
#include "r4_point.h"



//__________________________________________________________________________________________________

class Ray4 {
    // A ray (with origin and direction) in four-dimensional space.

  public:
    Point4  origin;
    Vector4 direction;

    Ray4() = default;
    Ray4(const Ray4&) = default;
    ~Ray4() = default;
    Ray4& operator= (const Ray4 &other) = default;

    Ray4(Point4 origin, Vector4 direction) : origin(origin), direction(direction) {}

    bool operator== (const Ray4& other) const;
    bool operator!= (const Ray4& other) const;

    Point4 operator() (double t) const;
};

#endif
