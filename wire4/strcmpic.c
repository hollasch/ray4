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
**  File:  strcmpic.c                              Author:  Steve Hollasch
**
**      This file contains the procedure strcmpic().  It compares two strings
**  in the same manner that strcmp() does, except that it ignores differences
**  between uppercase and lowercase letters.
**
****************************************************************************/

#include <ctype.h>


int  strcmpic  (s1, s2)
   register char *s1, *s2;		/* The Two Source Strings */
{
   auto char c1, c2;		/* The Current String Characters */

   while (c1 = *s1++, c2 = *s2++, (c1 && c2))
   {
      if (isupper(c1))  c1 |= 0x20;
      if (isupper(c2))  c2 |= 0x20;

      if (c1 != c2)  return (c1 < c2) ? -1 : 1;
   }

   return (c1 == c2) ? 0 : ((c1 < c2) ? -1 : 1);
}
