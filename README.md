Four-Space Visualization of 4D Objects
========================================

![4D Hypercube](images/fig48-small.jpg)
![4D Octahedron](images/fig411-small.jpg)
![Hypersphere Slices](images/fig54a-small.jpg)
![Hyperspheres](images/fig54b-small.jpg)

This repository contains my thesis (converted to HTML from the original nroff sources), as well as
the original code for my 4D wireframe viewer and my 4D raytracer.

[Four-Space Visualization of 4D Objects](http://hollasch.github.io/ray4/) is now a single
all-inclusive HTML file.

[wire4/](wire4) is my 4D wireframe viewer. It was written to run on an SGI Iris workstation. It
reads in arbitrary 4D wireframes from a simple text file and displays them interactively. The code
has not been maintained, though, and will not run on most modern computers.

[ray4/](ray4) is the 4D raytracer. It is written in standard C, and reads from and writes to
standard output, so should be able to run on most computers. The output is a 3D RGB image that can
be displayed with a variety of methods.

You can send any questions to me at [steve@hollasch.net](mailto:steve@hollasch.net).
