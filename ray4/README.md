ray4
====================================================================================================

This is `ray4`, a 4D raytracer I wrote for my master’s thesis in May 1991
([_Four-Space Visualization of 4D Objects_]).

This project includes the following files:

    README.md ........ This file
    copyright ........ Ray4 release copyright
    ray4.doc ......... Ray4 documentation
    makefile ......... Program make file (Unix)
    makefile-Amiga ... Program make file (Amiga)
    ray4.h ........... Main header file
    r4_globals.h ..... Ray4 global variables
    r4_image.h ....... Ray4 image cube file format structures
    vector.h ......... Vector operation macros
    v4cross.c ........ Vector cross-product routine
    v4normalize.c .... Vector normalization routine
    r4_main.c ........ Main ray4 functions
    r4_io.c .......... Ray4 input/output functions (for portability)
    r4_parse.c ....... Ray4 input file parsing routines
    r4_trace.c ....... Ray4 raytracing routines
    r4_hit.c ......... Ray4 ray-object intersection routines
    r4tosgi.c ........ Ray4 output file to GL-device display program
    r4toiff.c ........ Ray4 output file to Amiga display & IFF file
    *.r4 ............. Example input files

You will probably need to tweak the makefile to get it to run on your platform. I’ve tried to make
it as straightforward as possible, but let me know if you have problems.


Beta Testers
------------
- Greg Aharonian
- Brian Carney
- Mark Craig
- Mark Ferneau
- Marty Ryan


Example Run
-----------

    ray4 -b12 -idots.r4 -odots.icube -a10:11:20 -r78:65:12
    r4toiff dots.icube -t


----
Steve Hollasch / steve@hollasch.net



[_Four-Space Visualization of 4D Objects_]: https://hollasch.github.io/ray4/Four-Space_Visualization_of_4D_Objects.html
