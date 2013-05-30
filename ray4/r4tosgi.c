/***************************************************************************
//
//  File: r4tosgi.c
//
//    This program reads an output file from the Ray4 raytracer and displays
//  it on a Silicon Graphics Iris workstation.
//
//  Assumptions:
//    Images are no larger than 32767 x 32767 in size.
//
//  Revisions:
//
//    0.01  04-Mar-1991  Hollasch
//          Source code mods (r4image.h --> r4_image.h), no steve.h.
//
//    0.00  26-Nov-1990  Steve R. Hollasch
//          Preliminary version.
//
****************************************************************************/

#include <gl.h>
#include <device.h>
#include <stdio.h>

#include <unix.h>
#include "r4_image.h"


   /*********************/
   /***  Debug Flags  ***/
   /*********************/

#define DB_ARGS    1      /* Dump Command Options */
#define DB_MISC    1      /* Miscellaneous Output */
#define DB_HEADER  1      /* Dump Image Header Fields */


   /**********************************/
   /***  Usage & Version Messages  ***/
   /**********************************/

char notice[] = "\
\n\
r4tosgi  Version 0.00  26-Nov-1990  (C) 1990  Steven R. Hollasch\n\
\n";

char usage[] = "\
r4tosgi:  Display Ray4 output images on a Silicon Graphics Workstation.\n\
usage  :  r4tosgi [-t] [-z <Image Plane>] <Image Cube File>\n\
\n\
    -t:  Tiled display.  Display as many image slices as will fit in the\n\
         current screen, from left to right, top to bottom, starting at\n\
         the image plane given with the -z option (0 by default).\n\
\n\
    -z:  Image plane to display, or starting image plane with -t option.\n\
\n\
    After the image is displayed, click one of the mouse buttons to exit\n\
    the display.\n\
\n";


   /***************************/
   /***  Macro Definitions  ***/
   /***************************/

#define DISPWIDTH     (XMAXSCREEN+1)	/* Max Display Width */
#define DISPHEIGHT    (YMAXSCREEN+1)	/* Max Display Height */

#define true     1
#define false    0

#define nil      0

#ifdef _STDC_
#  define ARGS(list)    list		/* Function Argument List */
#else
#  define ARGS(list)    ()
#endif

#define print(string)   fputs(string,stdout)


   /**************************/
   /***  Type Definitions  ***/
   /**************************/

typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned long   ulong;


   /*******************************/
   /***  Function Declarations  ***/
   /*******************************/

void  main          ARGS((int, char**));
void  ParseArgs     ARGS((int, char**));
void  Halt          ARGS((char*, char*));
void  ImagePrep     ARGS((void));
void  InitSGI       ARGS((void));
void  DisplayImage  ARGS((ushort, short, short));


   /**************************/
   /***  Global Variables  ***/
   /**************************/

ImageHdr  image;		/* Image Record */
long      imgstart;		/* Image Data Start Position */
long      imgsize;		/* Image Slice Size */
Boolean   ginit_done = false;	/* True if Graphics Have Been Initialized */
short     indata;		/* Input Data Slot */
char     *infile;		/* Input Ray4 Image File */
FILE     *instream;		/* Image File Input Stream */
short     offx, offy;		/* Image Rectangle Offset Values */
RGBvalue *red, *blu, *grn;	/* RGB Scanline Buffers */
ushort    resx, resy;		/* Image Resoulution */
ulong     scanlsize;		/* Scanline Buffer Size */
uchar    *scanbuff;		/* Scanline Buffer */
Boolean   tile = false;		/* Tiling Option Switch */
ushort    zplane;		/* Image Plane */



/****************************************************************************
**  The main entry procedure.
****************************************************************************/

void  main  (argc, argv)
   int   argc;		/* Command Line Argument Count */
   char *argv[];	/* Command Line Argument Array */
{
   auto    short  horz_space;	/* Horizontal Spacing Between Tiled Images */
   auto    short  vert_space;	/* Vertical   Spacing Between Tiled Images */


   print (notice);

   ParseArgs (argc, argv);

   ImagePrep ();

   if (!tile)
   {  offx = (DISPWIDTH  - resx) / 2;
      offy = (DISPHEIGHT - resy) / 2;
   }
   else
   {  auto short ii;	/* Scratch Variable */

      ii = DISPWIDTH / resx;
      horz_space = (DISPWIDTH - (resx * ii)) / (ii + 1);
      ii = DISPHEIGHT / resy;
      vert_space = (DISPHEIGHT - (resy * ii)) / (ii + 1);

      offx = horz_space;
      offy = vert_space;
   }

#  if DB_MISC
      printf ("offx       %d\n", offx);
      printf ("offy       %d\n", offy);
      printf ("horz_space %d\n", horz_space);
      printf ("vert_space %d\n", vert_space);
      print  ("\n");
#  endif

   InitSGI ();

   do
   {  DisplayImage (zplane, offx, offy);

      if (!tile) break;

      ++zplane;

      offx += resx + horz_space;
      if ((offx + resx) > DISPWIDTH)
      {  offx = horz_space;
	 offy += resy + vert_space;
	 if ((offy + resy) > DISPHEIGHT)
	    break;
      }

   } while (zplane <= image.last[2]);

   /* Wait for an input event before exiting. */

   qread (&indata);

   Halt (nil, nil);
}



/****************************************************************************
**  This function is the exit point of the program.  It prints an optional
**  termination message and then frees program resources and halts the
**  program.
****************************************************************************/

void  Halt  (msg, arg)
   char *msg;	/* Termination Message */
   char *arg;	/* Optional Message String Argument */
{
   if (instream)  fclose (instream);

   if (scanbuff)  free (scanbuff);
   if (red)       free (red);
   if (grn)       free (grn);
   if (blu)       free (blu);

   if (ginit_done)
   {  greset ();
      color (BLACK);
      clear ();
      gexit ();
   }

   if (msg)
   {  print  ("r4tosgi:  ");
      printf (msg, arg);
      print  ("\n\n");
      exit (1);
   }

   exit (0);
}



/****************************************************************************
**  This procedure parses the command line arguments and switches.
****************************************************************************/

void  ParseArgs  (argc, argv)
   int   argc;		/* Command Line Argument Count */
   char *argv[];	/* Command Line Argument Array */
{
   register int argi;	/* Argument Array Index */

   if (argc < 2)
   {  print (usage);
      Halt (nil, nil);
   }

   for (argi=1;  argi < argc;  ++argi)
   {
      if (*argv[argi] != '-')
	 infile = argv[argi];
      else switch (argv[argi][1])
      {
	 case 't':
	    tile = true;
	    break;

	 case 'z':
	    if (argv[argi][2])
	    zplane = atoi (argv[argi]+2);
	    else
	    zplane = atoi (argv[++argi]);
	    break;

	 default:
	    Halt ("Invalid option (%s).", argv[argi]);
	    break;
      }
   }

#  if DB_ARGS
      printf ("infile %s\n", infile ? infile : "nil");
      printf ("tile   %s\n", tile ? "enabled" : "disabled");
      printf ("zplane %u\n", zplane);
      print  ("\n");
#  endif
}



/****************************************************************************
**  This function reads in the image header, allocates resources and sets
**  global variables to prepare for subsequent image display.
****************************************************************************/

void  ImagePrep  ()
{
   if (!infile)
      Halt ("No input file specified.", nil);

   /* Open the input file. */

   if (!(instream = fopen (infile, "r")))
      Halt ("Open failed on input file (%s).", infile);

   /* Read in the image cube header. */

   if (1 != fread (&image, sizeof(image), 1, instream))
      Halt ("Header read error on input file (%s).", infile);

#  if DB_HEADER
      print  ("Image Header Fields:\n");
      printf ("  magic       %08lx\n", image.magic);
      printf ("  version     %u\n", image.version);
      printf ("  bits/pixel  %u\n", image.bitsperpixel);
      printf ("  aspect      %u : %u : %u\n",
	 image.aspect[0], image.aspect[1], image.aspect[2]);
      printf ("  first       %u : %u : %u\n",
	 image.first[0], image.first[1], image.first[2]);
      printf ("  last        %u : %u : %u\n",
	 image.last[0], image.last[1], image.last[2]);
      print  ("\n");
#  endif

   resx = image.last[0] - image.first[0] + 1;
   resy = image.last[1] - image.first[1] + 1;

   /* Ensure that the magic number and format version are correct. */

   if (image.magic != 'Ray4')
      Halt ("Input file (%s) is not a Ray4 image file.", infile);

   if (image.version != 1)
   {  printf ("r4tosgi:  Unknown Ray4 format version (%d).\n",
	 image.version);
      Halt ("Aborting.", nil);
   }

   /* Ensure that the z-plane is in range of the file data. */

   if ((zplane < image.first[2]) || (zplane > image.last[2]))
   {  printf("r4tosgi:  Z plane (%u) is out of range [%u,%u].\n",
      zplane, image.first[2], image.last[2]);
      Halt ("Aborting.", nil);
   }

   /* Ensure that we can display the full image. */

   if (resx > DISPWIDTH)
      Halt ("Image too wide to display.", nil);
   if (resy > DISPHEIGHT)
      Halt ("Image too tall to display.", nil);

   /* Calculate the starting file position of the image plane. */

   scanlsize = (ulong)(resx) * 3L;

   if (image.bitsperpixel == 12)
   {  if (scanlsize & 1)  ++scanlsize;
      scanlsize >>= 1;
   }
   else if (image.bitsperpixel != 24)
   {  printf ("r4tosgi:  This image has %u bits per pixel.\n",
      image.bitsperpixel);
      Halt ("Can't handle this file format.", nil);
   }

   if (!(scanbuff = (uchar*) malloc ((unsigned)scanlsize)))
      Halt ("Out of memory.", nil);

   imgstart = sizeof (ImageHdr);
   imgsize  = (long)resy * scanlsize;

   red = (RGBvalue*) malloc ((unsigned)(sizeof(RGBvalue)*resx));
   grn = (RGBvalue*) malloc ((unsigned)(sizeof(RGBvalue)*resx));
   blu = (RGBvalue*) malloc ((unsigned)(sizeof(RGBvalue)*resx));

   if (!red || !grn || !blu)
      Halt ("Out of memory.", nil);

#  if DB_MISC
      printf ("resx      = %u\n", resx);
      printf ("resy      = %u\n", resy);
      printf ("scanlsize = %lu\n", scanlsize);
      print  ("\n");
#  endif
}



/****************************************************************************
**  This routine initializes the Silicon Graphics Iris workstation.
****************************************************************************/

void  InitSGI  ()
{
   ginit ();		/* Initialize the display. */
   ginit_done = true;

   cursoff ();

   RGBmode ();		/* Select RGB mode (24-bit color). */
   gconfig ();
   RGBcolor (0, 0, 0);
   clear ();

   qdevice (KEYBD);
}



/****************************************************************************
**  This procedure displays an image slice at the given x,y offset.
****************************************************************************/

void  DisplayImage  (plane, offx, offy)
   ushort  plane;	/* Image Plane to Display */
   short   offx;	/* Image Offset, Horizontal Component */
   short   offy;	/* Image Offset, Vertical Component */
{
   auto    Boolean   ebflag;		/* Even-Byte Boundary Flag */
   auto    uchar    *ptr;		/* Scanline Buffer Point */
   auto    RGBvalue *Vr, *Vg, *Vb;	/* Color Array Pointers */
   auto    short     xx, yy;		/* Pixel Position */


   fseek (instream, imgstart + (imgsize * (long)(plane-image.first[2])), 0);

   for (yy=image.first[1];  yy <= image.last[1];  ++yy)
   {
      /* Read in the next scanline. */

      if (1 != fread (scanbuff, scanlsize, 1, instream))
	 Halt ("Read error on image file (%s).", infile);

      ebflag = true;
      ptr    = scanbuff;

      Vr = red;
      Vg = grn;
      Vb = blu;

      for (xx=image.first[0];  xx <= image.last[0];  ++xx)
      {
	 if (image.bitsperpixel == 24)
	 {  *Vr++ = *ptr++;
	    *Vg++ = *ptr++;
	    *Vb++ = *ptr++;
	 }
	 else if (ebflag)
	 {  *Vr++ = *ptr & 0xF0;
	    *Vg++ = *ptr++ << 4;
	    *Vb++ = *ptr & 0xF0;
	    ebflag = false;
	 }
	 else
	 {  *Vr++ = *ptr++ << 4;
	    *Vg++ = *ptr & 0xF0;
	    *Vb++ = *ptr++ << 4;
	    ebflag = true;
	 }
      }

      /* Write the scanline. */

      cmov2i (offx, DISPHEIGHT-(offy+yy));
      writeRGB (resx, red, grn, blu);
   }
}
