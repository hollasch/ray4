ray4
====================================================================================================

This is `ray4`, a 4D raytracer I wrote for my master’s thesis in May 1991
([_Four-Space Visualization of 4D Objects_][thesis]).

Ray4 User Manual
-----------------
See the [Ray4 User Manual][] for usage information.


Project Structure
------------------
This project includes the following directories:

  | Directory   |                                                  |
  |-------------|--------------------------------------------------|
  | [craig/][]  | Mark Craig contributions                         |
  | inputs.r4/  | Sample input ray4 files                          |
  | [ray4-c/][] | Original C raytracer distributed with the thesis |
  | src/        | Source code for ray4, image4 and unit tests      |


Building
---------
This project now builds with CMake, which should allow the code to work on all CMake-supported
platforms. Build from the `/ray4/ray4/` directory.


Testing
--------
This project includes unit tests created with the Catch2 unit-testing library. These tests can be
executed by running `build/Debug/tests.exe` or `build/Release/tests.exe`.


Example Run
-----------

    ray4 -b12 -idots.r4 -odots.icube -a10:11:20 -r78:65:12
    r4toiff dots.icube -t


Image4
-------
This project also includes [`image4`][], a tool to work with the 3D image cubes generated by `ray4`.


Beta Testers
------------
Thanks to the following beta testers for helping me with this code:

  - Greg Aharonian
  - Brian Carney
  - Mark Craig
  - Mark Ferneau
  - Marty Ryan

----
Steve Hollasch / steve@hollasch.net



[craig/]:           craig/README.md
[`image4`]:         image4.md
[Ray4 User Manual]: ray4.md
[ray4-c/]:          ray4-c/README.md
[thesis]:           https://hollasch.github.io/ray4/Four-Space_Visualization_of_4D_Objects.html
