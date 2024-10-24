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
**  File:  v3cross.c                              Author:  Steve Hollasch
**
**      This routine forms the cross product of two 3-vectors.  Note that the
**    destination vector may be one of the source vectors.
**
****************************************************************************/

#include <vector.h>


void  V3_Cross  (result,u,v)
   Vector3  result;		/* Result Vector */
   Vector3  u, v;		/* Source Vectors */
{
   auto Vector3  temp;		/* Intermediate Vector */

   temp[0] = (u[1] * v[2]) - (u[2] * v[1]);
   temp[1] = (u[2] * v[0]) - (u[0] * v[2]);
   temp[2] = (u[0] * v[1]) - (u[1] * v[0]);

   V3_2Vec (result,=,temp);
}
