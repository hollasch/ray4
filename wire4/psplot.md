psplot
====================================================================================================

NAME
-----
**psplot** - Simple PostScript plotting tool.


SYNOPSIS
---------
    psplot [-b] [-x <Width>] [-y <Height>]


DESCRIPTION
------------
`psplot` reads a simple plotting file from the input stream and writes the output PostScript file to
the output stream.

The following command-line options are supported:

    -b   :  Draw the boundary box.
    -x N :  Set the drawing region width  to N points (72 points / inch).
    -y N :  Set the drawing region height to N points (72 points / inch).

The output image will be plotted in a centered box on an 8.5in × 11in paper.


INPUT FILE FORMAT
------------------
The input file format is fairly simple. Command lines begin with a command character and are
followed by the command arguments. Input lines that do not begin with a command character are
ignored (lines that begin with the character '#' or whitespace characters are guaranteed to be
ignored).

The following commands are supported:

  | Command | Syntax                                                      |
  |---------|-------------------------------------------------------------|
  | window  | `W <Xmin:points> <Ymin:points> <Xmax:points> <Ymax:points>` |
  | move    | `M <X:real> <Y:real>`                                       |
  | draw    | `D <X:real> <Y:real>`                                       |

The window command sets the current drawing window on the 8-1/2"x11" paper. The arguments are in
points (72 points / inch), and range from 1-612 and 1-792 points for the X and Y coordinates,
respectively.

The move and draw commands operate within the current drawing window and take real coordinates in
the range [0,1]x[0,1]. The move command sets the current position without drawing, and the draw
command draws a line from the current position to the new position.

Additionally, lines that begin with the character '!' are copied literally to the output PostScript
file.


EXAMPLES
---------
The following command invocation plots the image in a centered box 4" wide by 5" tall:

    psplot -x4 -y 5 <plotfile >plotfile.ps

The following plot file draws an X and a + across the entire drawing region:

    ----BOF
      This line is a comment.
    # So is this line.
    M 1 0
    D 0 1
    M 0 0
    D 1 1
    M 0 .5
    D 1 .5
    M .5 0
    D .5 1
    ----EOF
