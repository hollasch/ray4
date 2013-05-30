
    This is ray4, version 1.30.  Ray4 is a 4D raytracer that I wrote for my
master's thesis in May 1991 ("Four-Space Visualization of 4D Objects").  The
thesis is available via anonymous FTP in both PostScript and TROFF formats,
at ftp.kpc.com.

    This release should include the following files:

    README                // This File
    copyright             // Ray4 Release Copyright
    ray4.doc              // Ray4 Documentation

    makefile              // Program Make File (Unix)
    makefile-Amiga        // Program Make File (Amiga)
    ray4.h                // Main Header File
    r4_globals.h          // Ray4 Global Variables
    r4_image.h            // Ray4 Image Cube File Format Structures
    vector.h              // Vector Operation Macros
    v4cross.c             // Vector Cross-Product Routine
    v4normalize.c         // Vector Normalization Routine
    r4_main.c             // Main Ray4 Functions
    r4_io.c               // Ray4 Input/Output Functions (for portability)
    r4_parse.c            // Ray4 Input File Parsing Routines
    r4_trace.c            // Ray4 Raytracing Routines
    r4_hit.c              // Ray4 Ray-Object Intersection Routines
    r4tosgi.c             // Ray4 Output File to GL-Device Display Program
    r4toiff.c             // Ray4 Output File to Amiga display & IFF File

    scube.r4              // Example Input Files
    sphere.r4
    tcube.r4
    dots.r4
    dots3.r4
    dots4.r4
    format.r4
    blank.r4
    wdots.r4

    You will probably need to tweak the makefile to get it to run on
your platform.  I've tried to make this as straightforward as possible,
but let me know if you have problems.


Beta Testers
------------
    Greg Aharonian
    Brian Carney
    Mark Craig
    Mark Ferneau
    Marty Ryan


Example Run
-----------

    ray4 -b12 -idots.r4 -odots.icube -a10:11:20 -r78:65:12
    r4toiff dots.icube -t


/////////////////////////////////////////////////////////////////////////////
Steve Hollasch (hollasch@kpc.com)                            08-February-1993
