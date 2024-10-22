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

#define  DEFINE_GLOBALS
#include "ray4.h"
#include "r4_globals.h"
#include "r4_image.h"


   /***  Usage Messages  ***/

char notice[] = "\
\n\
Ray4 4D Raytracer, Version 2\n\
Copyright (C) Steve Hollasch 1991-1996. All rights reserved.\n\
\n\n";

char usage[] = "\
ray4 :  4-Space Ray Tracer\n\
usage:  ray4 -s<Scan Range> -a<Aspect Ratios> -r<Image Resolution>\n\
             -b<Bits Per Pixel> -i<Input File> -o<Output File>\n\
\n\
    The arguments to the -s, -a, and -r options are all colon-separated\n\
fields of X:Y:Z triples. All components are unsigned 16-bit integers.\n\
The aspect ratios are the width, height & depth (in some integer units) of\n\
a square voxel. The scan range fields may consist of a range with a low\n\
number, then a '-', and then a high number. You can specify the entire\n\
range with a single underscore character. You may select 12 or 24 color\n\
bits per pixel (default is 24).\n\
\n\
    Examples:  ray4 -b12 -a2:1:2 -s_:_:_ -r256:256:256 <MyFile -omy.img\n\
               ray4 -a 1:1 -r 1024:768 -s 0-1023:0-767 -iSphere4 -os2.img\n\
";

   /***  Constant Definitions  ***/

#define MIN_SLB_COUNT 5        // Minimum Scanline Buffer Count
#define MIN_SLB_SIZE  (5<<10)  // Minimum Scanline Buffer Size


   /***  Function Declarations  ***/

void  ProcessArgs  (int, char**);
char *GetField     (char*, ushort*);
char *GetRange     (char*, ushort*, ushort*);
void  PrintScene   (void);
void  CalcRayGrid  (void);
void  FireRays     (void);


   /***  File-Global Variables  ***/

ImageHdr iheader = {       // Output Image Header
    R4_IMAGE_ID,
    1,
    24,
    {1, 1, 1},
    {0, 0, 0},
    {0xffff, 0xffff, 0xffff}
};

Vector4  Gx,  Gy,  Gz;      // Ray-Grid Basis Vectors
Point4   Gorigin;           // Ray-Grid Origin Point
ushort   res[3] = {0,0,0};  // Full Output Image Resolution
ulong    scanlsize;         // Scanline Size
ulong    slbuff_count;      // Number of Lines in Scanline Buffer
char    *scanbuff;          // Scanline Buffer
time_t   StartTime;         // Timestamp



//==================================================================================================

void main (int argc, char *argv[]) {
    // The following is the entry procedure for the ray4 ray tracer.

    print (notice);

    ProcessArgs (argc, argv);

    OpenInput  ();
    ParseInput ();

    // If the global ambient factor is zero, then clear all of the ambient factor flags in the
    // objects.

    if ((ambient.r + ambient.g + ambient.b) < EPSILON) {
        ObjInfo *optr = objlist;      /* Object Pointer */

        while (optr) {
            optr->flags &= ~AT_AMBIENT;
            optr = optr->next;
        }
    }

    // Open the output stream and write out the image header (to be followed by the generated
    // scanline data.

    OpenOutput ();
    WriteBlock ((char*)(&iheader), sizeof(iheader));

    // Determine the size of a single scanline.

    scanlsize = (3 * (1 + iheader.last[0] - iheader.first[0]));

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

    scanbuff = NEW (char, scanlsize * slbuff_count);

    CalcRayGrid ();   // Calculate the grid cube to fire rays through.

    StartTime = time(0);
    FireRays ();      // Raytrace the scene.

    Halt (nullptr);       // Clean up and exit.
}

//==================================================================================================

void ProcessArgs (int argc, char *argv[]) {
    // This subroutine grabs the command-line arguments and the environment variable arguments (from
    // RAY4) and sets up the raytrace parameters.

    char  *ptr;     /* Scratch String Pointer */
    char  *eptr;    /* Environment Variable Pointer */
    int    ii;      /* Option Array Index */
    char **opta;    /* Option Argument Array */
    int    optc;    /* Option Argument Count */

    /* If the "RAY4" environment variable is not defined, then just use the
    ** command-line options, otherwise concatenate the command-line options
    ** to the options defined in the RAY4 environment variable. */

    if (!(eptr = getenv ("RAY4"))) {
        optc = argc - 1;
        opta = NEW (char*, optc);
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
        opta = NEW (char*, optc);

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

    for (ii=0;  ii < optc;  ++ii) {
        char oc;   /* Option Character */

        if (opta[ii][0] != '-') {
            printf ("ray4:  Unexpected argument (%s).\n", opta[ii]);
            print  (usage);
            exit (1);
        }

        oc = opta[ii][1];

        if (opta[ii][2])
            ptr = opta[ii]+2;
        else
            ptr = opta[++ii];

        switch (oc) {

            case 'a': {
                if (ptr = GetField(ptr,&iheader.aspect[0]), (!ptr || !*ptr))
                    Halt ("Invalid X argument for -a option.");

                if (ptr = GetField(ptr,&iheader.aspect[1]), !ptr)
                    Halt ("Invalid Y argument for -a option.");

                if (ptr = GetField(ptr,&iheader.aspect[2]), !ptr)
                    Halt ("Invalid Z argument for -a option.");

                break;
            }

            case 'b': {
                iheader.bitsperpixel = static_cast<unsigned char>(atoi (ptr));
                if ((iheader.bitsperpixel != 12) && (iheader.bitsperpixel != 24))
                {   printf ("r4toiff:  %d bits per pixel is not supported (select 12 or 24).\n", iheader.bitsperpixel);
                    iheader.bitsperpixel = 24;
                }
                break;
            }

            case 'i': {
                if (infile)
                    DELETE(infile);
                infile = NEW (char, strsize(ptr));
                strcpy (infile, ptr);
                break;
            }

            case 'o': {
                if (outfile)
                    DELETE(outfile);
                outfile = NEW (char, strsize(ptr));
                strcpy (outfile, ptr);
                break;
            }

            case 'r': {
                if (ptr = GetField(ptr,&res[X]), (!ptr || !*ptr))
                    Halt ("Invalid X argument for -r option.");

                if (ptr = GetField(ptr,&res[Y]), !ptr)
                    Halt ("Invalid Y argument for -r option.");

                if (ptr = GetField(ptr,&res[Z]), !ptr)
                    Halt ("Invalid Z argument for -r option.");

                break;
            }

            case 's': {
                ptr = GetRange(ptr,&iheader.first[0],&iheader.last[0]);
                if (!ptr || !*ptr)
                    Halt ("Bad X field argument to -s option.");

                ptr = GetRange(ptr,&iheader.first[1],&iheader.last[1]);
                if (!ptr)
                    Halt ("Bad Y field argument to -s option.");

                ptr = GetRange(ptr,&iheader.first[2],&iheader.last[2]);
                if (!ptr)
                    Halt ("Bad Z field argument to -s option.");

                break;
            }

            default: {
                printf ("ray4:  Unknown option (-%c).\n", oc);
                print  (usage);
                exit (1);
            }
        }
    }

    DELETE (opta);

    if ((iheader.aspect[0] == 0) || (iheader.aspect[1] == 0))
        Halt ("X and Y aspect ratios must be non-zero.");

    if ((res[0] == 0) || (res[1] == 0))
        Halt ("X and Y resolution must be non-zero.");

    if (res[2] == 0)  res[2] = 1;

    if (  (iheader.first[0] >  iheader.last[0])
       || (iheader.first[1] >  iheader.last[1])
       || (iheader.first[2] >  iheader.last[2])
       || (iheader.first[0] >= res[0])
       || (iheader.first[1] >= res[1])
       || (iheader.first[2] >= res[2])
       )
    {
        Halt ("Invalid scan range given.");
    }

    if (iheader.last[0] >= res[0])  iheader.last[0] = res[0]-1;
    if (iheader.last[1] >= res[1])  iheader.last[1] = res[1]-1;
    if (iheader.last[2] >= res[2])  iheader.last[2] = res[2]-1;
}

//==================================================================================================

char *GetField (char *str, ushort *value) {
    // These subroutine process the command-line arguments. The first two routines get each field of
    // the resolution, aspect ratio, and scan range triples.

    if (!str)   return nullptr;
    if (!*str)  return *value=0, str;

    if ((*str < '0') || ('9' < *str))
        return nullptr;

    *value = static_cast<ushort>(atoi (str));

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

    *val1 = *val2 = static_cast<ushort>(atoi (str));

    while (('0' <= *str) && (*str <= '9'))
        ++str;

    if (*str == 0)    return str;
    if (*str == ':')  return str+1;
    if (*str != '-')  return nullptr;

    ++str;
    if ((*str < '0') || ('9' < *str))
        return nullptr;

    *val2 = static_cast<ushort>(atoi (str));
    while (('0' <= *str) && (*str <= '9'))
        ++str;

    return (*str == ':') ? (str+1) : str;
}

//==================================================================================================

void Halt (char *message, ...) {
    // This procedure replaces printf() to print out an error message, and has the side effect of
    // cleaning up before exiting (de-allocating memory, closing open files, and so on).

    Attributes *aptr;   /* Attributes-List Pointer */
    Light      *lptr;   /* Light-List Pointer */
    ObjInfo    *optr;   /* Object-List Pointer */
    va_list     args;   /* List of Optional Arguments */

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

    if (infile)    DELETE (infile);
    if (outfile)   DELETE (outfile);
    if (scanbuff)  DELETE (scanbuff);

    while ((lptr = lightlist)) {          /* Free the lightsource list. */
        lightlist = lightlist->next;
        DELETE (lptr);
    }

    while ((optr = objlist)) {            /* Free the object list. */
        objlist = objlist->next;
        DELETE (optr);
    }

    while ((aptr = attrlist)) {           /* Free the attribute list. */
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

//==================================================================================================

char *MyAlloc (size_t size) {
    // This routine allocates memory using the system malloc() function. If the malloc() call fails
    // to allocate the memory, this routine halts the program with an "out of memory" message.

    char *block;  // Allocated Memory Block

    if (0 == (block = static_cast<char*>(malloc (size))))
        Halt ("Out of memory.");

    return block;
}

//==================================================================================================

void MyFree (void *addr) {
    free (addr);
}

//==================================================================================================

void CalcRayGrid (void) {
    // This procedure calculates the ray-grid basis vectors.

    Real    GNx, GNy, GNz;  // Ray-Grid Vector Norms
    Vector4 Los;            // Line-of-Sight Vector
    Real    losnorm;        // Line-of-Sight-Vector Norm

    // Get the normalized line-of-sight vector.

    V4_3Vec (Los,=,Vto,-,Vfrom);
    losnorm = V4_Norm (Los);

    if (losnorm < EPSILON)
        Halt ("To-Point & From-Point are the same.");
    V4_Scalar (Los, /=, losnorm);

    // Generate the normalized ray-grid basis vectors.

    V4_Cross (Gz, Vover,Vup,Los);
    if (! V4_Normalize(Gz))
        Halt ("Line-of-sight, Up vector and Over vector aren't orthogonal.");

    V4_Cross (Gy, Gz,Los,Vover);
    if (! V4_Normalize(Gy))
        Halt ("Orthogonality problem while generating GRIDy.");

    V4_Cross (Gx, Gy,Gz,Los);      // Gy, Gz & Los are all unit vectors.

    // Now compute the proper scale of the grid unit vectors.

    GNx = 2.0 * losnorm * tan (DegreeToRadian*Vangle/2.0);

    GNy = GNx
        * ((Real) res[Y] / (Real) res[X])
        * ((Real)iheader.aspect[Y]/ (Real)iheader.aspect[X]);

    GNz = GNx
        * ((Real) res[Z] / (Real) res[X])
        * ((Real)iheader.aspect[Z]/ (Real)iheader.aspect[X]);

    // Scale each grid basis vector.

    V4_Scalar (Gx, *=, GNx);
    V4_Scalar (Gy, *=, GNy);
    V4_Scalar (Gz, *=, GNz);

    // Find the ray-grid origin point.

    Gorigin[0] = Vto[0] - (Gx[0]/2.0) - (Gy[0]/2.0) - (Gz[0]/2.0);
    Gorigin[1] = Vto[1] - (Gx[1]/2.0) - (Gy[1]/2.0) - (Gz[1]/2.0);
    Gorigin[2] = Vto[2] - (Gx[2]/2.0) - (Gy[2]/2.0) - (Gz[2]/2.0);
    Gorigin[3] = Vto[3] - (Gx[3]/2.0) - (Gy[3]/2.0) - (Gz[3]/2.0);

    // Finally, scale the grid basis vectors down by the corresponding resolutions.

    V4_Scalar (Gx, /=, res[X]);
    V4_Scalar (Gy, /=, res[Y]);
    V4_Scalar (Gz, /=, res[Z]);

    Gorigin[0] += (Gx[0]/2.0) + (Gy[0]/2.0) + (Gz[0]/2.0);
    Gorigin[1] += (Gx[1]/2.0) + (Gy[1]/2.0) + (Gz[1]/2.0);
    Gorigin[2] += (Gx[2]/2.0) + (Gy[2]/2.0) + (Gz[2]/2.0);
    Gorigin[3] += (Gx[3]/2.0) + (Gy[3]/2.0) + (Gz[3]/2.0);
}

//==================================================================================================

void FireRays () {
    // This is the main routine that fires the rays through the ray grid and into the 4D scene.

    bool     eflag;                   // Even RGB Boundary Flag
    ulong    scancount;               // Scanline Counter
    char    *scanptr;                 // Scanline Buffer Pointer
    ushort   Xindex, Yindex, Zindex;  // Ray-Grid Loop Indices */
    Point4   Yorigin, Zorigin;        // Ray-Grid Axis Origins

    scancount = 0;
    scanptr   = scanbuff;
    eflag     = true;

    for (Zindex=iheader.first[Z];  Zindex <= iheader.last[Z];  ++Zindex) {
        V4_3Vec (Zorigin, =, Gorigin, +, Zindex*Gz);
        for (Yindex=iheader.first[Y];  Yindex <= iheader.last[Y];  ++Yindex) {
            printf ("%6u %6u\r",
            iheader.last[Z] - Zindex, iheader.last[Y] - Yindex);
            fflush (stdout);

            V4_3Vec (Yorigin, =, Zorigin, +, Yindex*Gy);

            if (!eflag) {
                ++scanptr;
                eflag = true;
            }

            for (Xindex=iheader.first[X];  Xindex <= iheader.last[X];  ++Xindex) {
                Color    color;   // Pixel Color
                Vector4  Dir;     // Ray Direction Vector
                Point4   Gpoint;  // Current Grid Point
                Real     norm;    // Vector Norm Value

                // Calculate the unit ViewFrom-RayDirection vector.

                V4_3Vec (Gpoint, =, Yorigin, +, Xindex*Gx);
                V4_3Vec (Dir, =, Gpoint, -, Vfrom);
                norm = V4_Norm (Dir);
                V4_Scalar (Dir, /=, norm);

                // Fire the ray.

                RayTrace (Vfrom, Dir, &color, (ulong)(0));

                // Scale the resulting color to 0-255.

                Color_Scale (color, *=, 256.0);
                color.r = CLAMP (color.r, 0.0, 255.0);
                color.g = CLAMP (color.g, 0.0, 255.0);
                color.b = CLAMP (color.b, 0.0, 255.0);

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
                WriteBlock (scanbuff, scanlsize * slbuff_count);
            }
        }
    }

    // If there are scanlines in the scanline buffer, then write the remaining scanlines to disk.

    if (scancount != 0)
        WriteBlock (scanbuff, scanlsize * scancount);
}
