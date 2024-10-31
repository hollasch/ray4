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
// image4
//
// This tool manipulates 3D image cubes generated by the `ray4` 4D raytracer.
//==================================================================================================

#include <iostream>
#include <string>

using namespace std;


//==================================================================================================
// Information Text Definitions

static auto version = L"image4 0.0.0 | 2024-10-31 | https://github.com/hollasch/ray4/tree/master/image4";

static auto usage = LR"(
image4: 3D image manipulation tool for ray4 ray tracer
usage:  image4 [-h | --help] <-i|--image <imageFile>> [-o|--output <outputImageFile>]
               [-s|--slice <start>[-<end>][x<stepSize>]] [-c|--crop <minX>,<minY>-<maxX>,<maxY>]
               [--query] [--tiled <pixelWidth>[x<horizontalCount>]]

This tool reads a 3D image cube produced by the ray4 4D ray tracer, and either reports information
about the file, or generates one or more images from that image cube, depending on the command line
options.

-h, --help
    Print usage and version information.

-i, --image <imageFile>
    Required argument unless the help option is given. The input image file is produced by the
    `ray4` 4D ray tracer, either the original image cube format, or the newer (2024 and after)
    format.

--query
    If provided, print information about the image cube file instead of generating output images.

-o, --output <outputImageFile>
    Required to produce one or more output image files. This file will be overwritten if it already
    exists. The output image file may contain the substring `##` (exactly two consecutive hash
    characters), which will be replaced with the slice index. If the output file name does not
    contain this substring, the slice index will be appended to the end if more than one slice is
    requested.

-s, --slice <start>[-<end>][x<stepSize>]
    Optional. If unspecified, all slices will be generated. If one number is given, then that slice
    will be output. If a range is specified (such as '--start 5-10'), then all slices in that range
    will be generated. A step size may be included by appending an 'x' and a number. This works for
    whole range ('--slice x5'), offset to end of cube ('--slice 20x5'), and stepped range
    ('--slice 20-80x10').

-c, --crop <minX>,<minY>-<maxX>,<maxY>
    Output images may be optionally cropped to the specified region. If specified, all images will
    be similarly cropped.

--tiled <pixelWidth>[x<horizontalCount>]
    Generates a contact sheet of thumbnail images. If only a pixel width is specified, then the
    output image will be that many pixels wide, and slices will be sized to match the X:Y aspect
    ratio of the image cube. For example, with an image cube that is 200x100x50, and given a tiled
    target of 2000 pixels horizontally, the generated contact sheet will have 64 tiles, 8 thumbnails
    wide by 7 thumbnails, with the bottom row only having two thumbnails. In other words, given N
    slices, the target is a sqrt(N) x sqrt(N) grid.
        If a horizontalCount is specified, then each row will have the specified number of
    thumbnails, sized to fit, with as tall a result as needed to display all slices.
)";


//==================================================================================================
struct Parameters {
    bool    printHelp {false};
    wstring imageCubeFilename;
    bool    printFileInfo {false};
    wstring outputFilepattern;
    int     sliceStart{0};
    int     sliceEnd{-1};
    int     sliceStep{1};
    int     cropXMin{0};
    int     cropXMax{-1};
    int     cropYMin{0};
    int     cropYMax{-1};
    bool    showTiled{false};
    int     tiledPixelWidth{0};
    int     tiledHorizontalCount{0};
};


//==================================================================================================
int main() {
    Parameters params;

    wcout << "Command-line parameters:\n";
    wcout << "    printHelp: " << (params.printHelp ? "true\n" : "false\n");
    wcout << "    imageCubeFilename: '" << params.imageCubeFilename << "'\n";
    wcout << "    printFileInfo: " << (params.printFileInfo ? "true\n" : "false\n");
    wcout << "    outputFilepattern: '" << params.outputFilepattern << "'\n";
    wcout << "    slice: " << params.sliceStart << " - " << params.sliceEnd << " x " << params.sliceStep << '\n';
    wcout << "    crop: " << params.cropXMin << ',' << params.cropYMin
                          << " - " << params.cropXMax << ',' << params.cropYMax << '\n';
    wcout << "    showTiled: " << (params.showTiled ? "true\n" : "false\n");
    wcout << "    tiledPixelWidth: " << params.tiledPixelWidth << '\n';
    wcout << "    tiledHorizontalCount: " << params.tiledHorizontalCount << '\n';

    return 0;
}
