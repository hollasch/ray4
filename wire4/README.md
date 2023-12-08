wire4
====================================================================================================

This is the initial release of the 4D wireframe viewer that I wrote for my master’s thesis,
“Four-Space Visualization of 4D Objects”, in May 1991. The thesis is also available in this same
GitHub project or from http://hollasch.github.io/ray4.


`wire4` User Manual
--------------------
See the [Wire4 User Manual][] for usage information.


`psplot`
---------
`psplot` is a simple PostScript plotting tool to convert wire4 output to a PostScript file. See the
[psplot User Manual][] for more information.


Source Code
------------
This release includes the following files:

| File            | Description
|:----------------|:----------------------------------------
| `README.md`     | _This File_
| `copyright`     | _Copyright Notice_
| `wire4.doc`     | _User’s Manual for wire4_
| `psplot.doc`    | _User’s Manual for psplot_
| `makefile`      | _Unix Make File_
| `wire4.h`       | _wire4 Main Header File_
| `vector.h`      | _Header File for Vector Operations_
| `psplot.c`      | _Source for psplot_
| `strcmpic.c`    | _Case-Insensitive String Comparison_
| `v3cross.c`     | _3D Vector Cross Product_
| `v4cross.c`     | _4D Vector Cross Product_
| `w4_display.c`  | _Graphics Display Functions_
| `w4_geometry.c` | _4D Geometry Functions_
| `w4_main.c`     | _Main wire4 File_
| `w4_parse.c`    | _wire4 Input File Parsing Functions_
| `axes.w4`<br>`cube.w4`<br>`cube3d.w4`<br>`curve1.w4`<br>`curve2.w4`<br>`curve3.w4`<br>`dual.w4`<br>`frustum.w4`      | _wire4 Input Files_

You will probably need to tweak the makefile to get it to run on your platform. I’ve tried to make
this as straightforward as possible, but let me know if you have problems.

Wire4 supports wireframe display of arbitrary 4D objects (from ASCII input files) and displays
objects with user-specified edge colors, depth-cueing, and interactive 3D and 4D rotation. Both 4D
to 3D and 3D to 2D projections can be interactively set to parallel or perspective projection. Also
included in the package is a program to generate simple PostScript files of arbitrary wireframe
views.

Wire4 uses the GL library, so it is mostly useful for SGI boxes, although other platforms that have
a GL compatibility library may also run the program.

Also available for release is my 4D raytracing program. Let me know if you are interested and I’ll
send you a copy. Ray4 runs on any platform that supports C and floating point, but currently the
only display programs available are for the Amiga, SGI platforms, and Sun rasterfiles.

Finally, I’d like to acknowledge and thank the folks who volunteered to beta-test this software for
me:

- Greg Aharonian
- Brian Carney
- Mark Ferneau
- Marty Ryan

----
Steve Hollasch / steve@hollasch.net


[Wire4 User Manual]:  wire4.md
[psplot User Manual]: psplot.md
