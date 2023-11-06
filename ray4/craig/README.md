Mark Craig Contributions
====================================================================================================

NOTE: To convert ray4 image files to SUN rasterfile, you need the PBM graphics package - See below
for info.

`run` and `run2` are sample scripts that can be used to run ray4 in the background. For example
`run dots` will raytrace the dots.r4 input file in the background redirecting all screen output to
the file `temp` and saving the ray4 image file as `dots.icube`.

To convert ray4 image data files to MTV format, use `r4tomtv`. r4tomtv uses the same command line
arguments as r4tosgi. It will create MTV files whose name is of the form:

    slice_(data file prefix)_(zplane)

For example: `slice_dots_0` would be the 0 zplane slice of the `dots.icube` file.

To convert an individual MTV file to SUN rasterfile, use `mtvtorast`. For example: `mtvtorast
slice_dots_0` will create the SUN rasterfile named `slice_dots_0.ras`. `screenload slice_dots_0.ras`
will display the image on a SUN.

`convert` will convert all slice MTV files to SUN rasterfile automatically. It also will delete all
MTV slice files if the user responds with "y" to the delete files prompt.

To display an individual MTV file on a SUN screen without saving to a SUN rasterfile, use `display`.
For example: `display slice_dots_0` will convert and display the slice.

Additional information (I don't know if the email addresses are still valid):

**MTV** --
  MTV RAYTRACER written by Mark Terrence VandeWettering (markv@cs.uoregon.edu)

**PBM** --
  Extended Portable Bitmap Toolkit written by Jef Poskanzer (pokey@well.sf.ca.us)
  `{ucbvax, lll-crg, sun!pacbell, apple, hplabs}!well!pokey`

Both available at many anonymous FTP sites.

Mark R. Craig
<br>Florida Institute of Technology
<br>mark@zach.fit.edu
