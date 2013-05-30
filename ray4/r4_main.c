/****************************************************************************
//
//  File:  r4_main.c
//
//    This file contains the main procedures in the Ray4 4D ray tracer.
//    This program is written to run on the Silicon Graphics Iris work-
//    station, but should be able to run in any environment that supports
//    the main Unix functions.  The following files comprise the Ray4 ray
//    tracer:
//
//        ray4.h      :  Ray4 Common Header File
//        r4_main.c   :  Main Program File
//        r4_hit.c    :  Ray/Object Intersection Routines
//        r4_io.c     :  Input/Output Routines
//        r4_parse.c  :  Routines for Parsing the Input File
//        r4_trace.c  :  Ray Firing, Shading & Illumination Procedures
//
//  Revisions:
//
//    1.25 27-May-91  Hollasch
//         Fixed bug in tetrahedron/parallelepiped normal and intersection
//         assignment code; previous version did not ensure that the
//         passed parameter pointers were non-nil.
//
//    1.24 15-May-91  Hollasch
//         Fixed logic flaw in HitSphere t1/t2 selection.
//
//    1.23 12-May-91  Hollasch
//         Fixed bug in shadow-ray intersection routine call; `mindist' was
//         incorrectly passed by value, rather than by reference.
//
//    1.22 04-Mar-91  Hollasch
//         Moved global variables & declarations to r4_globals.h.
//         Renamed r4image.h to r4_image.h.
//         Fixed error in r4_trace.c where objects behind point light sources
//           could still produce shadows.
//
//    1.21 02-Feb-91  Hollasch
//         Altered scanline buffer implementation to use a minimum sized
//         buffer; moved buffer constants from ray4.h to r4_main.c
//
//    1.20 05-Jan-91  Hollasch
//         Implemented the attribute list.  Color-rendering attributes are
//         no longer embedded in each object description.  Also reversed
//         the meaning of the directional light source direction:  the
//         given vector is the direction _towards_ the light source.
//
//    1.10 19-Dec-90  Hollasch
//         Added parallelepipeds; generalized tetrahedra code to accomodate
//         parallelepipeds with a minimum of additional code.
//
//    1.03 13-Dec-90  Hollasch
//         Added internal verification code for tetrahedra for debugging
//         purposes.  Added reflections.
//
//    1.02 05-Dec-90  Hollasch
//         Fixed problems with ray-grid basis-vector generation.
//
//    1.01 20-Nov-90  Hollasch
//         Fixed ray-triangle intersection; moved some variable computations
//         from on-the-fly to precomputed fields in structures for both
//         2D triangles and tetrahedrons.
//
//    1.00 15-Nov-90  Hollasch
//         Added tetrahedron and triangle code.
//
//    0.02 24-Oct-90  Hollasch
//         Changed the aspect ratio definition.
//
//    0.01 21-Oct-90  Steve R. Hollasch
//         Initial version.
//
****************************************************************************/

#include <stdio.h>
#include <unix.h>

#define  DEFINE_GLOBALS
#include "ray4.h"
#include "r4_globals.h"
#include "r4_image.h"


   /*********************/
   /***  Debug Flags  ***/
   /*********************/

#define  DB_GRID     0		/* Grid Generation Output */
#define  DB_OPTIONS  0		/* Print Command Line Option Info */
#define  DB_SCENE    0		/* Print Scene Information */


   /************************/
   /***  Usage Messages  ***/
   /************************/

char notice[] = "\
\n\
       Ray4 [4D Raytracer]        Version 1.25          27-May-1991\n\
       Steve R. Hollasch                              (C) 1990-1991\n\
\n\n";

char usage[] = "\
ray4 :  4-Space Ray Tracer\n\
usage:  ray4 -s<Scan Range> -a<Aspect Ratios> -r<Image Resolution>\n\
             -b<Bits Per Pixel> -i<Input File> -o<Output File>\n\
\n\
    The arguments to the -s, -a, and -r options are all colon-separated\n\
fields of X:Y:Z triples.  All components are unsigned 16-bit integers.\n\
The aspect ratios are the width, height & depth (in some integer units) of\n\
a square voxel.  The scan range fields may consist of a range with a low\n\
low number, then a '-', and then a high number.  You can specify the\n\
entire range with a single underscore character.  You may select 12 or 24\n\
color bits per pixel (default is 24).\n\
\n\
    Examples:  ray4 -b12 -a2:1:2 -s_:_:_ -r256:256:256 <MyFile -omy.img\n\
               ray4 -a 1:1 -r 1024:768 -s 0-1023:0-767 -iSphere4 -os2.img\n\
";

   /******************************/
   /***  Constant Definitions  ***/
   /******************************/

#define MIN_SLB_COUNT  5	/* Minimum Scanline Buffer Count */
#define MIN_SLB_SIZE   (5<<10)	/* Minimum Scanline Buffer Size */


   /*************************************/
   /***  Local Function Declarations  ***/
   /*************************************/

void  ProcessArgs  ARGS((int, char**));
char *GetField     ARGS((char*, value));
char *GetRange     ARGS((char*, ushort*, ushort*));
void  PrintScene   ARGS((void));
void  CalcRayGrid  ARGS((void));
void  FireRays     ARGS((void));


   /*******************************/
   /***  File-Global Variables  ***/
   /*******************************/

ImageHdr  iheader =		/* Output Image Header */
   { 'Ray4', 1, 24, {1,1,1}, {0,0,0}, {0xFFFF,0xFFFF,0xFFFF} };

Vector4   Gx,  Gy,  Gz;		/* Ray-Grid Basis Vectors */
Point4    Gorigin;		/* Ray-Grid Origin Point  */
ushort    res[3] = {0,0,0};	/* Full Output Image Resolution */
ulong     scanlsize;		/* Scanline Size */
ulong     slbuff_count;		/* Number of Lines in Scanline Buffer */
char     *scanbuff;		/* Scanline Buffer */
ulong     StartTime;		/* Timestamp */



/****************************************************************************
**  The following is the entry procedure for the ray4 ray tracer.
****************************************************************************/

void  main  (argc, argv)
   int   argc;
   char *argv[];
{
   print (notice);

   ProcessArgs (argc, argv);

   OpenInput  ();
   ParseInput ();

   /* If the global ambient factor is zero, then clear all of the ambient
   // factor flags in the objects. */

   if ((ambient.r + ambient.g + ambient.b) < EPSILON)
   {  register   ObjInfo *optr;      /* Object Pointer */
      for (optr=objlist;  optr;  optr=optr->next)
	 optr->flags &= ~AT_AMBIENT;
   }

   /* Open the output stream and write out the image header (to be followed
   // by the generated scanline data. */

   OpenOutput ();
   WriteBlock (&iheader, (ulong) sizeof(iheader));

   /* Determine the size of a single scanline. */

   scanlsize = (3 * (1 + iheader.last[0] - iheader.first[0]));

   if (iheader.bitsperpixel == 12)
   {  if (scanlsize & 1)
	 ++scanlsize;
      scanlsize >>= 1;
   }

   /* Compute the number of scanlines and size of the scanline buffer that
   // meets the parameters MIN_SLB_COUNT and MIN_SLB_SIZE.  */

   if ((MIN_SLB_SIZE / scanlsize) > MIN_SLB_COUNT)
      slbuff_count = MIN_SLB_SIZE / scanlsize;
   else
      slbuff_count = MIN_SLB_COUNT;

   scanbuff = NEW (char, scanlsize * slbuff_count);

#  if (DB_SCENE)
      PrintScene ();
#  endif

   CalcRayGrid ();   /* Calculate the grid cube to fire rays through. */

   StartTime = time (nil);
   FireRays ();      /* Raytrace the scene. */

   Halt (nil, 0);      /* Clean up and exit. */
}



/****************************************************************************
**  This subroutine grabs the command-line arguments and the environment
**  variable arguments (from RAY4) and sets up the raytrace parameters.
****************************************************************************/

void  ProcessArgs  (argc, argv)
   int   argc;
   char *argv[];
{
   register char   *ptr;	/* Scratch String Pointer */
   register char   *eptr;	/* Environment Variable Pointer */
   register ulong   ii;		/* Option Array Index */
   register char  **opta;	/* Option Argument Array */
   register int     optc;	/* Option Argument Count */


   if (!(eptr = getenv ("RAY4")))
   {  optc = argc - 1;
      opta = NEW (char*, optc);
      for (ii=0;  ii < optc;  ++ii)
	 opta[ii] = argv[ii+1];
   }
   else
   {
      register int opti;   /* Option Index */

#     define SPACE(c)   ((c == ' ') || (c == '\t'))

      for (optc=0, ptr=eptr;  *ptr;  )
      {  while (SPACE(*ptr))  ++ptr;
	 if (!*ptr)  break;
	 ++optc;
	 while (++ptr, *ptr && !SPACE(*ptr))
	    ;
      }

      optc += argc - 1;
      opta = NEW (char*, optc);

      for (opti=0, ptr=eptr;  *ptr;  )
      {  while (SPACE(*ptr))  ++ptr;
	 if (!*ptr)  break;
	 opta[opti++] = ptr;
	 while (++ptr, *ptr && !SPACE(*ptr))
	    ;
	 if (*ptr)  *ptr++ = 0;
      }

      for (ii=1;  ii < argc;  ++ii)
	 opta[opti++] = argv[ii];

   }

   for (ii=0;  ii < optc;  ++ii)
   {
      auto   char  oc;      /* Option Character */

      if (opta[ii][0] != '-')
      {  printf ("ray4:  Unexpected argument (%s).\n", opta[ii]);
	 print  (usage);
	 exit (1);
      }

      oc = opta[ii][1];

      if (opta[ii][2])
	 ptr = opta[ii]+2;
      else
	 ptr = opta[++ii];

      switch (oc)
      {
	 case 'a':
	 {  if (ptr = GetField(ptr,&iheader.aspect[0]), (!ptr || !*ptr))
	       Halt ("Invalid X argument for -a option.",0);

	    if (ptr = GetField(ptr,&iheader.aspect[1]), !ptr)
	       Halt ("Invalid Y argument for -a option.",0);

	    if (ptr = GetField(ptr,&iheader.aspect[2]), !ptr)
	       Halt ("Invalid Z argument for -a option.",0);

	    break;
	 }

	 case 'b':
	 {  iheader.bitsperpixel = atoi (ptr);
	    if ((iheader.bitsperpixel != 12) && (iheader.bitsperpixel != 24))
	    {  printf ("r4toiff:  %d bits per pixel is not supported (select 12 or 24).\n", iheader.bitsperpixel);
	       iheader.bitsperpixel = 24;
	    }

	    break;
	 }

	 case 'i':
	 {  if (infile)
	       DELETE(infile);
	    infile = NEW (char, strsize(ptr));
	    strcpy (infile, ptr);
	    break;
	 }

	 case 'o':
	 {  if (outfile)
	       DELETE(outfile);
	    outfile = NEW (char, strsize(ptr));
	    strcpy (outfile, ptr);
	    break;
	 }

	 case 'r':
	 {  if (ptr = GetField(ptr,&res[X]), (!ptr || !*ptr))
	       Halt ("Invalid X argument for -r option.",0);

	    if (ptr = GetField(ptr,&res[Y]), !ptr)
	       Halt ("Invalid Y argument for -r option.",0);

	    if (ptr = GetField(ptr,&res[Z]), !ptr)
	       Halt ("Invalid Z argument for -r option.",0);

	    break;
	 }

	 case 's':
	 {  ptr = GetRange(ptr,&iheader.first[0],&iheader.last[0]);
	    if (!ptr || !*ptr)
	       Halt ("Bad X field argument to -s option.",0);

	    ptr = GetRange(ptr,&iheader.first[1],&iheader.last[1]);
	    if (!ptr)
	       Halt ("Bad Y field argument to -s option.",0);

	    ptr = GetRange(ptr,&iheader.first[2],&iheader.last[2]);
	    if (!ptr)
	       Halt ("Bad Z field argument to -s option.",0);

	    break;
	 }

	 default:
	 {  printf ("ray4:  Unknown option (-%c).\n", oc);
	    print  (usage);
	    exit (1);
	 }
      }
   }

   DELETE (opta);

   if ((iheader.aspect[0] == 0) || (iheader.aspect[1] == 0))
      Halt ("X and Y aspect ratios must be non-zero.",0);

   if ((res[0] == 0) || (res[1] == 0))
      Halt ("X and Y resolution must be non-zero.",0);

   if (res[2] == 0)  res[2] = 1;

   if (  (iheader.first[0] >  iheader.last[0])
      || (iheader.first[1] >  iheader.last[1])
      || (iheader.first[2] >  iheader.last[2])
      || (iheader.first[0] >= res[0])
      || (iheader.first[1] >= res[1])
      || (iheader.first[2] >= res[2])
      )
      Halt ("Invalid scan range given.",0);

   if (iheader.last[0] >= res[0])  iheader.last[0] = res[0]-1;
   if (iheader.last[1] >= res[1])  iheader.last[1] = res[1]-1;
   if (iheader.last[2] >= res[2])  iheader.last[2] = res[2]-1;

#  if (DB_OPTIONS)
   {  printf ("resolution  = %04u : %04u : %04u\n", V3_List(res));
      printf ("aspect      = %04u : %04u : %04u\n", V4_List(iheader.aspect));
      printf ("scan range  = %04u-%04u : %04u-%04u : %04u-%04u\n\n",
	 iheader.first[0], iheader.last[0],
	 iheader.first[1], iheader.last[1],
	 iheader.first[2], iheader.last[2]);
      printf ("input  file = %s\n", infile ? infile : "stdin");
      printf ("output file = %s\n", outfile);
   }
#  endif
}



/****************************************************************************
**  These subroutine process the command-line arguments.  The first two
**  routines get each field of the resolution, aspect ratio, and scan range
**  triples.
****************************************************************************/

char *GetField  (str, value)
   char   *str;		/* Source String */
   ushort *value;	/* Destination Value */
{
   if (!str)   return nil;
   if (!*str)   return *value=0, str;

   if ((*str < '0') || ('9' < *str))
      return nil;

   *value = atoi (str);

   while (('0' <= *str) && (*str <= '9'))
      ++str;

   return (*str == ':') ? (str+1) : str;
}

/*******************************/

char *GetRange  (str, val1, val2)
   char   *str;		/* Source String */
   ushort *val1;	/* First Destination Value of Range */
   ushort *val2;	/* Second Destination Value of Range */
{
   if (!str)   return nil;

   if (!*str)
   {  *val1 = *val2 = 0;
      return str;
   }

   if (*str == '_')
   {  *val1 = 0;
      *val2 = 0xFFFF;
      return (str[1] == ':') ? (str+2) : (str+1);
   }

   if ((*str < '0') || ('9' < *str))  return nil;

   *val1 = *val2 = atoi (str);

   while (('0' <= *str) && (*str <= '9'))
      ++str;

   if (*str == 0)      return str;
   if (*str == ':')   return str+1;
   if (*str != '-')   return nil;

   ++str;
   if ((*str < '0') || ('9' < *str))   return nil;

   *val2 = atoi (str);
   while (('0' <= *str) && (*str <= '9'))
      ++str;

   return (*str == ':') ? (str+1) : str;
}



/****************************************************************************
**  This procedure replaces printf() to print out an error message, and has
**  the side effect of cleaning up before exiting (de-allocating memory,
**  closing open files, and so on).
****************************************************************************/

void  Halt  (message, arg1)
   char *message;	/* Exception Message */
   char *arg1;		/* Optional Message Arguments */
{
   static   Attributes *aptr;	/* Attributes-List Pointer */
   static   Light      *lptr;	/* Light-List Pointer */
   static   ObjInfo    *optr;	/* Object-List Pointer */

   print ("\n");

   if (message)
   {  print  ("Ray4:  ");
      printf (message, arg1);
      print  ("\n\n");
   }

   CloseInput ();
   CloseOutput();

   if (infile)    DELETE (infile);
   if (outfile)   DELETE (outfile);
   if (scanbuff)  DELETE (scanbuff);

   while (lptr = lightlist)		/* Free the lightsource list. */
   {  lightlist = lightlist->next;
      DELETE (lptr);
   }

   while (optr = objlist)		/* Free the object list. */
   {  objlist = objlist->next;
      DELETE (optr);
   }

   while (aptr = attrlist)		/* Free the attribute list. */
   {  attrlist = attrlist->next;
      DELETE (aptr);
   }

   if (!message)
   {
      auto long  elapsed, hours, minutes, seconds;

      print  ("\n");
      printf ("       Total rays cast:  %lu\n", stats.Ncast);
      printf ("  Reflection rays cast:  %lu\n", stats.Nreflect);
      printf ("  Refraction rays cast:  %lu\n", stats.Nrefract);
      printf ("Maximum raytrace level:  %lu\n", stats.maxlevel);

      elapsed = time(nil) - StartTime;
      hours   = elapsed / 3600;
      minutes = (elapsed - 3600*hours) / 60;
      seconds = (elapsed - 3600*hours - 60*minutes);

      printf ("    Total Elapsed Time:  %ld seconds / %02ld:%02ld:%02ld\n",
	 elapsed, hours, minutes, seconds);
   }

   exit (!message) ? 0 : 1;
}



/****************************************************************************
//  This routine allocates memory using the system malloc() function.  If the
//  malloc() call fails to allocate the memory, this routine halts the
//  program with an "out of memory" message.
****************************************************************************/

char  *MyAlloc  (size)
   unsigned long  size;		/* Number of Bytes to Allocate */
{
   static   char *block;	/* Allocated Memory Block */

   if ( ! (block = malloc ((int)size)))
      Halt ("Out of memory.",0);

   return block;
}


void  MyFree  (addr)
   char *addr;		/* Address of Block to Free */
{
   free (addr);
}



#if (DB_SCENE)
/****************************************************************************
//  This routine dumps the object list for debugging.
****************************************************************************/

void  PrintScene  ()
{
   auto   Attributes *aptr;	/* Attributes Pointer */
   auto   Light      *lptr;	/* Light Pointer */
   auto   ObjInfo    *optr;	/* Object Pointer */

   printf ("sizeof(Attributes)     is %d\n", sizeof(Attributes));
   printf ("sizeof(Light)          is %d\n", sizeof(Light));
   printf ("sizeof(ObjInfo)        is %d\n", sizeof(ObjInfo));
   printf ("sizeof(Sphere)         is %d\n", sizeof(Sphere));
   printf ("sizeof(Tetrahedron)    is %d\n", sizeof(Tetrahedron));
   printf ("sizeof(Parallelepiped) is %d\n", sizeof(Parallelepiped));
   printf ("sizeof(Triangle)       is %d\n", sizeof(Triangle));
   print  ("\n");

   printf ("Scanline Buffer Count:  %lu\n", slbuff_count);
   printf ("Scanline Buffer Size :  %lu\n", slbuff_count * scanlsize);

   printf ("Global Ambient Light      :  %6.4lf, %6.4lf, %6.4lf\n",
      Color_List(ambient));
   printf ("Global Background Color   :  %6.4lf, %6.4lf, %6.4lf\n",
      Color_List(background));
   printf ("Global Index of Refraction:  %lg\n", global_indexref);
   printf ("Maximum Raytrace Depth    :  %u\n", maxdepth);
   print  ("\n");

   printf ("View From : <%6.4lf %6.4lf %6.4lf %6.4lf>\n", V4_List(Vfrom));
   printf ("View To   : <%6.4lf %6.4lf %6.4lf %6.4lf>\n", V4_List(Vto));
   printf ("View Up   : <%6.4lf %6.4lf %6.4lf %6.4lf>\n", V4_List(Vup));
   printf ("View Over : <%6.4lf %6.4lf %6.4lf %6.4lf>\n", V4_List(Vover));
   printf ("view Angle: %6.4lf\n", Vangle);
   print  ("\n");

   print  ("=== Lights ===\n");
   for (lptr=lightlist;  lptr;  lptr=lptr->next)
   {  printf ("\n    Color [%6.4lf, %6.4lf, %6.4lf]\n",
	 Color_List(lptr->color));
      if (lptr->type == L_POINT)
	 printf ("    Point @ %6.4lf, %6.4lf, %6.4lf, %6.4lf\n",
	    V4_List(lptr->u.pos));
      else if (lptr->type == L_DIRECTIONAL)
	 printf ("    Directional:  %6.4lf, %6.4lf, %6.4lf, %6.4lf\n",
	    V4_List(lptr->u.dir));
      else
	 printf ("    ERROR:  light.type == 0x%08X\n", lptr->type);
   }

   print  ("\n=== Attributes ===\n");
   for (aptr=attrlist;  aptr;  aptr=aptr->next)
   {  printf ("\nAttribute %08lx\n", aptr);
      printf ("    Ambient     : %6.4lf %6.4lf %6.4lf\n",
	 Color_List(aptr->Ka));
      printf ("    Diffuse     : %6.4lf %6.4lf %6.4lf\n",
	 Color_List(aptr->Kd));
      printf ("    Specular    : %6.4lf %6.4lf %6.4lf\n",
	 Color_List(aptr->Ks));
      printf ("    Transparency: %6.4lf %6.4lf %6.4lf\n",
	 Color_List(aptr->Kt));
      printf ("    Shine       : %lg\n",  aptr->shine);
      printf ("    IndexRefract: %lg\n",  aptr->indexref);
      printf ("    Flags       : %02x\n", aptr->flags);
   }

   print  ("\n=== Objects ===\n");
   for (optr=objlist;  optr;  optr=optr->next)
   {
      print  ("\n");
      printf ("    Attributes  : %08lx\n", optr->attr);
      printf ("    Flags       : %02x\n",  optr->flags);
      switch (optr->type)
      {
	 case O_SPHERE:
	    print  ("    Type        : Sphere\n");
	    printf ("    Center      : %6.4lf %6.4lf %6.4lf %6.4lf\n",
	       V4_List (((Sphere*)(optr))->center));
	    printf ("    Radius      : %lg (squared %lg)\n",
	       ((Sphere *)(optr))->radius, ((Sphere*)(optr))->rsqrd);
	    break;

	 case O_TETRAHEDRON:
	    print  ("    Type        : Tetrahedron\n");
	    printf ("    Vertex 0    : %6.4lf %6.4lf %6.4lf %6.4lf\n",
	       V4_List (((Tetrahedron*)(optr))->tp.vert[0]));
	    printf ("    Vertex 1    : %6.4lf %6.4lf %6.4lf %6.4lf\n",
	       V4_List (((Tetrahedron*)(optr))->tp.vert[1]));
	    printf ("    Vertex 2    : %6.4lf %6.4lf %6.4lf %6.4lf\n",
	       V4_List (((Tetrahedron*)(optr))->tp.vert[2]));
	    printf ("    Vertex 3    : %6.4lf %6.4lf %6.4lf %6.4lf\n",
	       V4_List (((Tetrahedron*)(optr))->tp.vert[3]));
	    printf ("    Normal      : %6.4lf %6.4lf %6.4lf %6.4lf\n",
	       V4_List (((Tetrahedron*)(optr))->tp.normal));
	    printf ("    Ax1,2,3     : %u %u %u\n",
	       ((Tetrahedron*)(optr))->tp.ax1,
	       ((Tetrahedron*)(optr))->tp.ax2,
	       ((Tetrahedron*)(optr))->tp.ax3);
	    printf ("    planeConst  : %6.4lf\n",
	       ((Tetrahedron*)(optr))->tp.planeConst);
	    printf ("    CramerDiv   : %6.4lf\n",
	       ((Tetrahedron*)(optr))->tp.CramerDiv);
	    printf ("    vec1        : %6.4lf %6.4lf %6.4lf %6.4lf\n",
	       V4_List (((Tetrahedron*)(optr))->tp.vec1));
	    printf ("    vec2        : %6.4lf %6.4lf %6.4lf %6.4lf\n",
	       V4_List (((Tetrahedron*)(optr))->tp.vec2));
	    printf ("    vec3        : %6.4lf %6.4lf %6.4lf %6.4lf\n",
	       V4_List (((Tetrahedron*)(optr))->tp.vec3));
	    break;


	 case O_PARALLELEPIPED:
	    print  ("    Type        : Parallelepiped\n");
	    printf ("    Vertex 0    : %6.4lf %6.4lf %6.4lf %6.4lf\n",
	       V4_List (((Parallelepiped*)(optr))->tp.vert[0]));
	    printf ("    Vertex 1    : %6.4lf %6.4lf %6.4lf %6.4lf\n",
	       V4_List (((Parallelepiped*)(optr))->tp.vert[1]));
	    printf ("    Vertex 2    : %6.4lf %6.4lf %6.4lf %6.4lf\n",
	       V4_List (((Parallelepiped*)(optr))->tp.vert[2]));
	    printf ("    Vertex 3    : %6.4lf %6.4lf %6.4lf %6.4lf\n",
	       V4_List (((Parallelepiped*)(optr))->tp.vert[3]));
	    printf ("    Normal      : %6.4lf %6.4lf %6.4lf %6.4lf\n",
	       V4_List (((Parallelepiped*)(optr))->tp.normal));
	    printf ("    Ax1,2,3     : %u %u %u\n",
	       ((Parallelepiped*)(optr))->tp.ax1,
	       ((Parallelepiped*)(optr))->tp.ax2,
	       ((Parallelepiped*)(optr))->tp.ax3);
	    printf ("    planeConst  : %6.4lf\n",
	       ((Parallelepiped*)(optr))->tp.planeConst);
	    printf ("    CramerDiv   : %6.4lf\n",
	       ((Parallelepiped*)(optr))->tp.CramerDiv);
	    printf ("    vec1        : %6.4lf %6.4lf %6.4lf %6.4lf\n",
	       V4_List (((Parallelepiped*)(optr))->tp.vec1));
	    printf ("    vec2        : %6.4lf %6.4lf %6.4lf %6.4lf\n",
	       V4_List (((Parallelepiped*)(optr))->tp.vec2));
	    printf ("    vec3        : %6.4lf %6.4lf %6.4lf %6.4lf\n",
	       V4_List (((Parallelepiped*)(optr))->tp.vec3));
	    break;

	 case O_TRIANGLE:
	    print  ("    Type        : Triangle\n");
	    printf ("    Vertex 0    : %6.4lf %6.4lf %6.4lf %6.4lf\n",
	       V4_List (((Triangle*)(optr))->vert[0]));
	    printf ("    Vertex 1    : %6.4lf %6.4lf %6.4lf %6.4lf\n",
	       V4_List (((Triangle*)(optr))->vert[1]));
	    printf ("    Vertex 2    : %6.4lf %6.4lf %6.4lf %6.4lf\n",
	       V4_List (((Triangle*)(optr))->vert[2]));
	    printf ("    vec1        : %6.4lf %6.4lf %6.4lf %6.4lf\n",
	       V4_List (((Triangle*)(optr))->vec1));
	    printf ("    vec2        : %6.4lf %6.4lf %6.4lf %6.4lf\n",
	       V4_List (((Triangle*)(optr))->vec2));
	    break;

	 default:
	    printf ("    ***> Object type unknown (%08X)\n", optr->type);
	    break;
      }
   }
   print ("\n\n");
}
#endif



/****************************************************************************
//  This procedure calculates the ray-grid basis vectors.
****************************************************************************/

void  CalcRayGrid  ()
{
   auto   Lfloat   GNx, GNy, GNz;	/* Ray-Grid Vector Norms */
   auto   Vector4  Los;			/* Line-of-Sight Vector */
   auto   Lfloat   losnorm;		/* Line-of-Sight-Vector Norm */

   /* Get the normalized line-of-sight vector. */

   V4_3Vec (Los,=,Vto,-,Vfrom);
   losnorm = V4_Norm (Los);

   if (losnorm < EPSILON)
      Halt ("To-Point & From-Point are the same.", 0);
   V4_Scalar (Los, /=, losnorm);

   /* Generate the normalized ray-grid basis vectors. */

   V4_Cross (Gz, Vover,Vup,Los);
   if (! V4_Normalize(Gz))
      Halt ("Line-of-sight, Up vector and Over vector aren't orthogonal.",0);

   V4_Cross (Gy, Gz,Los,Vover);
   if (! V4_Normalize(Gy))
      Halt ("Orthogonality problem while generating GRIDy.",0);

   V4_Cross (Gx, Gy,Gz,Los);      /* Gy, Gz & Los are all unit vectors. */

   /* Now compute the proper scale of the grid unit vectors. */

   GNx = 2.0 * losnorm * tan (DTOR*Vangle/2.0);

   GNy = GNx
       * ((Lfloat) res[Y] / (Lfloat) res[X])
       * ((Lfloat)iheader.aspect[Y]/ (Lfloat)iheader.aspect[X]);

   GNz = GNx
       * ((Lfloat) res[Z] / (Lfloat) res[X])
       * ((Lfloat)iheader.aspect[Z]/ (Lfloat)iheader.aspect[X]);

   /* Scale each grid basis vector. */

   V4_Scalar (Gx, *=, GNx);
   V4_Scalar (Gy, *=, GNy);
   V4_Scalar (Gz, *=, GNz);

   /* Find the ray-grid origin point. */

   Gorigin[0] = Vto[0] - (Gx[0]/2.0) - (Gy[0]/2.0) - (Gz[0]/2.0);
   Gorigin[1] = Vto[1] - (Gx[1]/2.0) - (Gy[1]/2.0) - (Gz[1]/2.0);
   Gorigin[2] = Vto[2] - (Gx[2]/2.0) - (Gy[2]/2.0) - (Gz[2]/2.0);
   Gorigin[3] = Vto[3] - (Gx[3]/2.0) - (Gy[3]/2.0) - (Gz[3]/2.0);

   /* Finally, scale the grid basis vectors down by the corresponding
   ** resolutions.  */

   V4_Scalar (Gx, /=, res[X]);
   V4_Scalar (Gy, /=, res[Y]);
   V4_Scalar (Gz, /=, res[Z]);

   Gorigin[0] += (Gx[0]/2.0) + (Gy[0]/2.0) + (Gz[0]/2.0);
   Gorigin[1] += (Gx[1]/2.0) + (Gy[1]/2.0) + (Gz[1]/2.0);
   Gorigin[2] += (Gx[2]/2.0) + (Gy[2]/2.0) + (Gz[2]/2.0);
   Gorigin[3] += (Gx[3]/2.0) + (Gy[3]/2.0) + (Gz[3]/2.0);

#  if (DB_GRID)
      printf ("Gorigin <%6.4lf %6.4lf %6.4lf %6.4lf>\n", V4_List(Gorigin));
      printf ("Gx      <%6.4lf %6.4lf %6.4lf %6.4lf>\n", V4_List(Gx));
      printf ("Gy      <%6.4lf %6.4lf %6.4lf %6.4lf>\n", V4_List(Gy));
      printf ("Gz      <%6.4lf %6.4lf %6.4lf %6.4lf>\n", V4_List(Gz));
      print  ("\n");
#  endif
}



/****************************************************************************
//  This is the main routine that fires the rays through the ray grid and
//  into the 4D scene.
****************************************************************************/

void  FireRays  ()
{
   static boolean  eflag;			/* Even RGB Boundary Flag */
   static ulong    scancount;			/* Scanline Counter */
   static char    *scanptr;			/* Scanline Buffer Pointer */
   static ushort   Xindex, Yindex, Zindex;	/* Ray-Grid Loop Indices */
   static Point4   Yorigin, Zorigin;		/* Ray-Grid Axis Origins */

   scancount = 0;
   scanptr   = scanbuff;
   eflag     = true;

   for (Zindex=iheader.first[Z];  Zindex <= iheader.last[Z];  ++Zindex)
   {
      V4_3Vec (Zorigin, =, Gorigin, +, Zindex*Gz);
      for (Yindex=iheader.first[Y];  Yindex <= iheader.last[Y];  ++Yindex)
      {
	 printf ("%6u %6u\r",
	    iheader.last[Z] - Zindex, iheader.last[Y] - Yindex);
	 fflush (stdout);

	 V4_3Vec (Yorigin, =, Zorigin, +, Yindex*Gy);

	 if (!eflag)
	 {  ++scanptr;
	    eflag = true;
	 }

	 for (Xindex=iheader.first[X];  Xindex <= iheader.last[X];  ++Xindex)
	 {
	    static Color    color;	/* Pixel Color */
	    static Vector4  Dir;	/* Ray Direction Vector */
	    static Point4   Gpoint;	/* Current Grid Point */
	    static Lfloat   norm;	/* Vector Norm Value */

	    /* Calculate the unit ViewFrom-RayDirection vector. */

	    V4_3Vec (Gpoint, =, Yorigin, +, Xindex*Gx);
	    V4_3Vec (Dir, =, Gpoint, -, Vfrom);
	    norm = V4_Norm (Dir);
	    V4_Scalar (Dir, /=, norm);

	    /* Fire the ray. */

	    RayTrace (Vfrom, Dir, &color, (ulong)(0));

	    /* Scale the resulting color to 0-255. */

	    Color_Scale (color, *=, 256.0);
	    color.r = CLAMP (color.r, 0.0, 255.0);
	    color.g = CLAMP (color.g, 0.0, 255.0);
	    color.b = CLAMP (color.b, 0.0, 255.0);

	    /* Store the 24-bit RGB triple in the scanline buffer. */

	    if (iheader.bitsperpixel == 24)
	    {  *scanptr++ = (uchar)(color.r);
	       *scanptr++ = (uchar)(color.g);
	       *scanptr++ = (uchar)(color.b);
	    }
	    else if (eflag)
	    {  *scanptr++ = ((uchar)(color.r) & 0xF0)
			  | ((uchar)(color.g) >> 4);
	       *scanptr   = ((uchar)(color.b) & 0xF0);
	       eflag = false;
	    }
	    else
	    {  *scanptr++ |= ((uchar)(color.r) >> 4);
	       *scanptr++  = ((uchar)(color.g) & 0xF0)
			   | ((uchar)(color.b) >> 4);
	       eflag = true;
	    }
	 }

	 /* If the scanline output buffer is full now, write it to disk. */

	 if (++scancount >= slbuff_count)
	 {  scancount = 0;
	    scanptr   = scanbuff;
	    eflag     = true;
	    WriteBlock (scanbuff, scanlsize * slbuff_count);
	 }
      }
   }

   /* If there are scanlines in the scanline buffer, then write the
   ** remaining scanlines to disk.  */

   if (scancount != 0)
      WriteBlock (scanbuff, scanlsize * scancount);
}
