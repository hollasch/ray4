
/***********************************************************************
**
**  "strcmpic" is Copyright (c) 1991 by Steve R. Hollasch.
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
