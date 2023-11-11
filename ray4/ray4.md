ray4 User Manual
====================================================================================================


Introduction
-------------
The ray4 4D raytracer was developed in conjunction with my master's degree thesis:
[_Four-Space Visualization of 4D Objects_][thesis], Steve Hollasch, Arizona State University,
Computer Science, May 1991.

This raytracer works much like a regular 3D raytracer, except that the view is in four-space, the
lighting is true 4D lighting, and the objects are 4D objects in four-space. Unlike 3D raytracers,
which produce a 2D grid of RGB values, ray4 generates an image that is a 3D volume of RGB values.
Hence the size of the output image is usually much larger than for a regular 3D raytracer.

The input to the raytracer is a simple ASCII free-format file that specifies the global parameters,
the viewing paramters, the object definitions and the lighting paramters.

In this document, lines that denote example input file lines will be preceeded with a bar, e.g.:

    This is an example input file line.
    This is another.


Target Platforms
-----------------
The ray4 program should run on any platform that supports floating point math and Unix file I/O
operations. The ray4 program reads in the ASCII description file and generates the resulting image
cube file.

Included with the ray4 program are display programs: one for the Amiga microcomputer and one for
platforms that support the Silicon Graphics GL language. These display programs allow for the
display of slices of the image cube, either one at a time or several in a tiled display.
Volume-rendering programs have not been written for the 4D raytracer yet.


Input File Guidelines
----------------------
The input file is an ASCII with a fairly free format. In other words, whitespace characters separate
tokens and may occur anywhere in the file. Whitespace characters include tab, linefeed, vertical
tab, formfeed, carriage return, space, comma, open & close square brackets, and open & close curly
braces.

The following rules hold for the input file:

  * The following are whitespace characters: tab,  linefeed,  vertical tab,  formfeed,  carriage
    return, space,  comma,  open square bracket,  close square bracket, open curly brace,  close
    curly brace

  * Comments begin with the '>' character and end at the next carriage return or linefeed.

  * All keywords are case insensitive.

  * Maximum token length is 80 characters.

  * Each directive and all subfield names are significant only to the first five letters.

  * Attribute names are significant only to the first 20 letters.

  * Attributes inherit values of the most recently _named_ attribute.

  * Colors are specified as RGB triples by real numbers in the range of [0.0, 1.0]. Three numbers
    are expected.

"Directives" are commands that take "subfield" arguments. For example, the "View" directive has
subfields "from", "to", "up", "over" and "angle". Directives follow the format

    <DirectiveName> ( <Subfield1> <Args> ... <SubfieldN> <Args> )

The following directives are supported:

    View            (from, to, up, over, angle)
    Light           (point, direction, color)
    Attributes      (ambient, diffuse, specular, transparency, shine, indexrefraction, reflective)
    Plane           (attributes, normal, point)
    Sphere          (attributes, center, radius)
    Tetrahedron     (attributes, vertices)
    Parallelepiped  (attributes, vertices)

In addition to these directives there are the following global parameters:

    Ambient         <RGB>
    Background      <RGB>
    MaxDepth        <Integer [0,65535]>


Global Parameters
------------------
The global parameters have the following syntax:

    Ambient     <RGB>
    Background  <RGB>
    MaxDepth    <Integer [0,65535]>

The "ambient" parameter specifies the global ambient coefficient.


Viewing Specification
----------------------
The viewing parameters are specified with the "view" directive:

    View
    (   From   <4-Point>
        To     <4-Point>
        Up     <4-Vector>
        Over   <4-Vector>
        Angle  <Real (0,180)>
    )

The from subfield specifies the viewpoint, or where we're looking from.

The to subfield specifies the point that we're looking to. Together with the from-point, this
specifies the line of sight.

To specify the orientation of the view, we need an up vector and an additional vector (the
over-vector) to account for the additional degree of freedom in four-space.

Finally, the viewing angle specifies the telephoto/wide-angle effect of the view. The viewing angle
must be in the range of 0 to 180 degrees, exclusive. The smaller the viewing angle, the greater the
telephoto effect, and the larger the viewing angle, the greater the wide-angle effect.

Here's an example 4D view:

    View                                   > 4D View
    (   From   { 0.0, 0.0,  0.0, -1.0 }    > Along the W-Axis
        To     { 0.0, 0.0,  0.0,  0.0 }    > Towards the Origin
        Up     { 0.0, 1.0,  0.0,  0.0 }    > The "Up" Direction
        Over   { 0.0, 0.0, -1.0,  0.0 }    > The "Over" Direction
        Angle  45.0                        > A Reasonable Angle
    )

Note that the braces and commas are really just whitespace, but are added for readability.


Lighting Parameters
--------------------
Ray4 supports two types of 4D lighting: directional and point source. The directional type is
equivalent to a point light source at an infinite distance.

For directional light sources, there are two paramters: the direction _to_ the light source and the
color of the light source. For point light sources, the location of the light source is given,
rather than the direction to the light source.

The subfields for the light directive are "position"/"direction" and "color", for example:

    > Point light source located at the origin:

    Light
    (   position   { 0.0, 0.0, 0.0, 0.0 }
        color      [.800 .600 .122]
    )

    > Directional light source along the W axis:

    Light
    (   direction  { 0.0, 0.0, 0.0, -1.0 }
        color      [.750 .750 .750]
    )


Object Definitions
-------------------
The current version of the raytracer implements four different fundamental 4D objects:

  - 4-spheres,
  - 4-tetrahedrons,
  - 4-parallelepipeds, and
  - 4-planes.

All objects are defined with object directives that specify the geometrical paramters and give the
attributes that specify how the object is to be rendered.

### 4-Spheres
4-spheres are similar to their 3D counterparts, and were chosen because they were extremely easy to
implement. They also yield a good amount of insight into several aspects of 4-space raytracing.

The 4-sphere is defined with the "sphere" directive:

    Sphere
    (   --attributes--           > Explained later.
        center  {0, 0, 0, 0}     > Centered at the origin.
        radius  1.0              > Radius 1.0
    )

### 4-Tetrahedrons
4-tetrahedrons are basically like 3-tetrahedrons except that the vertices reside in 4-space. A
useful analogy is to consider a triangle, which is fundamentally a 2-space object, that is allowed
to reside in 3-space.

Like the triangle, the 4-tetrahedron is useful in 4D for "hypertiling" of 4D surfaces. For example,
six 4-tetrahedrons can be used to tile each of the eight "hyperfaces" of a 4-cube (analogously, two
triangles can be used to tile each of the six faces of a 3-cube).

The geometrical description of a 4-tetrahedron consists of the four vertices. Note that these
vertices must not be co-planar. The order of the vertices given is unimportant, since the
tetrahedron is essentially the solid bounded by the convex hull of the vertices.

    Tetrahedron
    (   --attributes--           > Explained later.
        Vertices                 > Four sets of four coordinates.
            { 0 0 0 0 }
            { 1 0 0 0 }
            { 1 1 0 0 }
            { 1 1 1 0 }
    )

Note that the intersection routine finds the barycentric coordinate of the ray-tetrahedron
intersection point, so future additions of Gouraud and Phong shading would be relatively easy to
implement.

### 4-Parallelepipeds
The 4-parallelepiped is an easy addition to the 4D raytracer since the only difference in the
intersection routines between the 4-tetrahedron and the 4-parallelepiped lies in the range of the
barycentric coordinates of the ray-object intersection point.

The geometric difference between the 4-parallelepiped and the 4-tetrahedron is the fact that the
4-parallelepiped is the solid bounded by three pairs of parallel faces. You can think of the
difference between a triangle and a parallelogram, each with vertices V0, V1 and V2. With the
triangle, the boundary is formed by the segments V0V1, V1V2 and V2V0. The parallelogram, however,
has an additional vertex whose position is constrained by the other three vertices and the
restriction that opposite segments are parallel.

Unlike the case for the 4-tetrahedron, the vertex order is important for the 4-parallelepiped (think
of triangles vs. parallelograms). The sides of the parallelogram are defined by the planes
containing V0V1V2, V0V1V3 & V0V2V3, and their parallel counterparts.

The object definition for a 4-parallelepiped looks identical to the one for the 4-tetrahedron:

    Parallelpiped
    (   --attributes--           > Explained later.
        Vertices                 > Four sets of four coordinates.
            0, 0, 0, 0
            1, 0, 0, 0
            0, 1, 0, 0
            0, 0, 1, 0
    )

Note that the above description defines a solid unit cube in 3-space that has one vertex at the
origin.

### 4-Planes
The infinite hyperplane is defined by a point on the plane and the plane normal vector:

    Plane
    (   --attributes--           > Explained later.
        point  { 10 -1 -7 37 }
        normal {  1  1  1  1 }
    )


Object Attributes
------------------
In this raytracer, the object rendering attributes are stored separately from their geometrical
descriptions. This method has the advantage that large groups of commonly rendered objects need not
duplicate storage to describe the same rendering attributes.

There are two ways of creating and invoking an attribute definition. One method uses a named
attribute which is defined globally and then invoked by name inside object definitions. The other
method (immediate definition) creates an unnamed attribute definition inside an object definition.

Here's an example of a named attribute definition and invocation:

    Attributes metallic_green
    (   Ambient          [ .010 .010 .010 ]
        Diffuse          [ .030 .170 .060 ]
        Specular         [ .080 .750 .110 ]
        Transparent      [ .000 .000 .000 ]
        Shine            10.0
        IndexRefraction  1.0
        Reflect          1
    )

    Sphere
    (   attributes  metallic_green   > Explained later.
        center      {0, 0, 0, 0}     > Centered at the origin.
        radius      1.0              > Radius 1.0
    )

Note that attribute names may not exceed 80 characters in length and will only be recognized to the
first 20 letters.

The following is an example of immediate attributes definition:

    Plane
    (   attributes
        (   ambient  [.030 .020 .200]
            diffuse  [.113 .080 .723]
            specular [.800 .800 .800]
            transpar [.100 .100 .100]
            shine    25.0
            IndexRef 1.50
            reflect  1
        )
        point      { 10 -1 -7 37 }
        normal     {  1  1  1  1 }
    )


Inheritance
------------
For both object and attribute definitions, any subfield may be omitted from the definition
directive. Fields which are omitted are assigned default values based on the previous definition of
the same type of directive.

All objects inherit default values from the previous definitions of like objects. For example, if
you want to define a bunch of scattered 4-spheres of the same radius, you need to give the radius
for the first 4-sphere, but may omit the radius subfield from subsequent definitions (until you want
to specify a 4-sphere of a different radius). Initial default values are undefined.

Like the case for object definitions, default values for attributes are inherited from previous
attribute definitions.

So basically, the following describe the inheritence rules:

  1.  The default values for object subfields are the values from the definition of the previous
      object of the same type. These default values are undefined for the first definition of a
      particular object.

  2.  The default values for attributes subfields are the values from the definition of the previous
      attributes. These default values are undefined for the first definition of attributes.

  3.  The default attributes bundle for a particular object is the attributes bundle invoked for the
      previous object of the same type. The first instance of a particular object MUST invoke, by
      name or by immediate definition, a particular collection of attributes.

For example:

    attributes A1 ( ... )             > Named attributes A1.
    attributes A2 ( ... )             > Named attributes A2.
    attributes A3 ( )                 > Inherits all attribute
                                      > values from A2.
    sphere ( attributes A2 ... )      > Sphere S1
    plane  ( attributes A1 ... )      > Plane P1
    plane  ( )                        > Inherits attributes A1 and
                                      > geometry from P1.
    sphere ( center ... radius ... )  > Inherits attributes A2
                                      > from S1 definition.
    plane  ( attributes ( ... ) )     > Inherits geometry from P2,
                                      > attribute values from A3.
    attributes A4 ( )      > Inherits values from the attributes
                           > definition (immediate mode) in the
                           > previous plane definition.


Lighting Model
---------------
Ray4 uses the following lighting model:

$$
I = I_aK_a + \sum_{L=2}^{N_L} I[L][K_d \cos(\theta) + K_s \cos^n(\alpha)] + K_sI_r + K_tI_t
$$

where

  - **$I_a$   [RGB]** -- Global ambient light
  - **$I[L]$ [RGB]** -- Light contributed by light L
  - **$I_r$   [RGB]** -- Light contributed by reflection
  - **$I_t$   [RGB]** -- Light contributed by transmission/refraction
  - **$K_a$   [RGB]** -- Object ambient color
  - **$k_d$   [RGB]** -- Object diffuse color
  - **$k_s$   [RGB]** -- Object specular color
  - **$k_t$   [RGB]** -- Object transparent color
  - **$n$    [Real]** -- Phong specular factor
  - **$N_L$   [Integer]** -- Number of light sources

Unlike other illuminations, ray4 uses RGB triples for all illumination intensity components. This
allows for a wider degree of control over object color, from plastic material to metallic material,
and also allows for greater ambient lighting control.

The current version of ray4 does not implement distance attenuation of light.


Display Device Parameters
--------------------------
As you may have noticed, the scene description file does not specify in any way the size, resolution
or other aspects of the final image. This allows for a more generic scene description so that image
files can be exchanged without modification for display on other systems.

The parameters of the target image are specified on the command line of the ray4 invocation. There
are six command-line arguments:

  * `-i Input_Scene_Description_Filename`
    <br>The name of the file that contains the ray4 scene description.

  * `-o Output_Image_Cube_Filename`
    <br>The name of the output image cube data. I suggest the ".icube" ending to the filename. The
    format of this output file is described later.

  * `-b Number_of_Bits_Per_Voxel`
    <br>This can be 12 or 24, independent of platform. This option is useful for Amiga displays,
    since the stock Amiga can only handle 12 bits of color. It's also useful for other platforms if
    you have limited space (consider that a 128x128x128 image takes over 6 megabytes of storage for
    24-bit voxels).

  * `-a Image_Cube_Aspect_Ratio`
    <br>This parameter gives the size (in arbitrary units) of an image voxel. The syntax is
    "XX:YY:ZZ". For example, if an image voxel is square in the X and Y directions and five times
    as long in the Z direction, you'd give "-a 1:1:5" for the aspect ratio.

  * `-r Image_Cube_Resolution`
    <br>The resolution of the image cube in the X, Y and Z directions. The syntax is the same as
    that for the aspect ratio, e.g. "-r 1024:768:100".

  * `-s Image_Cube_Scan_Range`
    <br>Given a particular resolution of the image, you may wish to view only certain portions of
    the image cube. To do this, you can use the -s option to limit the range of the raytracing. The
    scan range has the syntax of "xx-XX:yy-YY:zz-ZZ". Alternatively, any subfield can be given as
    the single underscore character ("_") to denote that the entire range for that direction is to
    be traced. So if, for example, the entire image cube has the size 1024:768:100, you could trace
    only the middle Z scanplane by giving "-s _:_:50" as the scan range. Note that each coordinate
    is zero-based, so the 1024:768:100 image cube has a full scan range of 0-1023:0-767:0-99.

Option parameters may immediately follow the option letter or may be separated on the command line
by whitespace.

One difficulty that you should be aware of when specifying the aspect ratios and resolutions is that
these two alter the size of the image cube. So if, for example, you increase the resolution in the Z
direction, the 4D space traced out by the image cube region is also increased. If you decrease the
aspect ratio in the Z direction, then you must also increase the aspect ratios in the X and Z
directions.

As an example, suppose that the image cube is sized by an aspect ratio of 1:1:1 and a resolution of
100:100:100. If you wanted a quick preview of this scene by taking only 4 slices in the Z direction
with a resolution of 100:100:4, then in order to cover the same space in the X and Y directions
you'd need to give an aspect ratio of 1:1:25. In other words, the ``coarse'' voxels would be 25
times longer in the Z direction than usual. If instead you wanted to take an even finer resolution
in the Z direction with a resolution of 100:100:1000, then you'd need to specify an aspect ratio of
10:10:1 for thinner voxels (in the Z direction).


Output File Format
-------------------
The output file is in two parts: an information header and the image cube data. The information
header has the following C structure definition:

    typedef struct S_IMAGEHDR   ImageHdr;

    struct S_IMAGEHDR
    {  unsigned long   magic;        /* Magic Number */
       unsigned char   version;      /* Image File Version Number */
       unsigned char   bitsperpixel; /* Number of Bits per Pixel */
       unsigned short  aspect[3];    /* Aspect Ratios [X,Y,Z] */
       unsigned short  first[3];     /* Start Voxels for [X,Y,Z] */
       unsigned short  last[3];      /* End   Voxels for [X,Y,Z] */
    };

This structure assumes that `short's are sixteen bits in size. The fields are as follows:

  - `magic` -- Always set to 'Ray4' (0x52977934).
  - `version` -- Lower is older. Currently set to 1.
  - `bitsperpixel` -- Total bits for each pixel - red, green & blue.
  - `aspect` -- Aspect ratio.
  - `first` -- Starting voxel of the image cube (X,Y,Z).
  - `last` -- Ending voxel of the image cube (X,Y,Z). The dimension of the data is
    (last[0]-first[0]+1) horizontally, (last[1]-first[1]+1) vertically, and (last[2]-first[2] deep.

The header structure size will be a multiple of longwords. After the header follows the image data.
Each scanline consists of RGB triples, where each triple is `bitsperpixel` long. Scanlines are
stored left to right, top to bottom, "front" to "back". All scanlines begin on even byte boundaries.


Image Cube Display
-------------------
In order to display the resulting image cube, you can use either the `r4toiff` or `r4tosgi` tools.
The `r4toiff` program takes the image and displays it on an Amiga microcomputer, and has the option
of storing the image as an Interchange File Format image file. The `r4tosgi' tool displays the image
cube on a platform that supports the Silicon Graphics GL (Graphics Language) library.

For information on the command-line options for these tools, enter the toolname without arguments
for a help listing.



[thesis]: https://hollasch.github.io/ray4/Four-Space_Visualization_of_4D_Objects.html
