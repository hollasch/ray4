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
// r4_main.c
//
// This file contains the main procedures in the Ray4 4D ray tracer.
//==================================================================================================


#include <time.h>
#include <memory.h>
#include <string.h>
#include <stdarg.h>

#define DEFINE_GLOBALS
#include "ray4.h"
#include "r4_image.h"


    /************************/
    /***  Usage Messages  ***/
    /************************/

char notice[] = "\
\n\
ray4-c 2.0.0-alpha.2 | 2024-10-24 | https://github.com/hollasch/ray4\n\
\n";

char usage[] = "\
ray4-c:  4-Space Ray Tracer\n\
usage :  ray4-c -r<Image Resolution> -i<Input Filename> -o<Output Filename>\n\
                [-h] [-a<Aspect Ratio>] [-b<Bits Per Pixel>] [-s<Scan Range>]\n\
\n\
This program constructs a 4D raytraced image of the input scene file, outputing\n\
a 3D image cube of pixels.\n\
\n\
Options may also be specified in the RAY4 environment variable. Command-line\n\
options will be effectively concatenated to these options, so that command-line\n\
options may override those in the environment variable.\n\
\n\
-r<Image Resolution>\n\
    Image resolution specified as 'X:Y:Z'. X must be greater than zero. If Y or\n\
    Z are omitted or set to zero, they will be set to the same value as the X\n\
    resolution.\n\
\n\
-i<Input Filename>\n\
    Input filename, typically with extension '.r4'.\n\
\n\
-o<Output Filename>\n\
    Output 3D image cube filename, typically with extension '.icube'.\n\
\n\
-h\n\
    Print help information.\n\
\n\
-a<Aspect Ratio> (Optional)\n\
    The 3D image aspect ratio, specified as 'X:Y:Z'. X and Y must be non-zero.\n\
    By default, the pixel aspect ratio is 1:1:1.\n\
\n\
-b<Bits Per Pixel> (Optional)\n\
    The output number of RGB bits per pixel. This value must be either 12 or 24.\n\
    By default, 24 bits per pixel.\n\
\n\
-s<Scan Range> (Optional)\n\
    A subset of the full image resolution to raytrace, expressed as 'X:Y:Z'.\n\
    Each component may be a single number, a range of numbers (expressed as\n\
    'S-E', where S is the scan start, and E is the scan end, inclusive), or the\n\
    special value '_', which indicates the entire resolution for that dimension.\n\
    Unspecified values are interpreted as zero. By default, the full resolution\n\
    is scanned (equivalent to '_:_:_').\n\
\n\
Examples:\n\
    ray4-c -r128:128:128 <scene.r4 -o scene.icube\n\
\n\
    ray4-c -b12 -a2:1:2 -r256:256:256 -s_:_:_ <MyFile.r4 -omy.icube\n\
\n\
    ray4-c -a 1:1 -r 1024:768 -s 0-1023:0-767 -iSphere4 -os2.icube\n\
\n\
";

    /******************************/
    /***  Constant Definitions  ***/
    /******************************/

#define MIN_SLB_COUNT  5        // Minimum Scanline Buffer Count
#define MIN_SLB_SIZE   (5<<10)  // Minimum Scanline Buffer Size


    /*******************************/
    /***  File-Global Variables  ***/
    /*******************************/

ImageHdr iheader = {           // Output Image Header
    R4_IMAGE_ID,
    1,
    24,
    {1,1,1},
    {0,0,0}, {0xffff,0xffff,0xffff}
};

Vector4  Gx,  Gy,  Gz;      // Ray-Grid Basis Vectors
Point4   Gorigin;           // Ray-Grid Origin Point
ushort   res[3] = {0,0,0};  // Full Output Image Resolution
ulong    scanlsize;         // Scanline Size
ulong    slbuff_count;      // Number of Lines in Scanline Buffer
char    *scanbuff;          // Scanline Buffer
ulong    StartTime;         // Timestamp



//==================================================================================================

char *GetField (char *str, ushort *value) {
    // Gets the next colon-separated field from the input string. Returns pointer to the next field
    // in the string.

    if (!str)
        return nullptr;

    if (!*str) {
        *value = 0;
        return str;
    }

    if ((*str < '0') || ('9' < *str))
        return nullptr;

    *value = atoi(str);

    while (('0' <= *str) && (*str <= '9'))
        ++str;

    return (*str == ':') ? (str+1) : str;
}

//==================================================================================================

char *GetRange (
    char   *str,    // Source String
    ushort *val1,   // First Destination Value of Range
    ushort *val2)   // Second Destination Value of Range
{
    // Gets the two values in a colon-separated string of fields. A single value yields two equal
    // values for val1 and val2. Returns the next field in the string.

    if (!str)
        return nullptr;

    if (!*str) {
        *val1 = *val2 = 0;
        return str;
    }

    if (*str == '_') {
        *val1 = 0;
        *val2 = 0xFFFF;
        return (str[1] == ':') ? (str+2) : (str+1);
    }

    if ((*str < '0') || ('9' < *str))
        return nullptr;

    *val1 = *val2 = atoi(str);

    while (('0' <= *str) && (*str <= '9'))
        ++str;

    if (*str == 0)
        return str;

    if (*str == ':')
        return str+1;

    if (*str != '-')
        return nullptr;

    ++str;
    if ((*str < '0') || ('9' < *str))
        return nullptr;

    *val2 = atoi(str);
    while (('0' <= *str) && (*str <= '9'))
        ++str;

    return (*str == ':') ? (str+1) : str;
}

//==================================================================================================

void Halt (
    char *message,      // Exception Message
    ... )               // Optional Message Arguments
{
    // This procedure replaces printf() to print out an error message, and has the side effect of
    // cleaning up before exiting (de-allocating memory, closing open files, and so on).

    Attributes *aptr;   // Attributes-List Pointer
    Light      *lptr;   // Light-List Pointer
    ObjInfo    *optr;   // Object-List Pointer
    va_list     args;   // List of Optional Arguments

    print("\n");

    if (message) {
        va_start(args, message);

        print("Ray4:  ");
        vprintf(message, args);
        print("\n\n");

        va_end(args);
    }

    CloseInput();
    CloseOutput();

    if (infile)    DELETE (infile);
    if (outfile)   DELETE (outfile);
    if (scanbuff)  DELETE (scanbuff);

    while (lptr = lightlist) {          // Free the lightsource list.
        lightlist = lightlist->next;
        DELETE(lptr);
    }

    while (optr = objlist) {            // Free the object list.
        objlist = objlist->next;
        DELETE(optr);
    }

    while (aptr = attrlist) {           // Free the attribute list.
        attrlist = attrlist->next;
        DELETE(aptr);
    }

    if (!message) {
        long  elapsed, hours, minutes, seconds;

        print ("\n");
        printf("       Total rays cast:  %lu\n", stats.Ncast);
        printf("  Reflection rays cast:  %lu\n", stats.Nreflect);
        printf("  Refraction rays cast:  %lu\n", stats.Nrefract);
        printf("Maximum raytrace level:  %lu\n", stats.maxlevel);

        elapsed = time(0) - StartTime;
        hours   = elapsed / 3600;
        minutes = (elapsed - 3600*hours) / 60;
        seconds = (elapsed - 3600*hours - 60*minutes);

        printf("    Total Elapsed Time:  %ld seconds / %02ld:%02ld:%02ld\n",
        elapsed, hours, minutes, seconds);
    }

    exit((!message) ? 0 : 1);
}

//==================================================================================================

char *MyAlloc (size_t size) {
    // This routine allocates memory using the system malloc() function. If the malloc() call fails
    // to allocate the memory, this routine halts the program with an "out of memory" message.

    char *block;  // Allocated Memory Block

    if (0 == (block = malloc(size)))
        Halt("Out of memory.");

    return block;
}

//==================================================================================================

void MyFree (void *addr) {
    free(addr);
}

//==================================================================================================

void CalcRayGrid (void) {
    // This procedure calculates the ray-grid basis vectors.

    Real    GNx, GNy, GNz;    // Ray-Grid Vector Norms
    Vector4 Los;              // Line-of-Sight Vector
    Real    losnorm;          // Line-of-Sight-Vector Norm

    // Get the normalized line-of-sight vector.

    V4_3Vec(Los,=,Vto,-,Vfrom);
    losnorm = V4_Norm(Los);

    if (losnorm < EPSILON)
        Halt("To-Point & From-Point are the same.");
    V4_Scalar(Los, /=, losnorm);

    // Generate the normalized ray-grid basis vectors.

    V4_Cross(Gz, Vover,Vup,Los);
    if (! V4_Normalize(Gz))
        Halt("Line-of-sight, Up vector and Over vector aren't orthogonal.");

    V4_Cross(Gy, Gz,Los,Vover);
    if (! V4_Normalize(Gy))
        Halt("Orthogonality problem while generating GRIDy.");

    V4_Cross(Gx, Gy,Gz,Los);      // Gy, Gz & Los are all unit vectors.

    // Now compute the proper scale of the grid unit vectors.

    GNx = 2.0 * losnorm * tan(DegreeToRadian*Vangle/2.0);

    GNy = GNx
        * ((Real) res[Y] / (Real) res[X])
        * ((Real)iheader.aspect[Y]/ (Real)iheader.aspect[X]);

    GNz = GNx
        * ((Real) res[Z] / (Real) res[X])
        * ((Real)iheader.aspect[Z]/ (Real)iheader.aspect[X]);

    // Scale each grid basis vector.

    V4_Scalar(Gx, *=, GNx);
    V4_Scalar(Gy, *=, GNy);
    V4_Scalar(Gz, *=, GNz);

    // Find the ray-grid origin point.

    Gorigin[0] = Vto[0] - (Gx[0]/2.0) - (Gy[0]/2.0) - (Gz[0]/2.0);
    Gorigin[1] = Vto[1] - (Gx[1]/2.0) - (Gy[1]/2.0) - (Gz[1]/2.0);
    Gorigin[2] = Vto[2] - (Gx[2]/2.0) - (Gy[2]/2.0) - (Gz[2]/2.0);
    Gorigin[3] = Vto[3] - (Gx[3]/2.0) - (Gy[3]/2.0) - (Gz[3]/2.0);

    // Finally, scale the grid basis vectors down by the corresponding resolutions.

    V4_Scalar(Gx, /=, res[X]);
    V4_Scalar(Gy, /=, res[Y]);
    V4_Scalar(Gz, /=, res[Z]);

    Gorigin[0] += (Gx[0]/2.0) + (Gy[0]/2.0) + (Gz[0]/2.0);
    Gorigin[1] += (Gx[1]/2.0) + (Gy[1]/2.0) + (Gz[1]/2.0);
    Gorigin[2] += (Gx[2]/2.0) + (Gy[2]/2.0) + (Gz[2]/2.0);
    Gorigin[3] += (Gx[3]/2.0) + (Gy[3]/2.0) + (Gz[3]/2.0);
}

//==================================================================================================

void FireRays () {
    // This is the main routine that fires the rays through the ray grid and into the 4D scene.

    boolean  eflag;                   // Even RGB Boundary Flag
    ulong    scancount;               // Scanline Counter
    char    *scanptr;                 // Scanline Buffer Pointer
    ushort   Xindex, Yindex, Zindex;  // Ray-Grid Loop Indices
    Point4   Yorigin, Zorigin;        // Ray-Grid Axis Origins

    scancount = 0;
    scanptr   = scanbuff;
    eflag     = true;

    for (Zindex=iheader.start[Z];  Zindex <= iheader.end[Z];  ++Zindex) {
        V4_3Vec(Zorigin, =, Gorigin, +, Zindex*Gz);
        for (Yindex=iheader.start[Y];  Yindex <= iheader.end[Y];  ++Yindex) {
            printf("%6u %6u\r", iheader.end[Z] - Zindex, iheader.end[Y] - Yindex);
            fflush(stdout);

            V4_3Vec(Yorigin, =, Zorigin, +, Yindex*Gy);

            if (!eflag) {
                ++scanptr;
                eflag = true;
            }

            for (Xindex=iheader.start[X];  Xindex <= iheader.end[X];  ++Xindex) {
                Color    color;   // Pixel Color
                Vector4  Dir;     // Ray Direction Vector
                Point4   Gpoint;  // Current Grid Point
                Real     norm;    // Vector Norm Value

                // Calculate the unit ViewFrom-RayDirection vector.

                V4_3Vec(Gpoint, =, Yorigin, +, Xindex*Gx);
                V4_3Vec(Dir, =, Gpoint, -, Vfrom);
                norm = V4_Norm(Dir);
                V4_Scalar(Dir, /=, norm);

                // Fire the ray.

                RayTrace(Vfrom, Dir, &color, (ulong)(0));

                // Scale the resulting color to 0-255.

                Color_Scale(color, *=, 256.0);
                color.r = CLAMP(color.r, 0.0, 255.0);
                color.g = CLAMP(color.g, 0.0, 255.0);
                color.b = CLAMP(color.b, 0.0, 255.0);

                // Store the 24-bit RGB triple in the scanline buffer.

                if (iheader.bitsperpixel == 24) {
                    *scanptr++ = (uchar)(color.r);
                    *scanptr++ = (uchar)(color.g);
                    *scanptr++ = (uchar)(color.b);
                } else if (eflag) {
                    *scanptr++ = ((uchar)(color.r) & 0xF0)
                               | ((uchar)(color.g) >> 4);
                    *scanptr   = ((uchar)(color.b) & 0xF0);
                    eflag = false;
                } else {
                    *scanptr++ |= ((uchar)(color.r) >> 4);
                    *scanptr++  = ((uchar)(color.g) & 0xF0)
                                | ((uchar)(color.b) >> 4);
                    eflag = true;
                }
            }

            // If the scanline output buffer is full now, write it to disk.

            if (++scancount >= slbuff_count) {
                scancount = 0;
                scanptr   = scanbuff;
                eflag     = true;
                WriteBlock(scanbuff, scanlsize * slbuff_count);
            }
        }
    }

    // If there are scanlines in the scanline buffer, then write the
    // remaining scanlines to disk.

    if (scancount != 0)
        WriteBlock(scanbuff, scanlsize * scancount);
}

//==================================================================================================

void ProcessArgs (int argc, char *argv[]) {
    // This subroutine grabs the command-line arguments and the environment variable arguments (from
    // RAY4) and sets up the raytrace parameters.

    char  *ptr;     // Scratch String Pointer
    char  *eptr;    // Environment Variable Pointer
    int    ii;      // Option Array Index
    char **opta;    // Option Argument Array
    int    optc;    // Option Argument Count

    // If the "RAY4" environment variable is not defined, then just use the command-line options,
    // otherwise concatenate the command-line options to the options defined in the RAY4 environment
    // variable.

    if (!(eptr = getenv("RAY4"))) {
        optc = argc - 1;
        opta = NEW(char*, optc);
        for (ii=0;  ii < optc;  ++ii)
            opta[ii] = argv[ii+1];
    } else {
        int opti;   // Option Index

#       define SPACE(c)   ((c == ' ') || (c == '\t'))

        for (optc=0, ptr=eptr;  *ptr;  ) {
            while (SPACE(*ptr))
                ++ptr;
            if (!*ptr)
                break;
            ++optc;
            while (++ptr, *ptr && !SPACE(*ptr))
                continue;
        }

        optc += argc - 1;
        opta = NEW(char*, optc);

        for (opti=0, ptr=eptr;  *ptr;  ) {
            while (SPACE(*ptr))
                ++ptr;

            if (!*ptr)
                break;

            opta[opti++] = ptr;

            while (++ptr, *ptr && !SPACE(*ptr))
                continue;

            if (*ptr)
                *ptr++ = 0;
        }

        for (ii=1;  ii < argc;  ++ii)
            opta[opti++] = argv[ii];
    }

    if (optc <= 0) {
        print(usage);
        exit(0);
    }

    for (ii=0;  ii < optc;  ++ii) {
        char oc;   // Option Character

        if (opta[ii][0] != '-') {
            printf("ray4:  Unexpected argument (%s).\n", opta[ii]);
            print(usage);
            exit(1);
        }

        oc = opta[ii][1];

        if (opta[ii][2])
            ptr = opta[ii]+2;
        else
            ptr = opta[++ii];

        switch (oc) {
            case 'a': {
                if (ptr = GetField(ptr,&iheader.aspect[0]), (!ptr || !*ptr))
                    Halt("Invalid X argument for -a option.");

                if (ptr = GetField(ptr,&iheader.aspect[1]), !ptr)
                    Halt("Invalid Y argument for -a option.");

                if (ptr = GetField(ptr,&iheader.aspect[2]), !ptr)
                    Halt("Invalid Z argument for -a option.");

                break;
            }

            case 'b': {
                iheader.bitsperpixel = atoi(ptr);
                if ((iheader.bitsperpixel != 12) && (iheader.bitsperpixel != 24)) {
                    printf("ray4:  %d bits per pixel is not supported (select 12 or 24).\n", iheader.bitsperpixel);
                    iheader.bitsperpixel = 24;
                }
                break;
            }

            case 'h': {
                print(usage);
                exit(0);
                break;
            }

            case 'i': {
                if (infile)
                    DELETE(infile);
                infile = NEW(char, strsize(ptr));
                strcpy(infile, ptr);
                break;
            }

            case 'o': {
                if (outfile)
                    DELETE(outfile);
                outfile = NEW(char, strsize(ptr));
                strcpy(outfile, ptr);
                break;
            }

            case 'r': {
                char* resArg = ptr;

                ptr = GetField(ptr, &res[X]);

                if (!ptr || !*ptr) {
                    res[Y] = res[Z] = res[X];
                } else {
                    ptr = GetField(ptr, &res[Y]);

                    if (!ptr || !*ptr)
                        res[Z] = res[X];
                    else
                        ptr = GetField(ptr, &res[Z]);
                }

                if (ptr && *ptr)
                    Halt("Bad resolution argument to -r option (%s).", resArg);

                break;
            }

            case 's': {
                ptr = GetRange(ptr,&iheader.start[0],&iheader.end[0]);
                if (!ptr || !*ptr)
                    Halt("Bad X field argument to -s option.");

                ptr = GetRange(ptr,&iheader.start[1],&iheader.end[1]);
                if (!ptr)
                    Halt("Bad Y field argument to -s option.");

                ptr = GetRange(ptr,&iheader.start[2],&iheader.end[2]);
                if (!ptr)
                    Halt("Bad Z field argument to -s option.");

                break;
            }

            default: {
                printf("ray4:  Unknown option (-%c).\n", oc);
                print(usage);
                exit(1);
            }
        }
    }

    DELETE(opta);

    if ((iheader.aspect[0] < 1) || (iheader.aspect[1] < 1) || ((iheader.aspect[2] < 1)))
        Halt("Aspect ratio values must be greater than zero.");

    if (res[0] < 1)
        Halt("X resolution must be greater than zero.");

    if (  (iheader.start[0] >  iheader.end[0])
       || (iheader.start[1] >  iheader.end[1])
       || (iheader.start[2] >  iheader.end[2])
       || (iheader.start[0] >= res[0])
       || (iheader.start[1] >= res[1])
       || (iheader.start[2] >= res[2])
       )
    {
        Halt ("Invalid scan range given.");
    }

    if (iheader.end[0] >= res[0])  iheader.end[0] = res[0]-1;
    if (iheader.end[1] >= res[1])  iheader.end[1] = res[1]-1;
    if (iheader.end[2] >= res[2])  iheader.end[2] = res[2]-1;
}

//==================================================================================================
// Integer Write Routines. These routines write out values in big-endian format.
//==================================================================================================

void WriteInteger8 (uchar value) {
    WriteBlock(&value, 1);
}

void WriteInteger16 (ushort value) {
    unsigned char block[2];
    block[0] = 0xff & (value >> 8);
    block[1] = 0xff & value;

    WriteBlock(block, 2);
}

void WriteInteger32(ulong value) {
    unsigned char block[4];
    block[0] = 0xff & (value >> 24);
    block[1] = 0xff & (value >> 16);
    block[2] = 0xff & (value >>  8);
    block[3] = 0xff & value;

    WriteBlock(block, 4);
}

//==================================================================================================

void WriteHeader (ImageHdr* header) {
    // Write the image header to the output stream.

    WriteInteger32(header->magic);
    WriteInteger8(header->version);
    WriteInteger8(header->bitsperpixel);

    for (int i=0;  i < 3;  ++i)
        WriteInteger16(header->aspect[i]);

    for (int i=0;  i < 3;  ++i)
        WriteInteger16(header->start[i]);

    for (int i=0;  i < 3;  ++i)
        WriteInteger16(header->end[i]);
}

//==================================================================================================

void main (int argc, char *argv[]) {
    // The following is the entry procedure for the ray4 ray tracer.

    print(notice);

    ProcessArgs(argc, argv);

    OpenInput();
    ParseInput();

    // If the global ambient factor is zero, then clear all of the ambient factor flags in the
    // objects.

    if ((ambient.r + ambient.g + ambient.b) < EPSILON) {
        ObjInfo *optr = objlist;      // Object Pointer

        while (optr) {
            optr->flags &= ~AT_AMBIENT;
            optr = optr->next;
        }
    }

    // Open the output stream and write out the image header (to be followed by the generated
    // scanline data.

    OpenOutput();
    WriteHeader(&iheader);

    // Determine the size of a single scanline.

    scanlsize = (3 * (1 + iheader.end[0] - iheader.start[0]));

    if (iheader.bitsperpixel == 12) {
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

    scanbuff = NEW(char, scanlsize * slbuff_count);

    CalcRayGrid();        // Calculate the grid cube to fire rays through.
    StartTime = time(0);  // Reset the clock
    FireRays();           // Raytrace the scene.

    Halt(nullptr);  // Clean up and exit.
}
