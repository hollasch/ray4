/*******************************************************************************
"ray4" is Copyright (c) 1991,1992,1993,1996 by Steve R. Hollasch.

All rights reserved.  This software may be freely copied, modified and
redistributed, provided that this copyright notice is preserved in all copies.
This software is provided "as is", without express or implied warranty.  You
may not include this software in a program or other software product without
also supplying the source, except by express agreement with the author (Steve
Hollasch).  If you modify this software, please include a notice detailing the
author, date and purpose of the modification.
*******************************************************************************/

/*******************************************************************************

File:  r4_io.c

    This file contains the I/O routines needed for the Ray4 4D raytracer.  See
the r4_main.c header for more information on Ray4.  This version of r4_io.c
contains standard C I/O routines.

Revisions:

25-Jan-92  Hollasch
    Released to the public domain.

30-Sep-90  Steve R. Hollasch
    Initial version.

*******************************************************************************/

#include <stdio.h>
#include <fcntl.h>

#include "ray4.h"
#include "r4_globals.h"


    /***  Local Global Variables  ***/

FILE *instream  = nil;  /* Input Stream */
int   outstream = 0;    /* Output Stream */


/*****************************************************************************
These procedures close the input and output streams.
*****************************************************************************/

void  CloseInput  ()
{
    if (!infile || (*infile == 0) || (!instream))
        return;
    fclose (instream);
    instream = nil;
}

void  CloseOutput  ()
{
    if (!outstream)
        return;
    close (outstream);
    outstream = 0;
}



/*****************************************************************************
This routine returns then next character from the input stream.  If the end of
file is reached, it returns -1.
*****************************************************************************/

int UNREAD_NONE = -2;
int unreadChar = -2;

int  ReadChar  ()
{
    int character = unreadChar;

    if (unreadChar == UNREAD_NONE)
        return character;

    unreadChar = UNREAD_NONE;
    return character;
}

/*****************************************************************************
This routine, together with ReaedChar allow to put back a character, which is
something parsers often need. It cannot unread more than one character at
a time though.
*****************************************************************************/

void UnreadChar  (int character)
{
    unreadChar = character;
}



/*****************************************************************************
This subroutine opens the input file.  If no filename was given in the
command-line arguments, we'll use the standard input stream.
*****************************************************************************/

void  OpenInput  ()
{
    if (!infile || (*infile == 0))
        instream = stdin;
    else if (! (instream = fopen (infile, "r")))
        Halt ("Open failed on input file (%s).", infile);
}



/*****************************************************************************
This subroutine opens the output file.
*****************************************************************************/

void  OpenOutput  ()
{
    if (!outfile || (*outfile == 0))
        Halt ("No output file specified.");

    if (-1 == (outstream = open (outfile, O_TRUNC|O_CREAT|O_WRONLY, 0666)))
        Halt ("Open failed on output file (%s).", outfile);
}



/*****************************************************************************
This routine writes a block to the output file.
*****************************************************************************/

void  WriteBlock  (
    void *buff,    /* Source Buffer */
    int   num)     /* Number of Bytes to Write */
{
    if (num != write (outstream, buff, (int)(num)))
        Halt ("Write error to output file; aborting");
}
