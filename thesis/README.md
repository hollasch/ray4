Four-Space Visualization of 4D Objects
====================================================================================================

These files contain the HTML, PostScript and TROFF versions of my master's thesis in computer
science, _Four-Space Visualization of 4D Objects_. The latest HTML version is a single & complete
file: [Four-Space\_Visualization\_of\_4D\_Objects.html].

Abstract
--------
> In the field of scientific visualization, the term “four dimensional
> visualization” usually refers to the process of rendering a three dimensional
> field of scalar values. While this paradigm applies to many different data
> sets, there are also uses for visualizing data that correspond to actual
> four-dimensional structure. Four dimensional structures have typically been
> visualized via wireframe methods, but this process alone is usually insufficient
> for an intuitive understanding; all but the most simple datasets easily
> overwhelm the viewer. This research covers the visualization of four
> dimensional objects through wireframe methods with extended visualization cues,
> and through raytracing methods. Both the wireframe and raytracing methods
> employ true four-space viewing parameters and geometry. The raytracing approach
> easily solves the hidden surface and shadowing problems of 4D objects, and
> yields an image in the form of a three-dimensional field of RGB values, which
> can be rendered with a variety of existing methods. The 4D raytracer also
> supports true four-dimensional lighting, reflections and refractions.


Associated Programs
-------------------
I have also made my 4D wireframe viewer (`wire4`) and 4D raytracer (`ray4`) available in the public
domain, found in the parent directory.

The 4D wireframe viewer (`wire4`) was written to run on an SGI Iris workstation, and can read in
arbitrary 4D objects from a simple text file and display them interactively.

The 4D raytracer (`ray4`) should be able to run on any platform that supports C's floating point
library. The output is a 3D RGB image that can be displayed with a variety of methods.



[Four-Space\_Visualization\_of\_4D\_Objects.html]: http://hollasch.github.io/ray4/Four-Space_Visualization_of_4D_Objects.html
