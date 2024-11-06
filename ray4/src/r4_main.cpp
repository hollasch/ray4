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
// r4_main.cpp
//
// This file contains the main procedures in the Ray4 4D ray tracer.
//==================================================================================================

#define  DEFINE_GLOBALS
#include "ray4.h"
#include "r4_image.h"

#include <time.h>
#include <stdarg.h>

#include <codecvt>
#include <vector>

using ImageHeader = ImageHeader_1;


//__________________________________________________________________________________________________
// Usage Messages

static auto version = "ray4 3.0.0-alpha.3 | 2024-11-05 | https://github.com/hollasch/ray4\n";

static auto usage = R"(
ray4:   4-Space Ray Tracer
usage:  ray4 [-h|/?|--help] [-v|--version]
             [-i|--input|--scene <Scene File Name>]
             [-o|--output|--image <Image File Name>]
             [-r|--resolution <Image Resolution>]
             [-b|--bitsPerPixel <Bits Per Pixel>]
             [-s|--slice <Slice Plane>]

This program constructs a 4D raytraced image of the input scene file, outputing
a 3D image cube of pixels.

Options may also be specified in the RAY4 environment variable. Command-line
options will be effectively concatenated to these options, so that command-line
options may override those in the environment variable.

-h, --help
    Print help + version information and exit.

-v, --version
    Print version information and exit.

-i, --input, --scene <Input File Name> (Required)
    Input filename, typically with extension '.r4'.

-o, --output, --image <Output File Name> (Required)
    Output 3D image cube filename, typically with extension '.icube'.

-r, --resolution <Image Resolution> (Required)
    Image resolution specified as 'X:Y:Z'. X must be greater than zero. If Y or
    Z are omitted or set to zero, they will be set to the same value as the X
    resolution.

-b, --bitsPerPixel <Bits Per Pixel>
    The output number of RGB bits per pixel. This value must be either 12 or 24.
    By default, there are 24 bits per pixel.

-s, --slice <Slice Plane>
    By default, all image planes in the full Z resolution are traced. This
    option allows you to specify a single plane to output. For example, if the
    Z resolution is 100, then any value in the range [0, 99] is acceptable.

Examples:
    ray4 -r 128:128:128 -i scene.r4 -o scene.icube

    ray4 -b12 -r256:256:256 -iMyFile.r4 -omy.icube

    ray4 --resolution 1024:768 --slice 1023 --scene Sphere4 --image s2.icube

)";

//__________________________________________________________________________________________________
// Program Parameters

struct Parameters {
    bool    printHelp       { false };       // Print Help Information & Exit
    bool    printVersion    { false };       // Print Help Information & Exit
    wstring sceneFileName   { };             // Input Ray4 Scene File Name
    wstring imageFileName   { };             // Output Image File Name
    int     bitsPerPixel    { 24 };          // Number of Bits Per Pixel
    int     resolution[3]   { -1, -1, -1 };  // Output Image Resolution
    int     slice           { -1 };          // Image Slice Plane (-1 -> all)
};

enum class OptionType {
    Help,
    Version,
    SceneFileName,
    ImageFileName,
    Resolution,
    BitsPerPixel,
    Slice,
    Unrecognized,
};

struct OptionInfo {
    // A description of a single command-line option.

    OptionType type;
    wstring    singleLetter;  // Single dash letter
    wstring    longName;      // Double dash string
    bool       takesValue;    // If true, takes option value
};

static vector<OptionInfo> optionInfo = {
    {OptionType::Unrecognized,   L"",   L"",               false},
    {OptionType::Help,           L"/?", L"--help",         false},
    {OptionType::Help,           L"-h", L"--help",         false},
    {OptionType::Version,        L"-v", L"--version",      false},
    {OptionType::SceneFileName,  L"-i", L"--input",        true},
    {OptionType::SceneFileName,  L"-i", L"--scene",        true},
    {OptionType::ImageFileName,  L"-o", L"--output",       true},
    {OptionType::ImageFileName,  L"-o", L"--image",        true},
    {OptionType::Resolution,     L"-r", L"--resolution",   true},
    {OptionType::BitsPerPixel,   L"-b", L"--bitsPerPixel", true},
    {OptionType::Slice,          L"-s", L"--slice",        true},
};

//__________________________________________________________________________________________________
// Constant Definitions

#define MIN_SLB_COUNT 5        // Minimum Scanline Buffer Count
#define MIN_SLB_SIZE  (5<<10)  // Minimum Scanline Buffer Size


// File-Global Variables

    Vector4  Gx,  Gy,  Gz;      // Ray-Grid Basis Vectors
    Point4   Gorigin;           // Ray-Grid Origin Point
    int      res[3] = {0,0,0};  // Full Output Image Resolution
    long     scanlsize;         // Scanline Size
    long     slbuff_count;      // Number of Lines in Scanline Buffer
    char    *scanbuff;          // Scanline Buffer
    time_t   StartTime;         // Timestamp


//__________________________________________________________________________________________________

char *MyAlloc (size_t size) {
    // This routine allocates memory using the system malloc() function. If the malloc() call fails
    // to allocate the memory, this routine halts the program with an "out of memory" message.

    char *block;  // Allocated Memory Block

    if (0 == (block = static_cast<char*>(malloc (size))))
        Halt ("Out of memory.");

    return block;
}

//__________________________________________________________________________________________________

void MyFree (void *addr) {
    free (addr);
}

//__________________________________________________________________________________________________

void Halt (const char *message, ...) {
    // This procedure replaces printf() to print out an error message, and has the side effect of
    // cleaning up before exiting (de-allocating memory, closing open files, and so on).

    va_list args;  // List of Optional Arguments

    print ("\n");

    if (message) {
        va_start(args, message);

        print  ("Ray4:  ");
        vprintf (message, args);
        print  ("\n\n");

        va_end(args);
    }

    CloseInput ();
    CloseOutput();

    if (scanbuff)
        DELETE (scanbuff);

    Light *lptr;  // Light-List Pointer
    while ((lptr = lightlist)) {          // Free the lightsource list.
        lightlist = lightlist->next;
        DELETE (lptr);
    }

    ObjInfo *optr;  // Object-List Pointer
    while ((optr = objlist)) {            // Free the object list.
        objlist = objlist->next;
        DELETE (optr);
    }

    Attributes *aptr;  // Attributes-List Pointer
    while ((aptr = attrlist)) {           // Free the attribute list.
        attrlist = attrlist->next;
        DELETE (aptr);
    }

    if (!message) {
        long  elapsed, hours, minutes, seconds;

        print  ("\n");
        printf ("       Total rays cast:  %lu\n", stats.Ncast);
        printf ("  Reflection rays cast:  %lu\n", stats.Nreflect);
        printf ("  Refraction rays cast:  %lu\n", stats.Nrefract);
        printf ("Maximum raytrace level:  %lu\n", stats.maxlevel);

        elapsed = static_cast<long>(time(0) - StartTime);
        hours   = elapsed / 3600;
        minutes = (elapsed - 3600*hours) / 60;
        seconds = (elapsed - 3600*hours - 60*minutes);

        printf ("    Total Elapsed Time:  %ld seconds / %02ld:%02ld:%02ld\n",
        elapsed, hours, minutes, seconds);
    }

    exit ((!message) ? 0 : 1);
}

//__________________________________________________________________________________________________

char *GetField (char *str, int &value) {
    // These subroutine process the command-line arguments. The first two routines get each field of
    // the resolution, aspect ratio, and scan range triples.

    if (!str)
        return nullptr;

    if (!*str) {
        value = 0;
        return str;
    }

    if ((*str < '0') || ('9' < *str))
        return nullptr;

    value = atoi(str);

    while (('0' <= *str) && (*str <= '9'))
        ++str;

    return (*str == ':') ? (str+1) : str;
}

//__________________________________________________________________________________________________

void parseOptionResolution (Parameters &params, const wstring& value) {
    // Parses the 1-3 resolution values from the given string and stores them in the given
    // Parameters. The input value is of the form "X:Y:Z", where Y and Z may be omitted. The input
    // value may not end in a colon. Missing resolutions are set to -1. On any failure, all three
    // resolutions are set to -1.

    params.resolution[0] = params.resolution[1] = params.resolution[2] = -1;

    const wchar_t* ptr = value.c_str();

    // Loop through the X, Y, and Z resolution values.

    for (auto resIndex = 0;  resIndex < 3;  ++resIndex, ++ptr) {
        if (!*ptr || *ptr < '0' || '9' < *ptr) break;

        // Scan in the resolution value.
        params.resolution[resIndex] = 0;
        while (*ptr && '0' <= *ptr && *ptr <= '9') {
            params.resolution[resIndex] = (10 * params.resolution[resIndex]) + (*ptr - L'0');
            ++ptr;
        }

        // After reading in a field, the only legal options are end of string/fields, or a colon to
        // separate the next field.

        if (!*ptr)
            return;

        if (*ptr != L':')
            break;
    }

    wcerr << "ray4: Invalid resolution argument: (" << value << ").\n";
    params.resolution[0] = params.resolution[1] = params.resolution[2] = -1;
    return;
}        

//__________________________________________________________________________________________________

const OptionInfo& getOptionInfo(const wstring& arg) {
    // Given a command-line option, return the corresponding OptionInfo structure.

    if (arg == L"/?")
        return optionInfo[1];

    for (auto i = 0;  i < optionInfo.size();  ++i) {
        if (optionInfo[i].type == OptionType::Unrecognized) continue;
        if (arg == optionInfo[i].longName || arg.starts_with(optionInfo[i].singleLetter))
            return optionInfo[i];
    }

    return optionInfo[0];
}

//__________________________________________________________________________________________________

bool processParameters (Parameters &params, int argc, wchar_t *argv[]) {
    // Process the RAY4 environment variable plus command-line options and load results into the
    // given Parameters object. Returns true on success, false on error.

    vector<wstring> args;

    // Gather arguments from the RAY4 environment variable.

    wstring arg;  // Current Argument String
    char *envVar = getenv("RAY4");
    while (envVar && *envVar) {
        arg.clear();
        while (*envVar && isspace(*envVar)) ++envVar;           // Skip whitespace.
        while (*envVar && !isspace(*envVar)) arg += *envVar++;  // Copy argument string.
        if (!arg.empty()) args.push_back(arg);
    }

    // Add arguments from the command line.

    for (auto argi = 1; argi < argc; ++argi)
        args.push_back(argv[argi]);

    if (args.empty()) {
        params.printHelp = true;
        return true;
    }

    for (auto argi = 0; argi < args.size(); ++argi) {
        auto arg = args[argi];
        auto optionInfo = getOptionInfo(arg);
        wstring optionValue;

        // Get option value if expected.

        if (optionInfo.takesValue) {
            if (arg[1] != '-' && arg[2] != 0) {  // Single letter option joined with argument value.
                optionValue = arg.substr(2);
            } else {
                // Option value is next argument.
                ++argi;
                if (argi < args.size()) {
                    optionValue = args[argi];
                } else {
                    wcerr << "ray4: Missing argument for (" << arg << ").\n";
                    return false;
                }
            }
        }

        if (optionInfo.type == OptionType::Unrecognized) {
            wcerr << "ray4: Unrecognized command argument: (" << arg << ").\n";
            return false;
        }

        switch (optionInfo.type) {
            default:
            case OptionType::Unrecognized:
                wcerr << "ray4: Unrecognized command argument: (" << arg << ").\n";
                return false;

            case OptionType::Help:
                params.printHelp = true;
                break;

            case OptionType::Version:
                params.printVersion = true;
                break;

            case OptionType::SceneFileName:
                params.sceneFileName = optionValue;
                break;

            case OptionType::ImageFileName:
                params.imageFileName = optionValue;
                break;

            case OptionType::Resolution:
                parseOptionResolution(params, optionValue);
                if (params.resolution[0] == -1) return false;
                if (params.resolution[1] == -1) params.resolution[1] = params.resolution[0];
                if (params.resolution[2] == -1) params.resolution[2] = params.resolution[0];
                break;

            case OptionType::BitsPerPixel:
                wchar_t *dummy;
                params.bitsPerPixel = stoi(optionValue);
                break;

            case OptionType::Slice:
                params.slice = stoi(optionValue);
                break;
        }
    }

    // Validate parameters.

    if (params.printHelp || params.printVersion) return true;  // Ignore any other bogus values.

    if (params.sceneFileName.empty()) {
        wcerr << "ray4: Missing input scene file name.\n";
        return false;
    }

    if (params.imageFileName.empty()) {
        wcerr << "ray4: Missing output image file name.\n";
        return false;
    }

    if (params.bitsPerPixel != 12 && params.bitsPerPixel != 24) {
        wcerr << "ray4: Invalid bits per pixel value: " << params.bitsPerPixel << ".\n";
        return false;
    }

    if (params.resolution[0] < 1) {
        wcerr << "ray4: Missing required resolution value(s).\n";
        return false;
    }

    if (params.resolution[2] <= params.slice) {
        wcerr << "ray4: Slice value " << params.slice
              << " out of Z resolution range of [0," << (params.resolution[2] - 1) << "].\n";
        return false;
    }

    return true;
}

//==================================================================================================
// Integer Write Routines -- These routines write out values in big-endian format.
//==================================================================================================

void WriteUInteger8(uint8_t value) {
    WriteBlock(&value, 1);
}

void WriteUInteger16(uint16_t value) {
    uint8_t block[2];
    block[0] = 0xff & (value >> 8);
    block[1] = 0xff & value;

    WriteBlock(block, 2);
}

void WriteUInteger32(uint32_t value) {
    uint8_t block[4];
    block[0] = 0xff & (value >> 24);
    block[1] = 0xff & (value >> 16);
    block[2] = 0xff & (value >>  8);
    block[3] = 0xff & value;

    WriteBlock(block, 4);
}

//__________________________________________________________________________________________________

void WriteHeader(const Parameters& params) {
    WriteUInteger32(ray4FormatMagic);  // 'Ray4' Magic ID
    WriteUInteger8(1);                 // Ray4 Image File Format Version

    WriteUInteger8(params.bitsPerPixel);

    for (int i=0;  i < 3;  ++i)  // Aspect Ratio = 1:1:1
        WriteUInteger16(1);

    // Scan Range Start
    WriteUInteger16(0);
    WriteUInteger16(0);
    WriteUInteger16(0);

    // Scan Range End
    WriteUInteger16(params.resolution[0] - 1);
    WriteUInteger16(params.resolution[1] - 1);
    WriteUInteger16((params.slice >= 0) ? 1 : (params.resolution[2] - 1));
}

//__________________________________________________________________________________________________

void CalcRayGrid (const Parameters& params) {
    // This procedure calculates the ray-grid basis vectors.

    // Get the normalized line-of-sight vector.

    Vector4 lineOfSight = Vto - Vfrom;
    double  lineOfSightNorm = lineOfSight.norm();

    if (!lineOfSight.normalize())
        Halt ("To-Point & From-Point are the same.");

    // Generate the normalized ray-grid basis vectors.

    Gz = cross(Vover, Vup, lineOfSight);
    if (!Gz.normalize())
        Halt ("Line-of-sight, Up vector and Over vector aren't orthogonal.");

    Gy = cross(Gz, lineOfSight, Vover);
    if (!Gy.normalize())
        Halt ("Orthogonality problem while generating GRIDy.");

    Gx = cross(Gy, Gz, lineOfSight);  // Gy, Gz & lineOfSight are all unit vectors.

    // Now compute the proper scale of the grid unit vectors.

    double GNx = 2.0 * lineOfSightNorm * tan(degreeToRadian*Vangle/2.0);
    double GNy = GNx * ((double) res[1] / (double) res[0]);
    double GNz = GNx * ((double) res[2] / (double) res[0]);

    // Scale each grid basis vector.

    Gx *= GNx;
    Gy *= GNy;
    Gz *= GNz;

    // Find the ray-grid origin point.

    Gorigin = Vto - (Gx/2) - (Gy/2) - (Gz/2);

    // Finally, scale the grid basis vectors down by the corresponding resolutions.

    Gx /= res[0];
    Gy /= res[1];
    Gz /= res[2];

    Gorigin += (Gx/2) + (Gy/2) + (Gz/2);
}

//__________________________________________________________________________________________________

void FireRays (const Parameters &params) {
    // This is the main routine that fires the rays through the ray grid and into the 4D scene.

    long   scancount = 0;       // Scanline Counter
    char  *scanptr = scanbuff;  // Scanline Buffer Pointer
    bool   eflag   = true;      // Even RGB Boundary Flag

    // Handle the Z limits where we're only rendering a single slice.
    int zStart, zLimit;
    if (params.slice < 1) {
        zStart = 0;
        zLimit = params.resolution[2];
    } else {
        zStart = params.slice;
        zLimit = params.slice + 1;
    }

    for (auto zIndex = zStart;  zIndex < zLimit;  ++zIndex) {
        Point4 zOrigin = Gorigin + (zIndex*Gz);
        for (auto yIndex = 0;  yIndex < params.resolution[1];  ++yIndex) {
            printf ("%6u %6u\r", params.resolution[2] - zIndex, params.resolution[1] - yIndex);
            fflush (stdout);

            Point4 Yorigin = zOrigin + (yIndex*Gy);

            if (!eflag) {
                ++scanptr;
                eflag = true;
            }

            for (auto xIndex = 0;  xIndex < params.resolution[0];  ++xIndex) {
                Color    color;   // Pixel Color
                Vector4  dir;     // Ray Direction Vector
                Point4   Gpoint;  // Current Grid Point
                double   norm;    // Vector Norm Value

                // Calculate the unit ViewFrom-RayDirection vector.

                Gpoint = Yorigin + (xIndex*Gx);
                dir = Gpoint - Vfrom;
                norm = dir.norm();
                dir /= norm;

                // Fire the ray.

                RayTrace (Ray4(Vfrom, dir), color, 0);

                // Scale the resulting color to 0-255.

                color *= 256.0;
                color = color.clamp(0.0, 255.0);

                // Store the 24-bit RGB triple in the scanline buffer.

                if (params.bitsPerPixel == 24) {

                    *scanptr++ = static_cast<uint8_t>(color.r);
                    *scanptr++ = static_cast<uint8_t>(color.g);
                    *scanptr++ = static_cast<uint8_t>(color.b);

                } else if (eflag) {

                    *scanptr++ = (static_cast<uint8_t>(color.r) & 0xF0) | (static_cast<uint8_t>(color.g) >> 4);
                    *scanptr   = (static_cast<uint8_t>(color.b) & 0xF0);
                    eflag = false;

                } else {

                    *scanptr++ |= (static_cast<uint8_t>(color.r) >> 4);
                    *scanptr++  = (static_cast<uint8_t>(color.g) & 0xF0) | (static_cast<uint8_t>(color.b) >> 4);
                    eflag = true;
                }
            }

            // If the scanline output buffer is full now, write it to disk.

            if (++scancount >= slbuff_count) {
                scancount = 0;
                scanptr   = scanbuff;
                eflag     = true;
                WriteBlock (scanbuff, scanlsize * slbuff_count);
            }
        }
    }

    // If there are scanlines in the scanline buffer, then write the remaining scanlines to disk.

    if (scancount != 0)
        WriteBlock (scanbuff, scanlsize * scancount);
}

//__________________________________________________________________________________________________

void ConvertUnicodeFileNames (const Parameters& params) {
    // For now, our code was written to work with C-style strings, but our input parameters for
    // scene and image filenames are wstrings. As a temporary workaround, convert the wstrings
    // to C-style strings.

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

    auto sceneFileName = converter.to_bytes(params.sceneFileName);
    auto imageFileName = converter.to_bytes(params.imageFileName);

    infile = new char[sceneFileName.size() + 1];
    outfile = new char[imageFileName.size() + 1];

    strcpy(infile,  sceneFileName.c_str());
    strcpy(outfile, imageFileName.c_str());
}

//__________________________________________________________________________________________________

int wmain (int argc, wchar_t *argv[]) {
    // The following is the entry procedure for the ray4 ray tracer.

    Parameters params;
    if (!processParameters(params, argc, argv))
        return 1;

    if (params.printHelp || params.printVersion) {
        if (params.printHelp)
            wcout << usage;
        wcout << version;
        return 0;
    }

    ConvertUnicodeFileNames(params);
    OpenInput(infile);
    ParseInput();

    // If the global ambient factor is zero, then clear all of the ambient factor flags in the
    // objects.

    if ((ambient.r + ambient.g + ambient.b) < epsilon) {
        ObjInfo *optr = objlist;      // Object Pointer

        while (optr) {
            optr->flags &= ~AT_AMBIENT;
            optr = optr->next;
        }
    }

    // Open the output stream and write out the image header (to be followed by the generated
    // scanline data.

    OpenOutput(outfile);
    WriteHeader(params);

    // Determine the size of a single scanline.

    scanlsize = 3 * params.resolution[0];

    if (params.bitsPerPixel == 12) {
        if (scanlsize & 1)
            ++scanlsize;
        scanlsize >>= 1;
    }

    // Compute the number of scanlines and size of the scanline buffer that meets the parameters
    // MIN_SLB_COUNT and MIN_SLB_SIZE.

    if ((MIN_SLB_SIZE / scanlsize) > MIN_SLB_COUNT)
        slbuff_count = MIN_SLB_SIZE / scanlsize;
    else
        slbuff_count = MIN_SLB_COUNT;

    scanbuff = NEW (char, scanlsize * slbuff_count);

    CalcRayGrid(params);   // Calculate the grid cube to fire rays through.

    StartTime = time(0);
    FireRays(params);  // Raytrace the scene.

    Halt(nullptr);     // Clean up and exit.
}
