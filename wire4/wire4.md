wire4 User Manual
====================================================================================================

Introduction
-------------
The wire4 program was developed in conjunction with my master's thesis on
[Four-Space Visualization of 4D Objects][thesis], May, 1991, at Arizona State University for the
Computer Science Department.

This program allows for the interactive display of 4D wireframe objects. It provides for 3D and 4D
rotations, 3D and 4D parallel & perspective projection, depthcueing, and other operations. The user
provides the 4D object definition and has control over both structure, display modes, and object
edge coloring.


Target Platform
----------------
This program uses GL for the graphics output and user input. It has been tested on the SGI 3130, SGI
4D, SGI 310 VGX, and IBM RS/6000 (with the SGI Sabine (Turbo) card, NOT with the GTO card).


Input File Structure
---------------------
The input file format is free form — whitespace characters may occur at any position in the input
file. Whitespace characters include formfeed, newline, carriage return, tab, space, colon,
semi-colon, forward slash, and comma.

Comments begin with the '#' character and end with the next newline character.

Input statements consist of a keyword, followed by some arguments. Supported keywords include the
following:

    ColorFar        <RGB>
    ColorMap        <Color # [1,255]> <RGB>
    ColorNear       <RGB>
    Depthcue        <"On" / "Off">
    DepthcueLevels  <Integer [1,255]>
    DepthFar        <Real>
    DepthNear       <Real>
    EdgeList        <# of Edges> <Vertex #> <Vertex #> <Color #>
                             ... <Vertex #> <Vertex #> <Color #>
    From3           <3D Point>
    From4           <4D Point>
    Over4           <4D Vector>
    Project3        <"Parallel" / "Perspective">
    Project4        <"Parallel" / "Perspective">
    RefCube         <"On" / "Off">
    To3             <3D Point>
    To4             <4D Point>
    Up3             <3D Vector>
    Up4             <4D Vector>
    Vangle3         <Real (0.0,180.0)>
    Vangle4         <Real (0.0,180.0)>
    VertexList      <# of Vertices> <4D Point> ... <4D Point>

Keywords may be supplied in mixed case; wire4 does not distinguish between uppercase and lowercase.
The parameters are defined as follows:

    <RGB>       :  <Red [0,255]> <Green [0,255> <Blue [0,255]>
    <3D Point>  :  <Real> <Real> <Real>
    <4D Point>  :  <Real> <Real> <Real> <Real>
    <3D Vector> :  <Real> <Real> <Real>
    <4D Vector> :  <Real> <Real> <Real> <Real>

The coordinates are given in left-handed coordinates; the Z axis points into the screen. The
analogous situation holds for the 4D coordinates.


Object Definition
------------------
The 4D object wireframe definition is given in the input file as a sequence of vertices and a
sequence of edges connecting pairs of the given vertices.

The vertex list is specified with the "VertexList" keyword. The keyword is followed by the number of
vertices in the 4D object, and then by the 4D points of each vertex. Vertices are numbered from 0 to
N-1, for example:

    VertexList  4
        1.0  0.0  0.0  0.0       # Vertex 0
        1.0  1.0  0.0  0.0       # Vertex 1
        1.0  1.0  1.0  0.0       # Vertex 2
        1.0  1.0  1.0  1.0       # Vertex 3

The edges of the 4D object wireframe are given with the EdgeList statement. The "EdgeList" keyword
is followed by the number of edges in the 4D object wireframe, then by two vertices and a color
index for each edge, for example:

    EdgeList  6
        0,1 : 1     # Vertex 0 → Vertex 1;  Color #1
        1,2 : 1     # Vertex 1 → Vertex 2;  Color #1
        2,3 : 1     # Vertex 2 → Vertex 3;  Color #1
        3,0 : 1     # Vertex 3 → Vertex 0;  Color #1
        0,2 : 2     # Vertex 0 → Vertex 2;  Color #2
        1,3 : 2     # Vertex 1 → Vertex 3;  Color #2

The color list is built via the "ColorMap" keyword. The colormap statement specifies a given color
index and the RGB value of that color. The RGB components may range from 0 to 255. The number of
available colors depends on the graphics display device, but on the Silicon Graphics Iris 3130,
there are 255 available colors (1–255). Colormap statements look like this:

    Colormap  1  100 100 100       # Color 1 is grey.
    Colormap  2  175  40 225       # Color 2 is violet.
    Colormap  3  235 150  70       # Color 3 is orange.


View Definitions
-----------------
There are two separate views of the 4D object:  one in four-space, and one in three-space. The
object is first projected from four-space to an intermediate 3D viewing region that corresponds to
the cube with vertices <±1, ±1, ±1, ±1>. This projection is governed by the 4D viewing parameters.

The projection of the 4D object is then further projected to the 2D display device viewport, and is
governed by the 3D viewing parameters.

The 3D viewing parameters consist of From3, To3, Up3 and Vangle3. The From3 point is the 3D location
we're looking from. The 3D point we're looking at is given by the To3 point. The 3D up direction is
given by the 3D Up3 vector and the 3D viewing angle is given by the Vangle3 parameter. The viewing
angle value must be in the range (0, 180).

The 4D viewing parameters are similar to the 3D viewing parameters except for the additional Over4
parameter. Because of the extra degree of freedom in four-space, we need an additional view vector
to specify the orientation of view. The Over4 vector must not be parallel to the Up4 vector or the
line of sight.


Running Wire4
--------------
`wire4` can be run in two ways. If it's called with command-line arguments, it opens up the object
description file specified on the command line. If it's called without arguments, it reads in the
object description from the standard input stream.

Interactive controls consist of the horizontal mouse movement, and either the three mouse buttons or
the keyboard.


Projection Modes
-----------------
Both the 3D and 4D projection modes can take on one of two forms: parallel projection or perspective
projection. In perspective projection, objects that are further away appear smaller than objects
that are closer. With parallel projection, objects do not diminish in size as they move farther
away.

The projection modes can be set in the object input file with the Project3 and Project4 keywords.
The argument for these statements may be either "Parallel" or "Perspective". For example:

    Project4  Perspective
    Project3  Parallel

Additionally, the projection modes can be selected interactively. It's often helpful to toggle this
interactively to try to get a handle on the structure of a 4D object, although it can also further
confuse you  =^).

The projection modes can be controlled by the mouse buttons or by the keyboard.

To toggle the 3D projection mode via mouse buttons, hold down the right mouse button and click the
left mouse button. Pressing the '3' key also toggles the 3D projection mode.

To toggle the 4D projection mode via mouse buttons, hold down the right mouse button and click the
middle mouse button. You can also toggle the 4D projection mode by pressing the '4' key.

The current projection modes will be displayed in the proper status boxes on the right side of the
display screen.

You can also display the outline of the intermediate 3D projection region with the "RefCube"
statement in the input file (as in "RefCube On" or "Refcube Off"), or interactively by pressing the
'r' key. The display of the intermediate projection region is helpful because it helps you to
distinguish between 3D rotations and 4D rotations without looking away from the object display.


Object Rotation
----------------
The object can be rotated interactively via the horizontal movement of the mouse. There are nine
possible rotations of the object view:  six for the 4D view of the object and three for the 3D view
of the 4D projection.

To select the proper rotation, you first need to select the rotation space (3D or 4D). This can be
done by clicking the middle mouse button or by pressing the 's' key. The current rotation space will
be displayed at the right side of the display screen.

To select the rotation plane, you can click the left mouse button or press the 'p' key. The 3D
rotation planes are XY, YZ and ZX. The 4D rotation planes are XY, YZ, ZX, XW, YW, and ZW. The
current rotation plane will be displayed at the right side of the display screen.


Depthcueing
------------
The 4D object can also be displayed with 4D depthcueing. The depthcueing is based on the 4D W
eye-coordinate. Typically, objects that are farther away are rendered with darker colors and objects
that are closer are rendered with lighter colors to simulate objects that fade with distance.

Interactively, depthcueing can be toggled on and off by pressing the 'd' key. Depthcueing can also
be initially on or off via the "DepthCue" statement in the input file, for example:

    DepthCue  on

or

    DepthCue  off

The depthcue parameters are given in the input file. The first parameters are the color of the
nearest points and the color of the farthest points. These are controlled by the ColorNear and
ColorFar parameters, for example:

    ColorNear:  255  20  20        # Red means close.
    ColorFar :   20 255  20        # Green means far.

or

    ColorNear   255 255 255        # White means close.
    ColorFar     50  50  50        # Dark grey means far.

Each edge drawn will be gradated into some number of segments to simulate a gradation from a near
color to a far color. The number of segments for each edge is 16 by default, but can be set with the
DepthCueLevels statement in the input file. For example,

    DepthCueLevels  5

will set the number of gradations per edge to 5. Obviously, the more the gradations, the slower the
display.

The final depthcue parameters deal with the range of depthcueing. If the near and far depthcue
distances are not given in the input file, they will automatically be chosen so that the object best
fills the depthcue region. To get the widest range, the near distance would be the distance to the
closest vertex and the far distance would be the distance to the farthest vertex. However, the near
and far planes can be set in the input file as well. Locations farther than the far distance will
all be given the ColorFar color, and locations closer than the near distance will all be assigned
the ColorNear color. You may wish to set the near location to somewhere inside the object or closer
than the closest point on the object.

For example, if you set both the near and far depthcue distances to the distance of the center of
the object, then half of the object would be colored with the ColorNear color and half of the object
would be colored with the ColorFar color. This would have the effect of imitating a cutting plane
through the object perpendicular to the line of sight.

The depthcue distance parameters are set with the DepthFar and DepthNear keywords, for example:

    DepthFar   100.0
    DepthNear  -10.0

Again, if you don't know what distances to choose, you can just leave out these statements to have
wire4 determine a good range for you.


Image Snapshots
----------------
For hardcopy display purposes, I added a snapshot feature to this program. This feature captures the
current 2D object image and adds it to a file called "snapshots". The current viewing parameters are
given as lines preceded with the '#' character, and the 2D edges are given as a series of "M <Real
[0,1]> <Real [0,1]>" (move) commands and "D <Real [0,1]> <Real [0,1]>" (draw) commands.

You can use the `psplot' program to convert this file to a simple PostScript file. For more
information, refer to psplot.doc.

To append the current image to the snapshots file, press the 'c' key.


[thesis]: https://hollasch.github.io/ray4/Four-Space_Visualization_of_4D_Objects.html
