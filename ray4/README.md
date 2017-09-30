ray4
====================================================================================================

This is ray4, a 4D raytracer I wrote for my master’s thesis in May 1991
([“Four-Space Visualization of 4D Objects”](http://hollasch.github.io/ray4)).

This project includes the following files:

| File           | Description
|----------------|-----------------------------------------------
| README.md      | This File
| copyright      | Ray4 Release Copyright
| ray4.doc       | Ray4 Documentation
| makefile       | Program Make File (Unix)
| makefile-Amiga | Program Make File (Amiga)
| ray4.h         | Main Header File
| r4_globals.h   | Ray4 Global Variables
| r4_image.h     | Ray4 Image Cube File Format Structures
| vector.h       | Vector Operation Macros
| v4cross.c      | Vector Cross-Product Routine
| v4normalize.c  | Vector Normalization Routine
| r4_main.c      | Main Ray4 Functions
| r4_io.c        | Ray4 Input/Output Functions (for portability)
| r4_parse.c     | Ray4 Input File Parsing Routines
| r4_trace.c     | Ray4 Raytracing Routines
| r4_hit.c       | Ray4 Ray-Object Intersection Routines
| r4tosgi.c      | Ray4 Output File to GL-Device Display Program
| r4toiff.c      | Ray4 Output File to Amiga display & IFF File
| scube.r4<br>sphere.r4<br>tcube.r4<br>dots.r4<br>dots3.r4<br>dots4.r4<br>format.r4<br>blank.r4<br>wdots.r4 | Example Input Files

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
