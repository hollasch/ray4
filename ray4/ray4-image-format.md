Ray4 Image File Format
====================================================================================================

A 4D ray trace generates a 3D image, just as a 3D ray trace generates a 2D image. One loose
convention is to name `ray4` scene files with a `.ray4` extension, and the resulting image files use
a `.icube` extension.

There are two versions of the image file. The initial version (1) was originally used when the code
was written in 1991. Version 2 of this file format was adopted in 2024 for the new C++ rewrite of
the `ray4` program.


Data Types
-----------
The following data types are used in the file format:

  | Type    | Size in Bytes | Description
  |---------|---------------|------------------------------------------------
  | uint8   | 1             | unsigned integer, 8 bits
  | uint16  | 2             | unsigned integer, 16 bits
  | uint32  | 4             | unsigned integer, 32 bits
  | float32 | 4             | IEEE 754 floating-point value, single-precision
  | float64 | 8             | IEEE 754 floating-point value, double-precision

Note that all values are written in big-endian (most significant bits first).



Ray4 Image File Format v2
--------------------------

### Image Header
An image file is split into two parts: the header and the image cube data. The file header has the
following format:

    struct ImageHeader {
       uint32 magic = 0x52617934;  // Magic Number = 'Ray4'
       uint8  version = 2;         // Image File Version Number
       uint8  pixelType;
       uint16 bitsPerPixel;        // Number of Bits per Pixel
       uint16 resolution[3];       // Image resolution Rx, Ry, Rz
    }

The following pixel types are supported:

    | Value | Meaning
    |-------|------------------------------------------
    |  0x00 | RGB unsigned integers, 0 to maximum value
    |  0x01 | RGB floating-point values, [0, 1]

The supported number of bits per pixel are:

  - Pixel Type 0x00: 24
  - Pixel Type 0x01: 96 (single-precision RGB), 192 (double-precision RGB)

### Image Data
Image data follows the image header.

Since much of a 3D image cube will likely be the background color, planes, lines and pixels are all
run-length encoded to conserve space.

The very first bit of image data is a single pixel of the background color.

Image data is ordered front-to-back, top-to-bottom, left-to-right.

Following the background pixel color are Rz image planes. Each plane starts with a single unsigned
byte. A value of 0x00 indicates a regular plane composed of Ry pixel lines. A value of 0x01
indicates a plane composed entirely of Rx × Ry background color pixels.

Each pixel line starts with a single byte, just as the image plane. A value of 0x00 indicates a
regular pixel line, and a value of 0x01 indicates a line of Rx pixels of background color.

_Note: come up with a way to run-length encode pixel spans within a line._



Ray4 Image File Format v1
--------------------------
The output file is in two parts: an information header and the image cube data. The information
header has the following C structure definition:

    struct ImageHeader {
       uint32 magic = 0x52617934;  // Magic Number = 'Ray4'
       uint8  version = 1;         // Image File Version Number
       uint8  bitsPerPixel;        // Number of Bits per Pixel
       uint16 aspect[3];           // Aspect Ratios [X,Y,Z]
       uint16 start[3];            // Start Voxels for [X,Y,Z]
       uint16 end[3];              // End   Voxels for [X,Y,Z]
    };

`bitsPerPixel` is the total number of bits for each pixel. Legal values are 12 or 24.

`aspect` specifies three values that represent an idealized size of a pixel in arbitrary units. For
exmaple, an aspect ratio of 1:2:3 means that pixels are spaced one unit in the X dimension, two
units in the Y dimension, and three units in the Z dimension.

A full image is resolutionX × resolutionY × resolutionZ, as specified on the `ray4` command line.
The `start` and `end` points indicate the subset of this full image that is present in the image
file. In other words, the image in the file will be `last[0] - first[0] + 1` pixels horizontally,
`last[1] - first[1] + 1` pixels vertically, and `last[2] - first[2] + 1` pixels deep.

Each scanline consists of pixel colors in red-green-blue order, where each pixel is `bitsPerPixel`
long. Scanlines are stored left-to-right, top-to-bottom, front-to-back. All scanlines begin on even
byte boundaries.
