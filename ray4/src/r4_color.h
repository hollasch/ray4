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

#ifndef R4_COLOR_H
#define R4_COLOR_H

class Color {
  public:
    double r, g, b;  // Each color should be in [0,1].

    Color() = default;
    Color(const Color&) = default;
    ~Color() = default;

    Color(double red, double green, double blue) : r(red), g(green), b(blue) {}

    bool operator== (const Color& other) const;
    bool operator!= (const Color& other) const;

    Color& operator*= (double scale);
    Color& operator*= (const Color& other);
    Color& operator+= (const Color& other);

    Color clamp(double min, double max) const;
};

Color operator* (double, const Color&);
Color operator* (const Color&, double);
Color operator* (const Color&, const Color&);

#endif
