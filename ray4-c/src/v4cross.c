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

#include "vector.h"


void  V4_Cross  (result,u,v,w)
   Vector4  result;     /* Result Vector */
   Vector4  u, v, w;    /* Source Vectors */
{
   auto Real     A, B, C, D, E, F;      /* Intermediate Values */
   auto Vector4  temp;                  /* Intermediate Vector */

   A = (v[0] * w[1]) - (v[1] * w[0]);
   B = (v[0] * w[2]) - (v[2] * w[0]);
   C = (v[0] * w[3]) - (v[3] * w[0]);
   D = (v[1] * w[2]) - (v[2] * w[1]);
   E = (v[1] * w[3]) - (v[3] * w[1]);
   F = (v[2] * w[3]) - (v[3] * w[2]);

   temp[0] =   (u[1] * F) - (u[2] * E) + (u[3] * D);
   temp[1] = - (u[0] * F) + (u[2] * C) - (u[3] * B);
   temp[2] =   (u[0] * E) - (u[1] * C) + (u[3] * A);
   temp[3] = - (u[0] * D) + (u[1] * B) - (u[2] * A);

   V4_2Vec (result,=,temp);
}
