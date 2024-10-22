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
**  File:  w4_parse.c                               Author:  Steve Hollasch
**
**      This program reads in an object-description file and displays the
**  4D wireframe on an Iris workstation.
**
**      This file contains the procedures that parse the input file and load
**  the object description & initial viewing parameters into the global
**  variables and data structures.
**
**      The input file may specify several aspects of the object.  The
**  commands include:
**
**      From4   <4-Vector>
**      To4     <4-Vector>
**      Up4     <4-Vector>
**      Over4   <4-Vector>
**      Vangle4 <Real>
**
**      From3   <3-Vector>
**      To3     <3-Vector>
**      Up3     <3-Vector>
**      Vangle3 <Real>
**
**      ColorMap   <Integer Index> <Red[0,255]> <Green[0,255]> <Blue[0,255]>
**
**      DepthCue        <On|Off>
**      DepthCueLevels  <Integer>
**      DepthFar        <Real>
**      DepthNear       <Real>
**      ColorFar        <Red[0,255]> <Green[0,255]> <Blue[0,255]>
**      ColorNear       <Red[0,255]> <Green[0,255]> <Blue[0,255]>
**
**      VertexList <Integer Count> <4-Vector> <4-Vector> ... <4-Vector>
**      EdgeList   <Integer Count> <Vertex Pair & Color> ...
**
**      The following are considered white space: " <TAB><CR>,:;".
**      Comments are preceded with a '#' and continue until the end of the
**      current line.
**
**
**  Revisions:
**
**  1.00  19-Jan-92  Hollasch
**        First public domain release.
**
**  0.01  15-Jul-91  Hollasch, Marty Ryan
**        Removed inclusion of unix.h
**
**  0.00  91-Mar-12  Steve R. Hollasch
**
****************************************************************************/

#include <stdio.h>
#include <gl.h>

#include <vector.h>
#include "wire4.h"


   /*******************************/
   /***  File Global Variables  ***/
   /*******************************/

static char *GetToken    ARGS((FILE*));
static void  ReadColor   ARGS((FILE*, RGBvalue[3]));
static long  ReadInteger ARGS((FILE*));
static Real  ReadReal    ARGS((FILE*));
static void  ReadVector  ARGS((FILE*, Real[], int));


   /*******************************/
   /***  File Global Variables  ***/
   /*******************************/

static char  whitespace[]=" \t\n\f\r,:;/";   /* String of Separator Chars */



/****************************************************************************
//  This routine reads in the 4D object description from the input stream,
//  according to the format given at the head of this source file.  The
//  `input' parameter is a pointer to a FILE structure of an already opened
//  input stream.  The calling code is responsible for closing the file
//  after this routine returns control to the caller.
****************************************************************************/

void  ReadObject  (input)
   FILE *input;                 /* Input Stream */
{
   register int   ii;           /* Loop Counter */
   auto     char  inbuff[1024]; /* Input Buffer */
   register char *token;        /* Next Input File Token */

   V3_3Scalar (dc_ColorNear, =, 244,244,244);
   V3_3Scalar (dc_ColorFar,  =,  63, 63, 63);
   dc_DepthNear = -1.0;
   dc_DepthFar  = -1.0;
   dc_Levels    = 16;
   NumVerts     = 0;
   NumEdges     = 0;

   while (NULL != (token = GetToken(input)))
   {
      if (streqic (token, "from3"))
         ReadVector (input, From3, 3);

      else if (streqic (token, "to3"))
         ReadVector (input, To3, 3);

      else if (streqic (token, "up3"))
         ReadVector (input, Up3, 3);

      else if (streqic (token, "vangle3"))
      {  Vangle3 = ReadReal(input);
         if ((Vangle3 <= 0.0) || (180.0 <= Vangle3))
         {  fprintf(stderr,"wire4:  Vangle3 out of range (%lg).\n",Vangle3);
            Vangle3 = 45.0;
         }
      }

      else if (streqic (token, "project3"))
      {  if (!(token = GetToken(input)))
            Halt ("Expected 'Perspective' or 'Parallel' after 'Project3'");
	 if (streqic (token, "perspective"))
	    Project3 = PERSPECTIVE;
	 else if (streqic (token, "parallel"))
	    Project3 = PARALLEL;
	 else
            Halt ("Expected 'Perspective' or 'Parallel' after 'Project3'");
      }

      else if (streqic (token, "from4"))
         ReadVector (input, From4, 4);

      else if (streqic (token, "to4"))
         ReadVector (input, To4, 4);

      else if (streqic (token, "up4"))
         ReadVector (input, Up4, 4);

      else if (streqic (token, "over4"))
         ReadVector (input, Over4, 4);

      else if (streqic (token, "vangle4"))
      {  Vangle4 = ReadReal(input);
         if ((Vangle4 <= 0.0) || (180.0 <= Vangle4))
         {  fprintf(stderr,"wire4:  Vangle4 out of range (%lg).\n",Vangle4);
            Vangle4 = 45.0;
         }
      }

      else if (streqic (token, "project4"))
      {  if (!(token = GetToken(input)))
            Halt ("Expected 'Perspective' or 'Parallel' after 'Project4'");
	 if (streqic (token, "perspective"))
	    Project4 = PERSPECTIVE;
	 else if (streqic (token, "parallel"))
	    Project4 = PARALLEL;
	 else
            Halt ("Expected 'Perspective' or 'Parallel' after 'Project4'");
      }

      else if (streqic (token, "refcube"))
      {  if (!(token = GetToken(input)))
            Halt ("Expected 'On' or 'Off' after RefCube keyword");
	 if (streqic (token, "on"))
	    RefCube = true;
	 else if (streqic (token, "off"))
	    RefCube = false;
	 else
            Halt ("Expected 'On' or 'Off' after RefCube keyword");
      }

      else if (streqic (token, "colormap"))
      {  auto int index;	/* Color Map Index */

      	 index = ReadInteger (input);
	 if ((index < 0) || (255 < index))
	    Halt ("Color index is out of range of [0,255]");
	 ReadColor (input, cmap[index]);
      }

      else if (streqic (token, "depthcue"))
      {  if (!(token = GetToken(input)))
            Halt ("Expected 'on' or 'off' after DepthCue keyword");
	 if (streqic (token, "on"))
	    DepthCue = true;
	 else if (streqic (token, "off"))
	    DepthCue = false;
	 else
	    Halt ("Expected 'on' or 'off' after DepthCue keyword");
      }

      else if (streqic (token, "depthfar"))
      {  if (0.0 > (dc_DepthFar = ReadReal(input)))
	    Halt ("FarDepth value must be non-negative");
      }

      else if (streqic (token, "depthnear"))
      {  if (0.0 > (dc_DepthNear = ReadReal(input)))
	    Halt ("NearDepth value must be non-negative");
      }

      else if (streqic (token, "colorfar"))
         ReadColor (input, dc_ColorFar);

      else if (streqic (token, "colornear"))
         ReadColor (input, dc_ColorNear);

      else if (streqic (token, "depthcuelevels"))
      {  dc_Levels = ReadInteger (input);
         if ((dc_Levels < 1) || (dc_Levels > 255))
	    Halt ("DepthCueLevels must be in [1,255]");
      }

      else if (streqic (token, "vertexlist"))
      {
         if (!(token = GetToken(input)))
            Halt ("Expected the number of vertices after VertexList");

         NumVerts = OBJ_VSTART + atoi (token);
         VertList = (Vertex *) malloc (NumVerts * sizeof(VertList[0]));

         for (ii=OBJ_VSTART;  ii < NumVerts;  ++ii)
            ReadVector (input, VertList[ii].position, 4);
      }

      else if (streqic (token, "edgelist"))
      {
         if (!(token = GetToken(input)))
            Halt ("Expected the number of edges after EdgeList");

         NumEdges = OBJ_ESTART + atoi (token);
         EdgeList = (Edge *) malloc (NumEdges * sizeof(EdgeList[0]));

         for (ii=OBJ_ESTART;  ii < NumEdges;  ++ii)
         {
	    EdgeList[ii].v1    = OBJ_VSTART + ReadInteger(input);
	    EdgeList[ii].v2    = OBJ_VSTART + ReadInteger(input);
	    EdgeList[ii].color = ReadInteger(input);

	    if ((EdgeList[ii].v1 <OBJ_VSTART)||(EdgeList[ii].v2 <OBJ_VSTART))
	       Halt ("Negative edge vertex number");
         }
      }

      else
      {  fprintf (stderr, "wire4:  Unexpected command word (%s).\n", token);
         Halt ("Aborting");
      }
   }

   if (!NumVerts)
      Halt ("No vertices given in input file");

   if (!NumEdges)
      Halt ("No edges given in input file");

   if (  (dc_DepthNear >= 0.0) && (dc_DepthFar >= 0.0)
      && (dc_DepthNear >= dc_DepthFar))
      Halt ("FarDepth must be greater than NearDepth");
}



/****************************************************************************
//  This routine reads the given input stream with respect to the given
//  whitespace character string and returns the next token in the stream.
//  If this function reaches the end of the stream, it returns NULL,
//  otherwise it returns a pointer to the token.  Tokens cannot be longer
//  than 255 bytes in length.
****************************************************************************/

static char  *GetToken  (input)
   FILE *input;                 /* Token Input Stream */
{
   static   char  buff[256];    /* Token Buffer */
   register int   cc;           /* Input Character */
   register int   len;          /* Token Length */
   register char *tok;          /* Token String Pointer */
   register char *whitechar;    /* Whitespace Character Pointer */

   for (tok=buff, len=0;  EOF != (cc = fgetc(input));  )
   {
      /* If the current character is a comment opener (#), then discard
      // the rest of the line. */

      if (cc == '#')
      {  while (cc=fgetc(input), (cc != EOF) && (cc != '\n'))
            ;
         continue;
      }

      /* Make sure the current character is not a whitespace character. */

      whitechar = whitespace;
      while ((*whitechar != 0) && (cc != *whitechar))
      ++whitechar;

      /* If it is a whitespace character, then either we have not yet reached
      // the start of the next token or we have reached the end of the
      // current token.  If we've reached the end, then null-terminate the
      // token and return it to the calling routine.  If the current token
      // has not begun, then continue to read characters from the input
      // stream.  */

      if (*whitechar != 0)
      {  if (len == 0)
	    continue;
         else
         {  *tok = 0;
	    return buff;
         }
      }

      /* If the current character is not a whitespace character, then add
      // the character to the token and continue.  */

      if (++len > (sizeof(buff)-1))
	 Halt ("Token too large (greater than 255 characters)");
      *tok++ = cc;
   }

   return NULL;
}



/****************************************************************************
//  This subroutine reads the next RGB triple from the input stream and
//  loads it into the RGB array.
****************************************************************************/

static void  ReadColor  (input, color)
   FILE     *input;		/* Token Input Stream */
   RGBvalue  color[3];		/* RGB Triple Array */
{
   register char *token;	/* Next Input Token */

   if (!(token = GetToken (input)))
      Halt ("Unexpected end-of-file (looking for red color component)");
   color[0] = atoi (token);

   if (!(token = GetToken (input)))
      Halt ("Unexpected end-of-file (looking for green color component)");
   color[1] = atoi (token);

   if (!(token = GetToken (input)))
      Halt ("Unexpected end-of-file (looking for blue color component)");
   color[2] = atoi (token);
}



/****************************************************************************
//  This subroutine reads the next integer-valued number from the input
//  stream and returns the value.
****************************************************************************/

static long  ReadInteger  (input)
   FILE *input;		/* Token Input Stream */
{
   register char *token;	/* Next Input Token */

   if (!(token = GetToken (input)))
      Halt ("Unexpected end-of-file (looking for integer)");

   return atol (token);
}



/****************************************************************************
//  This subroutine reads the next real-valued number from the input stream
//  and returns the value.
****************************************************************************/

static Real  ReadReal  (input)
   FILE *input;                 /* Token Input Stream */
{
   register char   *token;      /* Next Input Token */

   if (NULL == (token = GetToken (input)))
      Halt ("Unexpected end-of-file (looking for real)");

   return atof (token);
}



/****************************************************************************
//  This subroutine reads the next token from the input stream and loads the
//  vector values into the specified vector.
****************************************************************************/

static void  ReadVector  (input, vector, dimension)
   FILE *input;		/* Input Stream */
   Real  vector[];	/* Destination Vector */
   int   dimension;	/* Vector Dimension */
{
   register int ii;	/* Index Variable */

   for (ii=0;  ii < dimension;  ++ii)
      vector[ii] = ReadReal(input);
}
