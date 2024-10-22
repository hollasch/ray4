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

/***************************************************************************
**
**  File: r4tomtv.c
**
**    This program reads an output file from the Ray4 raytracer and outputs
**  mtv raytracer format files (1 file per slice).
**
**  Assumptions:
**    Images are no larger than 32767 x 32767 in size.
**
**  Revisions:
**
**    0.03  14-Nov-1991  Mark R. Craig (mark@zach.fit.edu)
**          Modified r4tosgi.c to output MTV format image files.
**
**    0.02  15-Jul-1991  Hollasch, Marty Ryan
**          Removed inclusion of unix.h.  Also keyed typedefs of uchar,
**          ushort and ulong off of _AIX.
**
**    0.01  04-Mar-1991  Hollasch
**          Source code mods (r4image.h --> r4_image.h), no steve.h.
**
**    0.00  26-Nov-1990  Steve R. Hollasch
**          Preliminary version.
**
****************************************************************************/

#include <stdio.h>

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
r4tomtv  Version 0.00  14-Nov-1991 (C) 1991 Steven R. Hollasch, Mark R. Craig\n\
\n";

char usage[] = "\
r4tomtv:  Convert Ray4 output images to MTV format image files.\n\
usage  :  r4tomtv [-t] [-z <Image Plane>] <Image Cube File>\n\
\n\
    -t:  Tiled display.  Process all image slices (output MTV image files)\n\
         starting at the plane given with the -z option (0 by default).\n\
\n\
    -z:  Image plane to process, or starting image plane with -t option.\n\
\n\
\n";


   /***************************/
   /***  Macro Definitions  ***/
   /***************************/

#define XMAXSCREEN 1151
#define YMAXSCREEN 899

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

#ifndef _AIX
   typedef unsigned char   uchar;
   typedef unsigned short  ushort;
   typedef unsigned long   ulong;
#endif


   /*******************************/
   /***  Function Declarations  ***/
   /*******************************/

void  main          ARGS((int, char**));
void  ParseArgs     ARGS((int, char**));
void  Halt          ARGS((char*, char*));
void  ImagePrep     ARGS((void));
void  DisplayImage  ARGS((ushort, short, short));


   /**************************/
   /***  Global Variables  ***/
   /**************************/

typedef uchar RGBvalue;     
typedef int Boolean;

ImageHdr  image;		/* Image Record */
long      imgstart;		/* Image Data Start Position */
long      imgsize;		/* Image Slice Size */
short     indata;		/* Input Data Slot */
char     *infile;		/* Input Ray4 Image File */
FILE     *instream;		/* Image File Input Stream */
RGBvalue *red, *blu, *grn; 	/* RGB Scanline Buffers */
ushort    resx, resy;		/* Image Resoulution */
ulong     scanlsize;		/* Scanline Buffer Size */
uchar    *scanbuff;		/* Scanline Buffer */
Boolean   tile = false;		/* Tiling Option Switch */
ushort    zplane;		/* Image Plane */

int num;
FILE *outstream;
char filename[180];
char outfile[80];

/****************************************************************************
**  The main entry procedure.
****************************************************************************/

void  main  (argc, argv)
   int   argc;		/* Command Line Argument Count */
   char *argv[];	/* Command Line Argument Array */
{

   print (notice);

   ParseArgs (argc, argv);

   ImagePrep ();

   while (zplane <= image.last[2])
   {  
      sprintf(filename,"slice_%s_%d",outfile,zplane);
      if ((outstream=fopen(filename,"w"))==NULL)
      {  Halt("Can't open %s\n",filename);
      }
      fprintf(outstream,"%d %d\n",resx,resy);

      printf("z:  %d\n",zplane);
      DisplayImage (zplane);
      num++;
      fclose(outstream);

      if (!tile) break;

      ++zplane;

   }

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
   if (red)  free (red);
   if (grn)  free (grn);
   if (blu)  free (blu);

   if (msg)
   {  print  ("r4tomtv:  ");
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
   int i;

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

   strcpy(outfile,infile);
   for (i=0;i<strlen(outfile);i++)
      if ((outfile[i]=='.') && (outfile[i+1]=='i'))
      {
	 outfile[i]='\0';
	 break;
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
   {  printf ("r4tomtv:  Unknown Ray4 format version (%d).\n",
	 image.version);
      Halt ("Aborting.", nil);
   }

   /* Ensure that the z-plane is in range of the file data. */

   if ((zplane < image.first[2]) || (zplane > image.last[2]))
   {  printf("r4tomtv:  Z plane (%u) is out of range [%u,%u].\n",
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
   {  printf ("r4tomtv:  This image has %u bits per pixel.\n",
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
**  This procedure outputs an image slice.
****************************************************************************/

void  DisplayImage  (plane)
   ushort  plane;	/* Image Plane to Display */
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
	 {  *Vr = *ptr++;
	    *Vg = *ptr++;
	    *Vb = *ptr++;
	 }
	 else if (ebflag)
	 {  *Vr = *ptr & 0xF0;
	    *Vg = *ptr++ << 4;
	    *Vb = *ptr & 0xF0;
	    ebflag = false;
	 }
	 else
	 {  *Vr = *ptr++ << 4;
	    *Vg = *ptr & 0xF0;
	    *Vb = *ptr++ << 4;
	    ebflag = true;
	 }
	 fputc(*Vr,outstream);
	 fputc(*Vg,outstream);
	 fputc(*Vb,outstream);
      }
   }
}
