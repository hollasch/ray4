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

//==================================================================================================
// r4_io.cpp
//
// This file contains the I/O routines needed for the Ray4 4D raytracer. See the r4_main.cpp header
// for more information on Ray4.
//==================================================================================================

#include <stdio.h>
#include <fcntl.h>

#include "ray4.h"


    /***  Local Global Variables  ***/

FILE *instream  = nullptr;  // Input Stream
FILE *outstream = nullptr;  // Output Stream



//__________________________________________________________________________________________________

void CloseInput () {
    // Closes the input stream.

    if (!instream)
        return;
    fclose (instream);
    instream = nullptr;
}

//__________________________________________________________________________________________________

void CloseOutput () {
    // Closes the output stream.

    if (!outstream)
        return;
    fclose (outstream);
    outstream = nullptr;
}

//__________________________________________________________________________________________________

const int UNREAD_NONE = -2;
static int unreadChar = UNREAD_NONE;

int ReadChar () {
    // This routine returns then next character from the input stream. If the end of file is
    // reached, it returns -1.

    if (unreadChar == UNREAD_NONE)
        return getc(instream);

    int c = unreadChar;
    unreadChar = UNREAD_NONE;
    return c;
}

//__________________________________________________________________________________________________

void UnreadChar (int c) {
    // This routine, together with ReadChar allow to put back a character, which is something
    // parsers often need. It cannot unread more than one character at a time though.

    unreadChar = c;
}

//__________________________________________________________________________________________________

void OpenInput (const char* fileName) {
    // This subroutine opens the input file. If no filename was given in the command-line arguments,
    // we'll use the standard input stream.

    instream = fopen (fileName, "r");
    if (!instream)
        Halt ("Open failed on input file (%s).", fileName);
}

//__________________________________________________________________________________________________

void OpenOutput (const char* fileName) {
    // This subroutine opens the output file.

    outstream = fopen (fileName, "wb");
    if (!outstream)
        Halt ("Open failed on output file (%s).", fileName);
}

//__________________________________________________________________________________________________

void WriteBlock (
    void *buff,  // Source Buffer
    int   num)   // Number of Bytes to Write
{
    // This routine writes a block to the output file.

    if (num != fwrite (buff, 1, num, outstream))
        Halt ("Write error to output file; aborting");
}
