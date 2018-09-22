Four-Space Visualization of 4D Objects
========================================

![4D Hypercube](images/fig48-small.jpg)
![4D Octahedron](images/fig411-small.jpg)
![Hypersphere Slices](images/fig54a-small.jpg)
![Hyperspheres](images/fig54b-small.jpg)

This repository contains my thesis (converted to HTML from the original nroff sources), as well as
the original code for my 4D wireframe viewer and my 4D raytracer.

The thesis is now a single all-inclusive HTML file: [_Four-Space Visualization of 4D Objects_][].

[`wire4`](wire4/) is my 4D wireframe viewer. It was written to run on an SGI Iris workstation. It
reads in arbitrary 4D wireframes from a simple text file and displays them interactively. The code
has not been maintained, though, and will not run on most modern computers.

[`ray4`](ray4/) is the 4D raytracer. It is written in standard C, and reads from and writes to
standard output, so should be able to run on most computers. The output is a 3D RGB image that can
be displayed with a variety of methods.

You can send any questions to me at [steve@hollasch.net](mailto:steve@hollasch.net).


Other 4D Work
-------------
I'm contacted from time to time by others who have referenced my thesis or are embarking on 4D
visualization work of their own. Here are links to other work I've seen.

- [4D Snake](https://www.youtube.com/watch?v=8IUnqm8j4BE)
- [4+ Dimensional Solid Wireframes](http://utopian-aunt.surge.sh/)



[_Four-Space Visualization of 4D Objects_]: https://hollasch.github.io/ray4/Four-Space_Visualization_of_4D_Objects.html
