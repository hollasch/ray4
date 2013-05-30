
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
