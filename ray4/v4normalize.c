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

#include <vector.h>


short  V4_Normalize  (V)
   Vector4  V;			/* Vector to Normalize */
{
   auto double  norm;	/* Norm of the Vector */

   norm = V4_Dot (V,V);
   if (norm < 1e-30)
      return 0;

   V4_Scalar (V, /=, norm);

   return 1;
}
