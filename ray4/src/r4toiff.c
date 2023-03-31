/***********************************************************************
**
**  "ray4" is Copyright (c) 1991,1992,1993 by Steve R. Hollasch.
**
**  All rights reserved.  This software may be freely copied, modified
**  and redistributed, provided that this copyright notice is preserved
**  in all copies.  This software is provided "as is", without express
**  or implied warranty.  You may not include this software in a program
**  or other software product without also supplying the source, or
**  without informing the end-user that the source is available for no
**  extra charge.  If you modify this software, please include a notice
**  detailing the author, date and purpose of the modification.
**
***********************************************************************/

/***************************************************************************
**
**  File: r4toiff.c
**
**    This program converts the output file from the Ray4 raytracer to Amiga
**  IFF format.
**
**  Assumptions:
**    Images are no larger than 65535 x 65535 in size.
**
**  Revisions:
**
**    1.50  08-Feb-1991  Hollasch
**        Released to the public domain.
**
**    1.42  27-May-1991  Hollasch
**        If the zplane is either left unspecified or is specified at zero,
**        zplane will automatically be set to the first image plane.
**
**    1.41  04-Feb-1991  Hollasch
**        Fixed bug in image plane seeks.  The previous version assumed that
**        image data always began at image plane zero (image.first[2]==0).
**
**    1.40  02-Feb-1991  Hollasch
**        Added the -g option.
**
**    1.30  29-Nov-1990  Hollasch
**        Added the -p option.
**
**    1.20  14-Nov-1990  Hollasch
**        Added tiled display.
**
**    1.12  11-Nov-1990  Hollasch
**        Repaired a bug in GetColorPalette.  The result was that not all
**        pixels were read during the first pass, and the scanline buffer
**        was not used up completely.
**
**    1.11  01-Nov-1990  Hollasch
**        Program now saves full screen only; too many problems with show
**        programs that don't properly handle ILBM images smaller than
**        the full screen.  Also removed ILBM code to another file.  Display
**        window is now a BACKDROP window.
**
**    1.10  24-Oct-1990  Hollasch
**        Added IFF output and colormap load/save.
**
**    0.01  20-Oct-1990  Steve R. Hollasch
**        Preliminary version.
**
****************************************************************************/

#include <stdio.h>

#include <libraries/dosextens.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>

#ifndef  LATTICE
#   include <functions.h>
#endif

#include "amiga.h"
#include "r4_image.h"


   /************************/
   /***  Debug Switches  ***/
   /************************/

#define  DB_MISC  0     /* Print out miscellaneous debugging info. */
#define  DB_OPTS  0     /* Print Command Line Settings */
#define  DB_TILE  0     /* Tiling Debug Statements */


   /**********************************/
   /***  Usage & Version Messages  ***/
   /**********************************/

char notice[] = "\
\n\
r4toIFF  Version 1.50  08-Feb-1993  (C) 1991,1992,1993  Steve R. Hollasch\n\
\n";

char *usage = "\
r4toiff:  Display Ray4 output images and convert to IFF image file.\n\
Usage  :  r4toiff <Input File> [-p<Pixel Size>] [-t] [-g]\n\
                  [-m<Color_Map_File>] [+m<Color_Map_File>]\n\
                  [-o<Output IFF File>] [-z<Integer>]\n\
\n\
    -g  Use greyscale for HAM palette.\n\
    +m  Generate a color map file with the current image's palette.\n\
    -m  Use the named color map file for the HAM palette.\n\
    -o  The output IFF file.  If not given, no file is produced.\n\
    -p  Pixel size as a power of two.  Must be in [0,5], default is 0.\n\
    -t  Display image slices in tiled fashion (as many as fit).\n\
    -z  The Z plane of the image cube to display.\n\
\n\
    This program will generate a HAM-encoded 320x200 or 320x400 image,\n\
    depending on the input file's aspect ratio.\n\
\n\
    Note that the -m option will only read color map files produced with\n\
    the +m option; it cannot extract color maps from IFF files.\n\
\n\
";

   /****************************/
   /***  Program Parameters  ***/
   /****************************/

#define  NPLANES    6           /* Number of Bit Planes for HAM Image */
#define  NPALETTE   16          /* # Popular Colors */

#define  MAXHEIGHT  400         /* Maximum Image Height */
#define  MAXWIDTH   320         /* Maximum Image Width */


   /***************************/
   /***  Macro Definitions  ***/
   /***************************/

#define lsizeof(x)      (long)(sizeof(x))

                /* File Address of a Given Image Plane */

#define PLANE_START(plane) \
   (lsizeof(ImageHdr) + (((plane)-image.first[2]) * scanlsize * resy))


   /**************************/
   /***  Type Definitions  ***/
   /**************************/

typedef struct FileHandle    Fhandle;
typedef struct IntuiMessage  IntuiMessage;
typedef struct Library       Library;
typedef struct RastPort      RastPort;
typedef struct Screen        Screen;
typedef struct ViewPort      ViewPort;
typedef struct Window        Window;


   /*******************************/
   /***  Function Declarations  ***/
   /*******************************/

void    ProcessArgs     (/* int, char** */);
void    Halt            (/* char* */);
void    GetColorPalette (/* ulong */);
short   ReadNextRGB     (/* void */);
void    DisplayImage    (/* ulong, short, short */);
void    ImagePrep       (/* void */);
void    InitGraphics    (/* ushort, ushort */);
ulong   GetHAM          (/* ulong, short*, short */);
ulong   ColorDistance   (/* short, short, short, short */);


   /*************************/
   /***  Data Structures  ***/
   /*************************/

struct NewScreen ScreenDef =
{
   0, 0,                /* Left Edge, Top Edge */
   320, 400,            /* Width, Height */
   NPLANES,             /* Number of Bit Planes (6 for HAM) */
   0, 1,                /* Detail & Block Pen Colors */
   HAM,                 /* View Modes */
   CUSTOMSCREEN,        /* Screen Type */
   NULL,                /* Default Font */
   NULL,                /* Default Title */
   NULL, NULL           /* Gadget List and Custom Bit Map */
};

struct NewWindow WinDef =
{
   0, 0,                        /* Left Edge, Top Edge */
   320, 400,                    /* Width, Height */
   0, 1,                        /* Detail & Block Pen Colors */
   MOUSEBUTTONS | CLOSEWINDOW,  /* IDCMP Flags */
   ACTIVATE | WINDOWCLOSE       /* Window Flags */
        | BORDERLESS | BACKDROP,
   NULL, NULL,                  /* Gadget List, Checkmark Image */
   NULL,                        /* Window Title */
   NULL, NULL,                  /* Screen Pointer, BitMap Pointer */
   320, 400,                    /* Minimum Width & Height */
   320, 400,                    /* Maximum Width & Height */
   CUSTOMSCREEN                 /* Window Type */
};


   /*************************************/
   /***  Global Variable Definitions  ***/
   /*************************************/

ulong         class;            /* Inuition Message Class */
Library      *GfxBase;          /* Graphics Library Base Pointer */
ImageHdr      image;            /* Image Header */
char         *infile = nil;     /* Input File Name */
Fhandle      *instream;         /* Input File Descriptor */
Library      *IntuitionBase;    /* Intuition Library Base Pointer */
char         *mapin, *mapout;   /* Color Map File Names */
IntuiMessage *msg;              /* Inuition Message */
short         offx, offy;       /* X & Y Image Offset Values */
char         *outfile = nil;    /* Output File Name */
short         palette[NPALETTE];/* Color Palette Array */
short         pixfact;          /* Pixel Factor [0,5] */
short         pixsize;          /* Pixel Size (2 ^ pixfact) */
RastPort     *rastport;         /* Graphics Raster Port */
short         resx, resy;       /* X and Y Image Resolution */
short         sresx, sresy;     /* Screen Resolution (considers pixsize) */
ulong         scanlsize;        /* Scanline Buffer Size */
uchar        *scanbuff;         /* Scanline Buffer */
Screen       *screen;           /* Graphics Screen */
boolean       tile = false;     /* Tile Display of Image Slices */
ViewPort     *viewport;         /* Graphics Viewport */
Window       *win;              /* Graphics Window */
boolean       use_greyscale=0;  /* If true, Use Greyscale For HAM-Encoding */
ulong         zplane;           /* Z-plane of the Image Cube */



/***************************************************************************/

void  main  (argc, argv)
   int    argc;
   char **argv;
{
   auto   short  horz_space;    /* Horizontal Spacing in Tiled Displays */
   auto   short  vert_space;    /* Vertical   Spacing in Tiled Displays */

   print (notice);

   if (argc <= 1)
   {  print (usage);
      Halt (nil, nil);
   }

   ProcessArgs (argc, argv);

   ImagePrep ();


   /* Find the most popular colors of the first image slice. */

   GetColorPalette (zplane);


   /* Display the image slices. */

   InitGraphics ();

   if (tile)
   {  auto   long  maxhorz, maxvert;    /* Max Images Across, Down */
      auto   long  ntile;               /* Number of Tiled Images, Max */

      ntile = image.last[2] - zplane + 1;

      maxhorz = win->Width  / sresx;
      maxvert = win->Height / sresy;

      if (ntile < maxhorz)         /* Tiles for Single Horizontal Strip */
      {  horz_space = (win->Width - (sresx * ntile)) / (ntile + 1);
         vert_space = (win->Height - sresy) / 2;
      }
      else
      {  horz_space = (win->Width  - (sresx * maxhorz)) / (maxhorz + 1);
         vert_space = (win->Height - (sresy * maxvert)) / (maxvert + 1);

         if ((vert_space == 0) && (ntile <= (maxhorz * (maxvert-1))))
         {  --maxvert;
            vert_space = (win->Height - (sresy * maxvert)) / (maxvert + 1);
         }

         if ((horz_space == 0) && (ntile <= (maxhorz-1) * maxvert))
         {  --maxhorz;
            horz_space = (win->Width  - (sresx * maxhorz)) / (maxhorz + 1);
         }
      }

      offx = horz_space;
      offy = vert_space;

#     if DB_TILE
         printf ("win->Width %d, win->Height %d\n", win->Width, win->Height);
         printf ("horz_space %d, vert_space  %d\n", horz_space, vert_space);
         printf ("offx %d, offy %d\n", offx, offy);
#     endif
   }
   else
   {  offx = (win->Width  - sresx) / 2;
      offy = (win->Height - sresy) / 2;
   }

   do
   {  DisplayImage (zplane, offx, offy);   /* Display the current image */

      if (!tile)
         break;

      ++zplane;

      offx += sresx + horz_space;
      if ((offx + sresx) > win->Width)
      {  offx = horz_space;
         offy += sresy + vert_space;
         if ((offy + sresy) > win->Height)
            break;
      }

   } while (zplane <= image.last[2]);

   if (outfile)         /* Write the output IFF file if desired. */
   {
      WriteILBM (outfile, screen);
      DisplayBeep (0L);
   }
   else
   {
      DisplayBeep (0L);

      /* Wait for the user to click the closewindow gadget before halting. */

      do
      {  WaitPort (win->UserPort);
         msg = (IntuiMessage *) GetMsg (win->UserPort);
         class = msg->Class;
         ReplyMsg (msg);
      } while (class != CLOSEWINDOW);
   }

   Halt (nil, nil);
}



/****************************************************************************
**  Halt() prints out the error message (if there is one), cleans up, and
**  halts the program.
****************************************************************************/

void  Halt  (msg, arg)
   char *msg;   /* Error Message */
   char *arg;   /* Message Argument */
{
   if (msg)
   {  print  ("r4toiff:  ");
      printf (msg, arg);
      print  ("\n");
   }

   if (instream)      Close (instream);

   if (win)           CloseWindow (win);
   if (screen)        CloseScreen (screen);

   if (GfxBase)       CloseLibrary (GfxBase);
   if (IntuitionBase) CloseLibrary (IntuitionBase);

   if (scanbuff)      FreeMem (scanbuff, scanlsize);

   exit (msg) ? 1 : 0;
}



/****************************************************************************
**  This procedure process the command-line arguments.  It does not return
**  if it detects an error.
****************************************************************************/

void  ProcessArgs  (argc, argv)
   int    argc;
   char **argv;
{
   register char   *aptr;       /* Argument Pointer */
   register ushort  argi;       /* Command-Line Argument Index */
   auto     char   *swistr;     /* Command-Line Switch String */

   for (argi=1;  argi < argc;  ++argi)
   {
      if ((argv[argi][0] != '-') && (argv[argi][0] != '+'))
      {  infile = argv[argi];
         continue;
      }

      swistr = argv[argi];
      if (argv[argi][2])
         aptr = argv[argi] + 2;
      else
         aptr = argv[++argi];

      switch (swistr[1])
      {
         case 'g':
            use_greyscale = true;
            --argi;
            break;

         case 'm':
            if (swistr[0] == '+')
               mapout = aptr;
            else if (swistr[0] == '-')
               mapin  = aptr;
            break;

         case 'o':
            outfile  = aptr;
            break;

         case 'p':   
            pixfact = atoi (aptr);

            if ((pixfact < 0) || (5 < pixfact))
            {  printf ("Pixel size (%d) out of range [0,5].\n", pixfact);
               Halt ("Aborting.", nil);
            }
            break;

         case 't':
            tile = true;
            --argi;
            break;

         case 'z':
            zplane = atol (aptr);
            break;

         default :
            printf ("r4toiff:  Unknown option (%c).\n", swistr[1]);
            print  (usage);
            Halt ("Aborting.", nil);
            break;
      }
   }

   pixsize = 1 << pixfact;

#  if DB_OPTS
      print  ("\nCommand Line Settings:\n");
      printf ("infile  %s\n", infile  ? infile  : "nil");
      printf ("outfile %s\n", outfile ? outfile : "nil");
      printf ("mapout  %s\n", mapout  ? mapout  : "nil");
      printf ("mapin   %s\n", mapin   ? mapin   : "nil");
      printf ("pixfact %d\n", pixfact);
      printf ("pixsize %d\n", pixsize);
      printf ("tile    %s\n", tile ? "enabled" : "disabled");
      printf ("zplane  %lu\n",zplane);
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

   instream = Open (infile, MODE_OLDFILE);
   if (!instream)
      Halt ("Open failed on input file (%s).", infile);

   /* Read in the image cube header. */

   if (lsizeof(image) != Read (instream, &image, lsizeof(image)))
      Halt ("Read error on input file (%s).", infile);

#  if DB_MISC
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

   resx  = image.last[0] - image.first[0] + 1;
   resy  = image.last[1] - image.first[1] + 1;

   if (  (((long)resx << pixfact) & ~0xFFFF)
      || (((long)resy << pixfact) & ~0xFFFF)
      )
      Halt ("Pixel size / image resolution overflow.\n", nil);

   sresx = resx << pixfact;
   sresy = resy << pixfact;

   /* Ensure that the magic number and format version are correct. */

   if (image.magic != R4_IMAGE_ID)
      Halt ("Input file is not a Ray4 image file.", nil);

   if (image.version != 1)
   {  printf ("r4toiff:  Unknown Ray4 format version (%d).\n",
      image.version);
      Halt ("Aborting.", nil);
   }

   /* Ensure that the z-plane is in range of the file data. */

   if (!zplane && (zplane < image.first[2]))
      zplane = image.first[2];
   else if ((zplane < image.first[2]) || (image.last[2] < zplane))
   {  printf("r4toiff:  Z plane (%lu) is out of range (range is [%u,%u]).\n",
         zplane, image.first[2], image.last[2]);
      Halt ("Aborting.", nil);
   }

   /* Ensure that we can display the full image. */

   if (sresx > MAXWIDTH)
      Halt ("Image too wide to display.", nil);
   if (sresy > MAXHEIGHT)
      Halt ("Image too high to display.", nil);

   /* Calculate the starting file position of the image plane. */

   scanlsize = (ulong)(resx) * 3L;

   if (image.bitsperpixel == 12)
   {  if (scanlsize & 1)  ++scanlsize;
      scanlsize >>= 1;
   }
   else if (image.bitsperpixel != 24)
   {  printf ("r4toiff:  This image has %u bits per pixel.\n",
         image.bitsperpixel);
      Halt ("Can't handle this file format.", nil);
   }

   scanbuff  = AllocMem (scanlsize, 0L);

#   if DB_MISC
      print  ("\nImage Miscellaneous\n");
      printf ("resx  %u, resy  %u\n", resx, resy);
      printf ("sresx %u, sresy %u\n", sresx, sresy);
      printf ("scanlsize = %lu\n", scanlsize);
      print  ("\n");
#   endif
}



/****************************************************************************
**  This function reads in the image data and collects the color histogram.
**  The indices to the NPALETTE most popular colors are stored in the
**  palette[] array, with the darkest of those colors in palette[0] (the
**  overscan border color).
****************************************************************************/

void  GetColorPalette  (slice)
   ulong  slice;      /* Image Slice */
{
   auto     ulong   *chist;             /* Color Histogram */
   auto     ushort   dark_index;        /* Index of Darkest Popular Color */
   auto     ushort   dark_value;        /* Darkest Color Value */
   auto     Fhandle *mapfile;           /* Color Map File Handle */
   auto     ulong    Npixels;           /* Number of Pixels in Image Slice */
   register ulong    ii, jj, kk;        /* Loop Indices */

   /* If the user specified a colormap source file from the command line,
   ** then all we need to do is to load that color map and return. */

   if (mapin)
   {
      if (!(mapfile = Open (mapin, MODE_OLDFILE)))
         Halt ("Couldn't open source map file (%s).", mapin);

      if (lsizeof(palette) != Read (mapfile, &palette, lsizeof(palette)))
      {  Close (mapfile);
         Halt ("Read error from source map file (%s).", mapin);
      }

      /* In case we didn't read a genuine map file, normalize the colors. */

      for (ii=0; ii < NPALETTE; ++ii)  palette[ii] &= 0x0FFF;
      Close (mapfile);

      return;
   }

   /* If the user wants to use the greyscale as the base palette for the HAM
   ** encoding rather than the most popular colors, then just load up the
   ** palette with the NPALETTE greyscale values and return. */

   if (use_greyscale)
   {
      for (ii=0;  ii < NPALETTE;  ++ii)
         palette[ii] = ii * 0x111;

      return;
   }

   /* Seek to the start of the image data. */

   if (-1 == Seek (instream, PLANE_START(slice), OFFSET_BEGINNING))
      Halt ("Seek failed on image file (%s).", infile);

   chist = (ulong *) AllocMem (lsizeof(chist[0]) * 0x1000L, MEMF_CLEAR);

   /* Get the histogram of the color distribution. */

   Npixels = (ulong)(resx) * (ulong)(resy);

   for (ii=0;  ii < Npixels;  ++ii)
      ++ chist[ ReadNextRGB() ];

   /* Load the first NPALETTE colors into the palette array, in sorted
   ** order. */

   palette[0] = 0x000;

   for (ii=1;  ii < NPALETTE;  ++ii)
   {
      jj = ii;
      while ((jj > 0) && (chist[ii] > chist[palette[jj-1]]))
         --jj;

      for (kk=ii;  kk > jj;  --kk)
         palette[kk] = palette[kk-1];

      palette[jj] = ii;
   }

   /* Now insert the remaining colors in the popular color array in
   ** descending order, with the most popular color in palette[0].  */

   for (ii=NPALETTE;  ii <= 0xFFF;  ++ii)
   {
      if (chist[ii] <= chist[palette[NPALETTE-1]])
         continue;

      for (jj=0;  chist[ii] <= chist[palette[jj]];  ++jj)
         ;

      for (kk=(NPALETTE-1);  kk > jj;  --kk)
         palette[kk] = palette[kk-1];

      palette[jj] = ii;
   }

   /* Now that we have the most popular colors for the color table, select
   ** the darkest of these colors and put it in location 0.  Note that in
   ** the following loop, `jj' holds the current color, and `kk' holds the
   ** value of that color.  */

   dark_value = 299*15 + 587*15 + 114*15;

   for (ii=0;  ii < NPALETTE;  ++ii)
   {
      jj = palette[ii];
      kk = (299 * (jj>>8)) + (587 * ((jj>>4)&0xF)) + (114 * (jj&0xF));
      if (kk < dark_value)
      {  dark_index = ii;
         dark_value = kk;
      }
   }

   /* Swap the darkest color into the 0 (background) color. */

   ii = palette[0];
   palette[0] = palette[dark_index];
   palette[dark_index] = ii;

   FreeMem (chist, lsizeof(chist[0]) * 0x1000L);

   /* If the user specified an output color map file, then save the current
   ** color map to the specified file name.  */

   if (mapout)
   {
      if (!(mapfile = Open (mapout, MODE_NEWFILE)))
         Halt ("Couldn't open output map file (%s).", mapout);

      if (lsizeof(palette) != Write (mapfile, &palette, lsizeof(palette)))
      {  Close (mapfile);
         Halt ("Write error to output map file (%s).", mapout);
      }
      Close (mapfile);
   }
}



/****************************************************************************
**  This routine reads the next RGB triple from the input file.
****************************************************************************/

short  ReadNextRGB  ()
{
   static   boolean  ebflag  = 1;               /* Even-Byte Flag */
   static   uchar   *scanptr = nil;             /* Scanline Buffer Pointer */
   static   ulong    xcount  = 0xFFFFFFFF;      /* Horizontal Pixel Count */
   static   short    red, grn, blu;             /* RGB Color Components */

   if (xcount < image.last[0])
      ++xcount;
   else
   {  xcount = image.first[0];
      Read (instream, scanbuff, scanlsize);
      scanptr = scanbuff;
      ebflag = 1;
   }

   if (image.bitsperpixel == 24)
   {  red = *scanptr++ >> 4;
      grn = *scanptr++ >> 4;
      blu = *scanptr++ >> 4;
   }
   else if (ebflag)
   {  red = *scanptr >> 4;
      grn = *scanptr++ & 0x0F;
      blu = *scanptr >> 4;
      ebflag = 0;
   }
   else
   {  red = *scanptr++ & 0x0F;
      grn = *scanptr >> 4;
      blu = *scanptr++ & 0x0F;
      ebflag = 1;
   }

   return (red << 8) | (grn << 4) | blu;
}



/****************************************************************************
**  This procedure displays the current image.  If the user clicks the
**  closewindow gadget while this is running, the program is halted.
****************************************************************************/

void  DisplayImage  (slice, offx, offy)
   ulong  slice;      /* Image Slice to Display */
{
   auto     ulong  cpix;        /* HAM-Encoded Pixel Color */
   auto     short  crgb, prgb;  /* Current and Previous Color */
   register long   xx, yy;      /* Pixel X & Y Coordinates */

   /* Seek to the start of the image data. */

   if (-1 == Seek (instream, PLANE_START(slice), OFFSET_BEGINNING))
      Halt ("Seek failed on image file (%s).", infile);

   for (yy=0;  yy < resy;  ++yy)
   {
      auto uchar red, grn, blu;         /* Color Components */

      /* Check for closewindow clicks. */

      while (msg = (IntuiMessage *) GetMsg (win->UserPort))
      {  class = msg->Class;
         ReplyMsg (msg);
         if (class == CLOSEWINDOW)
            Halt ("Closewindow button clicked; aborting.", nil);
      }

      if (pixfact == 0)
      {  for (xx=0;  xx < resx;  ++xx)
         {
            prgb = crgb;
            crgb = ReadNextRGB ();
            cpix = GetHAM (cpix, &crgb, (xx==0) ? -1 : prgb);

            SetAPen (rastport, cpix);
            WritePixel (rastport, xx + (long)offx, yy + (long)offy);
         }
      }
      else
      {  for (xx=0;  xx < resx;  ++xx)
         {
            auto   long  xmin, ymin;      /* Pixel Corner */

            prgb = crgb;
            crgb = ReadNextRGB ();
            cpix = GetHAM (cpix, &crgb, (xx==0) ? -1 : prgb);

            SetAPen (rastport, cpix);
            xmin = (xx << pixfact) + (long)offx;
            ymin = (yy << pixfact) + (long)offy;
            RectFill (rastport, xmin, ymin, xmin+pixsize-1, ymin+pixsize-1);
         }
      }
   }
}



/****************************************************************************
**  This subroutine initializes the graphics screen and the display window.
**  It also determines the appropriate display resolution and window size.
****************************************************************************/

void  InitGraphics  ()
{
   auto ushort  lines;          /* Number of Image Scan Lines */

   /* Open the libraries. */

   if (!(GfxBase = OpenLibrary("graphics.library", 0L)))
      Halt ("Couldn't open graphics library.", nil);

   if (!(IntuitionBase = OpenLibrary("intuition.library", 0L)))
      Halt ("Couldn't open intuition library.", nil);

   /* Determine the output image size */

   if (sresy > 200)
      lines = 400;
   else
   {  auto  long  Diff320x200;   /* Aspect Ratio Difference from 320x200 */
      auto  long  Diff320x400;   /* Aspect Ratio Difference from 320x400 */

      Diff320x200 = (11L * image.aspect[0]) - (10L * image.aspect[1]);
      Diff320x400 = (11L * image.aspect[0]) - (20L * image.aspect[1]);

      Diff320x200 = ABS(Diff320x200);
      Diff320x400 = ABS(Diff320x400);

      lines = (Diff320x200 < Diff320x400) ? 200 : 400;
   }

   ScreenDef.Height = lines;
   if (lines == 400) ScreenDef.ViewModes |= LACE;

   WinDef.Height = WinDef.MinHeight = WinDef.MaxHeight = lines;

   /* Open the Graphics screen & window. */

   if (!(screen = OpenScreen (&ScreenDef)))
      Halt ("Can't open screen.", nil);

   WinDef.Screen = screen;

   if (!(win = OpenWindow (&WinDef)))
      Halt ("Can't open window.", nil);

   viewport = &screen->ViewPort;
   rastport = win->RPort;
   ShowTitle (screen, FALSE);

   /* Set up the color map. */

   LoadRGB4 (viewport, palette, (long)(NPALETTE));

   /* Erase the window's title bar. */

   SetAPen  (rastport, 0L);
   SetDrMd  (rastport, JAM1);
   RectFill (rastport, 0L, 0L, (long)(win->Width-1), 11L);
}



/****************************************************************************
**  This routine finds the best HAM encoding for the next pixel, given the
**  desired absolute 12-bit color.
****************************************************************************/

#define  HAM_RED  0x20     /* Specify New Red   Color Value */
#define  HAM_GRN  0x30     /* Specify New Green Color Value */
#define  HAM_BLU  0x10     /* Specify New Blue  Color Value */

ulong  GetHAM (ppix, crgb, prgb)
   ulong   ppix;        /* Previous Pixel's HAM Code */
   short  *crgb;        /* Current RGB-Encoded Color */
   short   prgb;        /* Previous RGB-Encoded Color */
{
   auto     ulong  BestAbs;             /* Best Absolute Color */
   auto     short  cred, cgrn, cblu;    /* Current RGB Color Components */
   register ulong  distance;            /* Color Distance */
   register ulong  ii;                  /* Scratch Integer */
   auto     ulong  newdist;             /* New Distance */
   auto     short  pred, pgrn, pblu;    /* Previous RGB Color Components */

   /* If the new color is the same as the previous color, then use the same
   ** HAM encoding as for the previous pixel.  */

   if (*crgb == prgb)  return ppix;

   /* Extract the RGB color components for further comparisons.  */

   cred = (*crgb >> 8) & 0xF;
   cgrn = (*crgb >> 4) & 0xF;
   cblu = (*crgb     ) & 0xF;

   pred = (prgb >> 8) & 0xF;
   pgrn = (prgb >> 4) & 0xF;
   pblu = (prgb     ) & 0xF;

   /* If only one component differs from the previous value, then specify
   ** the new value for that component and we're done.  */

   if (prgb != -1)
   {  if ((pgrn == cgrn) && (pblu == cblu))   return (cred | HAM_RED);
      if ((pblu == cblu) && (pred == cred))   return (cgrn | HAM_GRN);
      if ((pred == cred) && (pgrn == cgrn))   return (cblu | HAM_BLU);
   }

   /* Find the absolute color that best matches the current color.  If an
   ** absolute color matches exactly, then use that color.  */

   for (distance=1000, ii=0;  ii < NPALETTE;  ++ii)
   {
      if (palette[ii] == *crgb)  return ii;

      newdist = ColorDistance (palette[ii], cred, cgrn, cblu);
      if (newdist < distance)
      {  BestAbs  = ii;
         distance = newdist;
      }
   }

   /* If there was no previous pixel, then we have no choice but to do an
   ** absolute color, so return the "closest" absolute color encoding. */

   if (prgb == -1)
   {  *crgb = palette[BestAbs];
      return BestAbs;
   }

   /* Find the color component that differs the most from the previous
   ** color components.  The order of bias is green, red, blue, since
   ** this is the order of human color sensitivity (intensity). */

   ii = 1;
   newdist = ABS (cgrn - pgrn);

   if (ABS (cred - pred) > newdist)
   {  ii = 0;
      newdist = ABS (cred - pred);
   }

   if (ABS (cblu - pblu) > newdist)
      ii = 2;

   switch (ii)
   {
      case 0:
         if (ColorDistance (*crgb, cred, pgrn, pblu) < distance)
         {  *crgb = (cred << 8) | (pgrn << 4) | pblu;
            return HAM_RED | cred;
         }
         break;

      case 1:
         if (ColorDistance (*crgb, pred, cgrn, pblu) < distance)
         {  *crgb = (pred << 8) | (cgrn << 4) | pblu;
            return HAM_GRN | cgrn;
         }
         break;

      case 2:
         if (ColorDistance (*crgb, pred, pgrn, cblu) < distance)
         {  *crgb = (pred << 8) | (pgrn << 4) | cblu;
            return HAM_BLU | cblu;
         }
         break;
   }

   /* At this point, the best absolute color match is closer than the
   ** closest possible relative match, so return the absolute color. */

   *crgb = palette[BestAbs];      /* do a best absolute */
   return BestAbs;
}



/*************************************************************************
**  This function returns the "distance" between two absolute 12-bit
**  colors.
*************************************************************************/

ulong  ColorDistance  (color1, red2, grn2, blu2)
   short  color1;
   short  red2, grn2, blu2;
{
   register short red, grn, blu;        /* RGB Color Components */

   /* set up for comparisons */

   red = red2 - (color1 >> 8);
   grn = grn2 - ((color1 >> 4) & 0xF);
   blu = blu2 - (color1 & 0xF);

   return (red*red + grn*grn + blu*blu);
}
