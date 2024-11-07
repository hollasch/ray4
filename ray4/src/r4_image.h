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
// r4_image.h
//
// This file contains structure and constant declarations for the ray4 image file output format.
//
// The following assumptions are made in the header structure:
//
//     - All fields are packed together (no space between fields).
//     - uchar  fields are unsigned  8-bit integers.
//     - ushort fields are unsigned 16-bit integers.
//     - ulong  fields are unsigned 32-bit integers.
//
// The fields are as follows:
//
//     magic       : uint32    : Equal to 'Ray4' (0x52977934)
//     version     : uint8     : Lower is older. Currently set to 1.
//     bitsperpixel: uint8     : Total bits for each pixel - red, green & blue. Legal values are 12
//                               or 24.
//     aspect      : uint16[3] : Aspect ratio. To form a perfect cube with all edges of equal
//                               length, it would be aspect[0] pixels in the X dimension, aspect[1]
//                               pixels in the Y dimension, and aspect[2] pixels in the Z dimension.
//     start       : uint16[3] : Starting corner of the image cube.
//     end         : uint16[3] : Ending corner of the image cube. The dimension of the data is
//                               (1+last[0]-first[0]) horizontally,
//                               (1+last[1]-first[1]) vertically, and
//                               (1+last[2]-first[2]) deep.
//
// All uint16 and uint32 values are stored in big-endian format. That is, most significant byte
// first, down to the least significant byte.
//
// Scan lines begin and end on byte boundaries, so if the number of pixels on a scanline is odd, the
// last nibble of the last byte of each scan line is ignored.
//
// After the header follows the image data. Each scanline consists of RGB triples, where each triple
// is `bitsperpixel' long. Scanlines are stored left to right, top to bottom, back to front. All
// scanlines begin on even byte boundaries.
//==================================================================================================

#ifndef R4_IMAGE_H
#define R4_IMAGE_H

#include <cstdint>


// Macro Definitions

const uint32_t ray4FormatMagic = 0x52617934L;  // 'Ray4'


// Structure Definitions

struct ImageHeader_1 {
    uint32_t magic;         // Magic Number = R4_IMAGE_ID
    uint8_t  version;       // Image File Version Number = 1
    uint8_t  bitsPerPixel;  // Number of Bits per Pixel
    uint16_t aspect[3];     // Aspect Ratios [X,Y,Z]
    uint16_t start[3];      // Starting Image Pixels for [X,Y,Z]
    uint16_t end[3];        // Ending Image Pixels for [X,Y,Z]
};

#endif
