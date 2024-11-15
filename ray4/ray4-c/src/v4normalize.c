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
**  File:  v4normalize.c                         Author:  Steve Hollasch
**
**      This procedure normalizes a 4-vector.  If the vector is shorter than
**    the square root of 1e-30, this routine will return 0, and will not
**    modify the vector.  If the vector has a non-zero length greater than
**    the root of 1e-30, then the vector will be normalized and this routine
**    will return 1.
**
****************************************************************************/

#include "r4_vector.h"


short  V4_Normalize  (V)
   Vector4  V;                  /* Vector to Normalize */
{
   auto double  norm;   /* Norm of the Vector */

   norm = V4_Dot (V,V);
   if (norm < 1e-30)
      return 0;

   V4_Scalar (V, /=, norm);

   return 1;
}
