//**************************************************************************************************
//  Copyright (c) 1991-2024 Steven R Hollasch
//
//  MIT License
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
//  and associated documentation files (the "Software"), to deal in the Software without
//  restriction, including without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies or
//  substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
//  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//**************************************************************************************************

/****************************************************************************
**
**  File:  psplot.c
**
**      This program implements a simple plotting tool that produces
**      PostScript output files.
**
**  Revisions:
**
**  1.00  23-Apr-1991  Steve Hollasch
**        Original version.
**
****************************************************************************/

#include <stdio.h>

/*-------------------------------------------------------------------------*/
static char usage[] = "\
psplot:  Postscript Plotting Tool\n\
usage :  psplot [-b] [-x <width>] [-y <height>]\n\
\n\
psplot reads a plot file from the input stream and writes the PostScript\n\
result to the output stream.  The command-line options are as follows:\n\
    -b  :  Draw the boundary box.\n\
    -x N:  Set the width  to N points (72 points / inch).\n\
    -y N:  Set the height to N points (72 points / inch).\n\
\n\
The input format is fairly simple.  Command lines begin with a command\n\
character and are followed by the command arguments:\n\
    Window:  W <Xmin:points> <Ymin:points> <Xmax:points> <Ymax:points>\n\
    Move  :  M <X:real> <Y:real>\n\
    Draw  :  D <X:real> <Y:real>\n\
\n\
The window command sets the current drawing window.  The X and Y coordinates\n\
must be in the ranges [1,612] and [1,792], respectively.  The move and draw\n\
commands take coordinates in the range [0,1]x[0,1] and operate within the\n\
current drawing window.  Any line that does not begin with a command letter\n\
is ignored.  Lines that begin with the character '!' are copied literally to\n\
the output PostScript file.\n\
\n";
/*-------------------------------------------------------------------------*/


   /***  Function Declarations  ***/

void  ProcessArgs  ();


   /***  Global Variables  ***/

short  cornerx =  90;	/* Left  Margin of Drawing Region (in Points) */
short  cornery = 180;	/* Lower Margin of Drawing Region (in Points) */
short  drawbox = 0;	/* If nonzero, draw the boundary box. */
FILE  *input;		/* Input Stream */
long   lcount;		/* Input Line Counter */
short  lengthx = 432;	/* Width  of Drawing Region (in Points) */
short  lengthy = 432;	/* Height of Drawing Region (in Points) */


/****************************************************************************
****************************************************************************/

int  main  (argc, argv)
   int   argc;
   char *argv[];
{
   auto char  buff[1024];

   ProcessArgs (argc, argv);

   printf ("newpath\n");

   if (drawbox)
   {  printf ("%4d %4d moveto\n", cornerx,           cornery);
      printf ("%4d %4d lineto\n", cornerx,           cornery+lengthy-1);
      printf ("%4d %4d lineto\n", cornerx+lengthx-1, cornery+lengthy-1);
      printf ("%4d %4d lineto\n", cornerx+lengthx-1, cornery);
      printf ("%4d %4d lineto\n", cornerx,           cornery);
   }

   for (lcount=1;  fgets (buff, sizeof buff, input);  ++lcount)
   {
      auto int    intx1,intx2, inty1,inty2;	/* Scanned Integer Values */
      auto float  realx, realy;			/* Scanned Real Values */

      switch (buff[0])
      {
         /* Reserved Comment Characters */

	 case '\n':
	 case '\t':
	 case ' ':
         case '#':
	    break;

	 /* Literal Inclusion */

	 case '!':
            fputs (buff+1, stdout);
	    break;

	 /* Move to Specified Location */

	 case 'M':
	 {
	    sscanf (buff+1, "%f %f", &realx, &realy);

	    if ((realx < 0.0) || (1.0 < realx))
	       fprintf (stderr,
		  "psplot:  [line %ld]  X value (%g) out of [0,1] range.\n",
		  lcount, realx
	       );
	    else if ((realy < 0.0) || (1.0 < realy))
	       fprintf (stderr,
		  "psplot:  [line %ld]  Y value (%g) out of [0,1] range.\n",
		  lcount, realy
	       );
	    else
	       printf
	       (  "%4d %4d moveto\n",
	          cornerx + (short)(realx * (float)(lengthx-1)),
	          cornery + (short)(realy * (float)(lengthy-1))
	       );
	    break;
	 }

	 /* Draw to Specified Location */

	 case 'D':
	 {
	    sscanf (buff+1, "%f %f", &realx, &realy);

	    if ((realx < 0.0) || (1.0 < realx))
	       fprintf (stderr,
		  "psplot:  [line %ld]  X value (%g) out of [0,1] range.\n",
		  lcount, realx
	       );
	    else if ((realy < 0.0) || (1.0 < realy))
	       fprintf (stderr,
		  "psplot:  [line %ld]  Y value (%g) out of [0,1] range.\n",
		  lcount, realy
	       );
	    else
	       printf
	       (  "%4d %4d lineto\n",
	          cornerx + (short)(realx * (float)(lengthx-1)),
	          cornery + (short)(realy * (float)(lengthy-1))
	       );
	    break;
	 }

	 /* Set the current drawing window. */

	 case 'W':
	 {
	    sscanf (buff+1, "%d %d %d %d", &intx1, &inty1, &intx2, &inty2);

	    if ((intx1 < 1) || (612 < intx1))
	       fprintf (stderr,
		  "psplot:  [line %d]  Window %s (%d) out of [1,612] range.\n",
		  lcount, "Xmin", intx1);
	    else if ((intx2 < 1) || (612 < intx2))
	       fprintf (stderr,
		  "psplot:  [line %d]  Window %s (%d) out of [1,612] range.\n",
		  lcount, "Xmax", intx2);
	    else if ((inty1 < 1) || (792 < inty1))
	       fprintf (stderr,
		  "psplot:  [line %d]  Window %s (%d) out of [1,792] range.\n",
		  lcount, "Ymin", inty1);
	    else if ((inty2 < 1) || (792 < inty2))
	       fprintf (stderr,
		  "psplot:  [line %d]  Window %s (%d) out of [1,792] range.\n",
		  lcount, "Ymax", inty2);
	    else if (intx1 > intx2)
	       fprintf (stderr,
		  "psplot:  [line %d]  Window %s (%d) greater than %s (%d).\n",
		  lcount, "Xmin", intx1, "Xmax", intx2);
	    else if (inty1 > inty2)
	       fprintf (stderr,
		  "psplot:  [line %d]  Window %s (%d) greater than %s (%d).\n",
		  lcount, "Ymin", inty1, "Ymax", inty2);
	    else
	    {  cornerx = intx1;
	       cornery = inty1;
	       lengthx = intx2 - intx1 + 1;
	       lengthy = inty2 - inty1 + 1;
	    }
	    break;
	 }

	 /* Ignore unrecognized lines. */

	 default:
	    break;
      }
   }

   printf ("stroke\nshowpage\n");

   if (input != stdin)
      fclose (input);
}



/****************************************************************************
//  This routine processes the command-line arguments and sets the global
//  variables accordingly.
****************************************************************************/

void  ProcessArgs  (argc, argv)
   int   argc;
   char *argv[];
{
   register int   argi;		/* Command-Line Argument Index */
   auto     char *fname = 0;	/* Input File Name */

   for (argi=1;  argi < argc;  ++argi)
   {
      if (*argv[argi] != '-')
      {  fname = argv[argi];
	 continue;
      }

      switch (argv[argi][1])
      {
         case 'b':
	    drawbox = 1;
	    break;

	 case 'x':
	 {
	    if (argv[argi][2])
	       lengthx = atoi (argv[argi]+2);
	    else if (++argi >= argc)
	    {  fprintf (stderr, "psplot:  Missing argument to -x option.\n");
	       exit (1);
	    }
	    else
	       lengthx = atoi (argv[argi]);
	    break;
	 }

	 case 'y':
	 {
	    if (argv[argi][2])
	       lengthy = atoi (argv[argi]+2);
	    else if (++argi >= argc)
	    {  fprintf (stderr, "psplot:  Missing argument to -y option.\n");
	       exit (1);
	    }
	    else
	       lengthy = atoi (argv[argi]);
	    break;
	 }

	 default:
	 {  fprintf
	    (  stderr, "psplot:  Unrecognized command-line option (%c).\n\n",
	       argv[argi][1]
	    );
	    fprintf (stderr, usage);
	    exit (1);
	 }
      }
   }

   cornerx = (612 - lengthx) / 2;
   cornery = (828 - lengthy) / 2;

   if (!fname)
      input = stdin;
   else if (!(input = fopen(fname,"r")))
   {  fprintf (stderr, "psplot:  Couldn't open input file (%s).\n", fname);
      exit (1);
   }
}
