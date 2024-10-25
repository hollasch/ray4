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

#include "ray4.h"
#include "r4_color.h"

inline double clamp_double(double x, double min, double max) {
    return clamp(x, min, max);
}

Color& Color::operator*= (double scale) {
    r *= scale;
    g *= scale;
    b *= scale;

    return *this;
}

Color& Color::operator*= (const Color& other) {
    r *= other.r;
    g *= other.g;
    b *= other.b;

    return *this;
}

Color& Color::operator+= (const Color& other) {
    r += other.r;
    g += other.g;
    b += other.b;

    return *this;
}

Color Color::clamp(double min, double max) const {
    return Color(clamp_double(r, min, max), clamp_double(g, min, max), clamp_double(b, min, max));
}

Color operator* (double scale, const Color& color) {
    return Color(scale * color.r, scale * color.g, scale * color.b);
}

Color operator* (const Color& color, double scale) {
    return scale * color;
}

Color operator* (const Color& c1, const Color& c2) {
    return Color(c1.r * c2.r, c1.g * c2.g, c1.b * c2.b);
}
