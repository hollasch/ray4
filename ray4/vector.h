/****************************************************************************
//
//  File:  vector.h                              Author:  Steve Hollasch
//
//      This header file contains the structure and macro definitions for
//  three and four-dimensional vectors.
//
****************************************************************************/

#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>


#if !defined (Real)
#   define Real  double
#endif

   /****************************/
   /***  Vector Definitions  ***/
   /****************************/

typedef  Real    Vector3 [3];	/* 3-Vector */
typedef  Real    Vector4 [4];	/* 4-Vector */

#define  Point3  Vector3	/* 3D Point (for readability) */
#define  Point4  Vector4	/* 4D Point (for readability) */


      /*  NOTE:  All of the following macros are unsafe; they cannot
      //         properly handle macro arguments that have side effects,
      //         such as post/pre increment or decrement.      */


   /************************************/
   /***  3-Vector Macro Definitions  ***/
   /************************************/

      /* This macro yields a list of the vector components */
	
#define V3_List(V)    V[0],V[1],V[2]

      /* This macro provides for scalar multiplication, division, or
      // assignment with a given 3-vector.  */

#define V3_Scalar(V,assign,k) \
if (1) \
{  V[0] assign (k); \
   V[1] assign (k); \
   V[2] assign (k); \
}else

      /* This macro provides for scalar multiplication or assignment
      // with a given 3-vector, where each coordinate has its own scalar
      // factor.   For example: V3_3Scalar (A, /=, scale0,scale1,scale2). */

#define V3_3Scalar(V,assign,a,b,c) \
if (1) \
{  V[0] assign (a); \
   V[1] assign (b); \
   V[2] assign (c); \
}else

      /* This macro provides for arithmetic operations of two 3-vectors.
      // For example: V3_2Vec( A, +=, B).  */

#define V3_2Vec(V,assign,U) \
if (1) \
{  V[0] assign U[0]; \
   V[1] assign U[1]; \
   V[2] assign U[2]; \
}else

      /* The V3_3Vec macro allows for operations with two source 3-vectors
      // and a destination 3-vector, e.g.  V3_3Vec (A, +=, B, -, C).  */

#define V3_3Vec(V,assign,A,op,B)  \
if (1) \
{  V[0] assign A[0] op B[0]; \
   V[1] assign A[1] op B[1]; \
   V[2] assign A[2] op B[2]; \
}else

      /* This macro calculates the dot product of two 3-vectors. */

#define V3_Dot(V,U) \
   ( (V[0]*U[0]) + (V[1]*U[1]) + (V[2]*U[2]) )

      /* V3_Norm calculates the norm (length) of a 3-vector. */

#define V3_Norm(V)   sqrt(V3_Dot(V,V))


   /************************************/
   /***  4-Vector Macro Definitions  ***/
   /************************************/

      /* This macro yields a list of the vector components */
	
#define V4_List(V)    V[0],V[1],V[2],V[3]

      /* This macro provides for scalar multiplication, division, or
      // assignment with a given 4-vector.  */

#define V4_Scalar(V,assign,k) \
if (1) \
{  V[0] assign (k); \
   V[1] assign (k); \
   V[2] assign (k); \
   V[3] assign (k); \
}else

      /* This macro provides for scalar multiplication or assignment
      // with a given 4-vector, where each coordinate has its own scalar
      // factor.   For example: V4_4Scalar (A, /=, x, y, z, w).  */

#define V4_4Scalar(V,assign,a,b,c,d) \
if (1) \
{  V[0] assign (a); \
   V[1] assign (b); \
   V[2] assign (c); \
   V[3] assign (d); \
}else

      /* This macro provides for arithmetic operations of two 4-vectors.
      // For example: V4_2Vec( A, +=, B).  */

#define V4_2Vec(V,assign,U) \
if (1) \
{  V[0] assign U[0]; \
   V[1] assign U[1]; \
   V[2] assign U[2]; \
   V[3] assign U[3]; \
}else

      /* The V4_3Vec macro allows for operations with two source 4-vectors
      // and a destination 4-vector, e.g.  V4_3Vec (A, +=, B, -, C).  */

#define V4_3Vec(V,assign,A,op,B) \
if (1) \
{  V[0] assign A[0] op B[0]; \
   V[1] assign A[1] op B[1]; \
   V[2] assign A[2] op B[2]; \
   V[3] assign A[3] op B[3]; \
}else

      /* This macro calculates the dot product of two 4-vectors. */

#define V4_Dot(V,U) \
   ( (V[0]*U[0]) + (V[1]*U[1]) + (V[2]*U[2]) + (V[3]*U[3]) )

      /* V4_Norm calculates the norm (length) of a 4-vector. */

#define V4_Norm(V)   sqrt(V4_Dot(V,V))


   /*******************************/
   /***  Function Declarations  ***/
   /*******************************/

void   V3_Cross		(/* Vector3, Vector3, Vector3 */);
void   V4_Cross		(/* Vector4, Vector4, Vector4, Vector4 */);
short  V3_Normalize	(/* Vector3 */);
short  V4_Normalize	(/* Vector4 */);

#endif
