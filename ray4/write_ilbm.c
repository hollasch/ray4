/***************************************************************************
**
**  File: WriteILBM.c                           Author:  Steve R. Hollasch
**
**    These routines write out an Amiga screen image to an ILBM image file.
**    The main routine is WriteILBM, which is called as follows:
**
**        short  WriteILBM  (char *fname,  Screen *screen)
**
**    This routine saves the image of the entire screen to an IFF file.
**    If the file write succeeds, WriteILBM will return 1, otherwise it
**    will return 0.
**
**
**  Revisions:
**    1.00  31-Oct-90  SRH
**          Initial version.
**
****************************************************************************/

#include <stdio.h>

#include <intuition/intuition.h>
#include "amiga.h"


   /***  Function Parameters  ***/

#define MAXWIDTH    640			/* Max Image Width (Pixels) */
#define MAXPLANES   6			/* Max # Image Bit Planes */
#define MAXROWSIZE  (MAXWIDTH/ 8)	/* Max # Bytes Image Bit-Plane Row */


   /***  Type Definitions  ***/

typedef struct FileHandle    Fhandle;
typedef struct RastPort      RastPort;
typedef struct Screen        Screen;
typedef struct ViewPort      ViewPort;


   /***  Function Declarations  ***/

void    WriteILBMBody (/* FILE*, long*, Screen* */);
ushort  PackRow       (/* uchar*, uchar*, ushort */);
ushort  PutLiteral    (/* uchar**, uchar*, int */);
ushort  PutPacked     (/* uchar**, uchar*, int */);


   /***  Data Structures  ***/

struct
{  ushort  wide, high;			/* Raster Width, Height */
   short   x, y;			/* Pixel Position, This Image */
   uchar   nPlanes;			/* Number of Source BitPlanes */
   uchar   masking;			/* Masking Type */
   uchar   compression;			/* Compression Type */
   uchar   pad1;			/* Unused */
   ushort  transparcolor;		/* Transparent Color Number */
   uchar   xAspect, yAspect;		/* Aspect Ratio  width:height */
   short   pageWidth, pageHeight;	/* Page Width & Height */
} BitMapHeader
=
{  0, 0,	/* wide, high */
   0, 0,	/* x, y */
   1,		/* nPlanes */
   0,		/* masking */
   1,		/* compression */
   0,		/* pad1 */
   0,		/* transparcolor */
   0, 0,	/* xAspect, yAspect */
   0, 0		/* pageWidth, pageHeight */
};


   /***  Global Variable Definitions  ***/

uchar  nplanes;		/* Number of Image Bitplanes */



/****************************************************************************
**  This procedure writes the image to disk in the IFF format.  See the file
**  header for a functional description.
****************************************************************************/

short  WriteILBM  (fname, screen)
   char   *fname;	/* Destination File Name */
   Screen *screen;	/* Image Screen */
{
   auto     long    bodysize;	/* Size of the BODY Property Chunk */
   auto     long    formsize;	/* Size of the FORM Property Chunk */
   auto     long    lng;	/* Temporary Long Value */
   register FILE   *out;	/* Output Stream */
   auto     long    pos_body;	/* Position of BODY Size Value in File */
   auto     long    pos_form;	/* Position of FORM Size Value in File */


   /* Load the global variables */

   nplanes = screen->BitMap.Depth;

   /* Open the output file. */

   if (!(out = fopen (fname, "w")))
      return false;

   /* Write the FORM block. */

   fwrite ("FORM", 1, 4, out);
   pos_form = ftell (out);
   lng = 0;
   fwrite (&lng, 4, 1, out);

   /* Write the ILBM keyword. */

   fwrite ("ILBM", 1, 4, out);

   /* Write the bitmap header. */

   fwrite ("BMHD", 1, 4, out);
   lng = sizeof(BitMapHeader);
   fwrite (&lng, 4, 1, out);

   BitMapHeader.yAspect    = 11;

   if (screen->Width == 320)
      BitMapHeader.xAspect = (screen->Height == 200) ? 10 : 20;
   else /*  Width == 640  */
      BitMapHeader.xAspect = (screen->Height == 200) ?  5 : 10;

   BitMapHeader.nPlanes    = nplanes;
   BitMapHeader.wide       = screen->Width;
   BitMapHeader.high       = screen->Height;
   BitMapHeader.pageWidth  = screen->Width;
   BitMapHeader.pageHeight = screen->Height;

   fwrite (&BitMapHeader, 1, sizeof(BitMapHeader), out);

   /* Write out the Commodore Amiga ViewModes. */

   fwrite ("CAMG", 1, 4, out);
   lng = 4L;
   fwrite (&lng, 1, 4, out);
   lng = screen->ViewPort.Modes;
   fwrite (&lng, 1, 4, out);

   /* Write out the color map. */

   {  auto  struct ColorMap *cmap;		/* Color Map Pointer */
      auto  long             ii;		/* Index Variable */

      fwrite ("CMAP", 1, 4, out);
      lng = 3 * screen->ViewPort.ColorMap->Count;
      fwrite (&lng, 1, 4, out);

      cmap = screen->ViewPort.ColorMap;

      for (ii=0;  ii < screen->ViewPort.ColorMap->Count;  ++ii)
      {  auto uchar  RGB[3];		/* 24-bit Color Triple */
   	 auto short  color;		/* 12-bit Packed Color Value */

   	 color  = GetRGB4 (cmap, ii);
   	 RGB[0] = (color >> 4) & 0xF0;
   	 RGB[1] = (color     ) & 0xF0;
   	 RGB[2] = (color << 4) & 0xF0;

   	 fwrite (RGB, 1, sizeof(RGB), out);
      }
   }

   /* Write out the image body. */

   WriteILBMBody (out, &pos_body, screen);

   /* Now update both the FORM size and the BODY size. */

   bodysize = ftell (out) - pos_body - 4;
   formsize = ftell (out) - pos_form - 4;

   fseek (out, pos_form, 0);
   fwrite (&formsize, 1, 4, out);

   fseek (out, pos_body, 0);
   fwrite (&bodysize, 1, 4, out);

   /* Done; close the output file and return. */

   fclose (out);

   return true;
}



/****************************************************************************
**  This routine writes the image body data to the designated output file
**  in the IFF graphics format.
****************************************************************************/

void  WriteILBMBody  (out, pos_body, screen)
   FILE   *out;		/* Output File Stream */
   long   *pos_body;	/* BODY Property Size Location */
   Screen *screen;	/* Image Screen */
{
   auto	ushort  nrowbytes;		/* # Bytes Per Image Bit-Plane Row */
   auto	long    ii, jj;			/* Loop Variables */
   auto	uchar  *imgdata[MAXPLANES];	/* Image Data, Each Plane */
   auto	short   rowinc;			/* Image Row Increment (Bytes) */

   fwrite ("BODY", 1, 4, out);
   *pos_body = ftell (out);
   ii = 0;
   fwrite (&ii, 1, 4, out);

   for (ii=0;  ii < nplanes;  ++ii)
      imgdata[ii] = screen->RastPort.BitMap->Planes[ii];

   for (jj=0;  jj < screen->Height;  ++jj)
   {
      for (ii=0;  ii < nplanes;  ++ii)
      {  auto uchar   rowbuff[MAXROWSIZE];	/* Encoded Row Buffer */
	 auto ushort  npacked;			/* # Bytes in Packed Row */

   	 npacked = PackRow (imgdata[ii],rowbuff,screen->BitMap.BytesPerRow);
   	 fwrite (rowbuff, 1, npacked, out);
   	 imgdata[ii] += screen->BitMap.BytesPerRow;
      }
   }
}



/****************************************************************************
**  This subroutine is used to pack an image row with the ByteRun1 IFF
**  encoding scheme.
****************************************************************************/

typedef enum { LITERAL, REPEAT } RunType;

ushort  PackRow  (src, dest, num)
   uchar  *src;		/* Source Buffer */
   uchar  *dest;	/* Destination (Encoded) Buffer */
   ushort  num;		/* Number of Source Bytes to Encode */
{
   auto     ushort   bcount;	/* Byte Output Count */
   auto     uchar   *blockptr;	/* Start of Current Block */
   auto     uchar   *dptr;	/* Destination Block Pointer */
   auto     uchar   *lastsrc;	/* Pointer to Last Source Byte */
   auto     uchar   *repstart;	/* Repeat Run Length */
   auto     RunType  runtype;	/* Block Run Type (Literal or Repeat) */
   register uchar   *sptr;	/* Source Buffer Pointer */

   bcount   = 0;
   lastsrc  = src + (num - 1);
   runtype  = LITERAL;
   blockptr = src;
   repstart = src;
   dptr     = dest;

   for (sptr=src+1;  sptr <= lastsrc;  ++sptr)
   {
      if (runtype == LITERAL)
      {
   	 if (*sptr != sptr[-1])
	    repstart = sptr;
   	 else if ((sptr - repstart) >= 2)
   	 {
	    if (blockptr < repstart)
	    {  bcount += PutLiteral (&dptr, blockptr, (repstart - blockptr));
	       blockptr = repstart;
	    }
	    runtype  = REPEAT;
   	 }
      }
      else if (*sptr != *blockptr)
      {  bcount += PutPacked (&dptr, blockptr, (sptr - repstart));
	 blockptr = repstart = sptr;
	 runtype  = LITERAL;
      }
   }

   if (runtype == LITERAL)
      bcount += PutLiteral (&dptr, blockptr, (sptr - blockptr));
   else /* runtype == REPEAT */
      bcount += PutPacked (&dptr, blockptr, (sptr - repstart));

   return bcount;
}



/****************************************************************************
**  This function writes out the source block as a ByteRun1 block of literal
**  bytes.  It returns the number of bytes written to the destination buffer
**  and sets the destination pointer to one byte after the encoded block.
****************************************************************************/

ushort  PutLiteral  (dest, block, len)
   uchar **dest;	/* Destination Buffer */
   uchar  *block;	/* Source Block */
   int     len;		/* Length of Source Block */
{
   auto	ushort  nwritten;	/* Number of Bytes Written */
   auto	ushort  blklen;		/* Current Block Length */

   nwritten = 0;

   for (nwritten=0;  len > 0;  len -= blklen)
   {
      blklen = MIN (len, 128);
      *(*dest)++ = blklen - 1;
      movmem (block, *dest, blklen);
      *dest += blklen;
      block += blklen;
      nwritten += (1 + blklen);
   }

   return nwritten;
}



/****************************************************************************
**  This function writes out the source block as a ByteRun1 block of repeated
**  bytes.  It returns the number of bytes written to the destination buffer
**  and sets the destination pointer to one byte after the encoded block.
****************************************************************************/

ushort  PutPacked  (dest, block, len)
   uchar **dest;	/* Destination Buffer */
   uchar  *block;	/* Source Block */
   int     len;		/* Length of Source Block */
{
   auto	ushort  nwritten;	/* Number of Bytes Written */
   auto	ushort  blklen;		/* Current Block Length */

   nwritten = 0;

   for (nwritten=0;  len > 0;  len -= blklen)
   {
      blklen = MIN (len, 128);
      *(*dest)++ = 1 - blklen;
      *(*dest)++ = *block;
      nwritten += 2;
   }

   return nwritten;
}
