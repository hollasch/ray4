/****************************************************************************
//
//  File:  r4_parse.c
//
//      This file contains procedures that are used to parse the input file
//    and build the scene for the raytrace.
//
//  Revisions:
//
//    1.20  01-Jan-91  Hollasch
//          Changed source file format to use enclosing parentheses for
//          directive fields, and added named and immediate attributes and
//          the attribute name list.  I also changed the meaning of the
//          direction vector given for directional light sources:  the
//          vector in the source file now specifies the direction _towards_
//          the directional light source.
//
//    1.10  19-Dec-90  Hollasch
//          Added parallelepiped objects.
//
//    1.03  13-Dec-90  Hollasch
//          Added code to enable object reflections and added internal
//          verification code for debugging tetrahedra.
//
//    1.00  15-Nov-90  Hollasch
//          Added triangle parsing code.
//
//    0.00  30-Sep-90  Steve R. Hollasch
//          Initial version.
//
****************************************************************************/

#include <stdio.h>

#include "ray4.h"
#include "r4_globals.h"

	/***  Debug Switches  ***

	/*  0: No debug
	//  1: Quiet Consistency Checking Enabled
	//  2: Debug Output Enabled
	*/

#define  DB_TETPAR   0	/* Debug Tetrahedron / Parallelepiped */
#define  DB_ATNAME   0	/* Display Attribute Name List */


	/***  Defined Constants  ***/

#define MAXTLEN    80	/* Maximum Token Length in Input File */
#define KEYSIG     5	/* Significant Chars of a Keyword (max 255) */
#define MAXATNAME  15	/* Maximum Attribute Name Length */

#define ERR ((char)(0))		/* Erroneous Character */
#define SPC ((char)(1))		/* Whitespace Character */
#define WRD ((char)(2))		/* Keyword Character */
#define NUM ((char)(3))		/* Number Character */
#define OTH ((char)(4))		/* Other Character */

#define BLACK  { 0.000, 0.000, 0.000 }		/* Color Black */


	/***  Function Declarations  ***/

void        DoAttributes     ARGS((void));
void        DoLight          ARGS((void));
void        DoSphere         ARGS((void));
void        DoTetrahedron    ARGS((void));
void        DoParallelepiped ARGS((void));
void        DoTriangle       ARGS((void));
void        DoView           ARGS((void));
void        Error            ARGS((char *, char *));
Attributes *FindAttributes   ARGS((char *));
char       *GetToken         ARGS((char *, boolean));
boolean     keyeq            ARGS((char *, char *));
void        ParseInput       ARGS((void));
void        Process_TetPar   ARGS((TetPar *));
Attributes *ReadAttributes   ARGS((void));
void        ReadColor        ARGS((char *, Color *));
void        ReadReal         ARGS((char *, Lfloat *));
void        ReadUint16       ARGS((char *, ushort *));
void        Read4Vec         ARGS((char *, Vector4));


	/* The following array is used to determine character types. */

char chtype[256] =
{  ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR, ERR,SPC,SPC,SPC, SPC,SPC,ERR,ERR,/* 0 */
   ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR,/* 1 */
   SPC,OTH,OTH,OTH, OTH,OTH,OTH,OTH, OTH,OTH,OTH,OTH, SPC,NUM,NUM,OTH,/* 2 */
   NUM,NUM,NUM,NUM, NUM,NUM,NUM,NUM, NUM,NUM,OTH,OTH, OTH,OTH,OTH,OTH,/* 3 */
   OTH,WRD,WRD,WRD, WRD,WRD,WRD,WRD, WRD,WRD,WRD,WRD, WRD,WRD,WRD,WRD,/* 4 */
   WRD,WRD,WRD,WRD, WRD,WRD,WRD,WRD, WRD,WRD,WRD,SPC, OTH,SPC,OTH,WRD,/* 5 */
   OTH,WRD,WRD,WRD, WRD,WRD,WRD,WRD, WRD,WRD,WRD,WRD, WRD,WRD,WRD,WRD,/* 6 */
   WRD,WRD,WRD,WRD, WRD,WRD,WRD,WRD, WRD,WRD,WRD,SPC, OTH,SPC,OTH,OTH,/* 7 */

   ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR,/* 8 */
   ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR,/* 9 */
   ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR,/* A */
   ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR,/* B */
   ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR,/* C */
   ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR,/* D */
   ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR,/* E */
   ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR /* F */
};


typedef enum { T_OTHER, T_VEC4, T_UINT16, T_REAL, T_COLOR } VarType;

struct
{  char     keyword[KEYSIG+1];
   VarType  vtype;
   char    *address;
} Globals[]
=
{
   { "attri", T_OTHER,  (char*)  DoAttributes    },
   { "ambie", T_COLOR,  (char*) &ambient         },
   { "backg", T_COLOR,  (char*) &background      },
   { "index", T_REAL,   (char*) &global_indexref },
   { "light", T_OTHER,  (char*)  DoLight         },
   { "maxde", T_UINT16, (char*) &maxdepth        },
   { "paral", T_OTHER,  (char*)  DoParallelepiped},
   { "spher", T_OTHER,  (char*)  DoSphere        },
   { "tetra", T_OTHER,  (char*)  DoTetrahedron   },
   { "trian", T_OTHER,  (char*)  DoTriangle      },
   { "view",  T_OTHER,  (char*)  DoView          }
};


typedef struct S_ATTRNAME  AttrName;

struct S_ATTRNAME
{  AttrName   *next;			/* Next Attribute Name Node */
   char        name[MAXATNAME+1];	/* Attribute Name */
   Attributes *attr;			/* Attributes */
};


   /*** Default Structures ***/

Light DefLight =
{  nil,				/* Next Light Source */
   { 1.0, 1.0, 1.0 },		/* Light Color */
   L_DIRECTIONAL		/* Light Type */
};

Attributes DefAttributes =
{  nil,				/* Next Attribute Node */
   BLACK, BLACK, BLACK, BLACK,	/* Ambient, Diffuse, Specular, Transpar. */
   1.0,				/* Shine */
   1.0,				/* Index of Refraction */
   0				/* Attribute Flags */
};

Sphere DefSphere =
{  {  nil,			/* Next Pointer */
      nil,			/* Attributes */
      O_SPHERE,			/* Object Type */
      0,			/* Object Flags */
      HitSphere			/* Sphere-Intersection Function */
   }
};

Tetrahedron DefTetra =
{  {  nil,			/* Next Pointer */
      nil,			/* Attributes */
      O_TETRAHEDRON,		/* Object Type */
      0,			/* Object Flags */
      HitTetPar			/* Tetrahedron-Intersection Function */
   }
};

Parallelepiped DefPllp =
{  {  nil,			/* Next Pointer */
      nil,			/* Attributes */
      O_PARALLELEPIPED,		/* Object Type */
      0,			/* Object Flags */
      HitTetPar			/* Parallelepiped-Intersection Function */
   }
};

Triangle DefTriangle =
{  {  nil,			/* Next Pointer */
      nil,			/* Attributes */
      O_TRIANGLE,		/* Object Type */
      0,			/* Object Flags */
      HitTriangle		/* Triangle-Intersection Function */
   }
};


   /*** Global Variables ***/

static AttrName   *attrnamelist = nil;	    /* Attribute Name List */
static boolean     eofflag = false;	    /* Non-Zero If EOF Input File */
static ulong       lcount = 1;		    /* Input Line Counter */
static Attributes *prevattr= &DefAttributes;/* Previously Named Attribute */
static char        token[MAXTLEN+1];	    /* Input Token */



/****************************************************************************
**  This routine parses the input scene description, and sets up the global
**  raytrace variables and the object lists.
****************************************************************************/

void  ParseInput  ()
{
   auto     AttrName   *attrname;	/* Attribute Name Node Pointer */
   register ushort      ii;		/* Scratch Index Value */
   register boolean   (*func)();	/* Function Pointer */

   while (GetToken(token, true))
   {
      for (ii=0;  ii < ALIMIT(Globals);  ++ii)
      {
         if (keyeq(token, Globals[ii].keyword))
            break;
      }

      if (ii >= ALIMIT(Globals))
         Error ("Unknown keyword (%s).", token);

      switch (Globals[ii].vtype)
      {
         case T_COLOR:
            ReadColor (token, (Color*)(Globals[ii].address));
            break;

         case T_UINT16:
            ReadUint16 (token, (ushort*)(Globals[ii].address));
            break;

         case T_REAL:
            ReadReal (token, (Lfloat*)(Globals[ii].address));
            break;

         case T_OTHER:
            func = (boolean(*)())(Globals[ii].address);
            (*func)();
            break;

         default:
            Halt ("Internal Error (Globals vtype switch).");
      }
   }

#  if DB_ATNAME
      print ("\n=== Attribute Name List ===\n");
      for (attrname=attrnamelist;  attrname;  attrname = attrname->next)
         printf ("%08lx  %s\n", attrname->attr, attrname->name);
      print ("\n");
#  endif

   /* Kill the attributes alias list. */

   while (attrname=attrnamelist, attrname)
   {  attrnamelist = attrname->next;
      DELETE (attrname);
   }
}



/****************************************************************************
**  This routine reads the input stream and returns the next token.  It's
**  basically the lexical analyzer.  It returns the destination buffer given
**  as the single parameter.
****************************************************************************/

char *GetToken  (buff, eofok)
   char    *buff;	/* Token Buffer */
   boolean  eofok;	/* If 1, EOF is OK, otherwise error */
{
   static   char    cc = ' ';	/* Input Character */
   register ushort  nn;		/* Length of Destination String */
   register char   *ptr;	/* Destination Buffer Pointer */

   if (cc == -1)
   {  eofflag = true;
      if (eofok)  return nil;
      Error ("Unexpected end-of-file.");
   }

   ptr = buff;
   nn  = 0;

   /* Skip past comments and whitespace. */

   for (;;)
   {  
      if (cc == '>')
      {  while (cc=ReadChar(), (cc != '\n') && (cc != '\r'))
         {  if (cc == -1)
            {  eofflag = true;
               if (eofok)  return nil;
               Error ("Unexpected end-of-file.");
            }
         }
      }

      while (chtype[cc] == SPC)
      {
         if (cc == '\n')  ++lcount;

         if (-1 == (cc = ReadChar ()))
         {  eofflag = true;
            if (eofok)  return nil;
            Error ("Unexpected end-of-file.");
         }
      }

      if (cc != '>')
         break;
   }

   /* If the next character is a punctuation character, then just return
   // that character as a single token. */

   if (chtype[(unsigned char)(cc)] == OTH)
   {  *ptr++ = cc;
      *ptr   = 0;
      cc     = ReadChar ();
      return buff;
   }

   /* If the next character starts an alphanumeric word, then read the word.
   // Words begin with an alphabetic character or an underbar, and may also
   // contain numbers, periods and dashes in the body. */

   if (chtype[cc] == WRD)
   {
      for (;;)
      {  if (++nn > MAXTLEN)
         {  *ptr = 0;
            Error ("Input token is too long (%s).", buff);
         }
         *ptr++ = cc;

         cc = ReadChar ();
         if ((cc == -1) || ((chtype[cc] != WRD) && (chtype[cc] != NUM)))
            break;
      }

      *ptr = 0;
      if (buff[0])  return buff;

      eofflag = true;
      if (eofok)  return nil;
      Error ("Unexpected end-of-file.");
   }

   if (chtype[cc] == NUM)
   {
      static boolean  eflag;	/* True After 'e' Character Is Read */
      static boolean  dflag;	/* True After '.' Character Is Read */

      eflag = dflag = 0;

      if (cc == '.')   dflag = 1;

      for (;;)
      {  if (++nn > MAXTLEN)
         {  *ptr = 0;
            Error ("Input token is too long (%s).", buff);
         }
         *ptr++ = cc;

         if (-1 == (cc = ReadChar ()))
            break;

         if (cc == '.')
         {  if (dflag || eflag)
               break;
            else
            {  dflag = 1;
               continue;
            }
         }
         else if (cc == 'e')
         {  if (eflag)   break;
            else
            {  eflag = 1;
               continue;
            }
         }
         else if (cc == '-')
         {  if ((ptr[-1] | 0x20) != 'e')
               break;
            continue;
         }

         if (chtype[cc] != NUM)  break;
      }

      *ptr = 0;
      if (buff[0])
         return buff;

      eofflag = true;
      if (eofok)  return nil;
      Error ("Unexpected end-of-file.");
   }

   if ((cc < 0x20) || (cc > 0x7E))
      Error ("Unexpected character in input stream (0x%02x).", cc & 0xFF);
   else
      Error ("Unexpected character in input stream ('%c').", cc & 0xFF);
}



/****************************************************************************
//  This routine compares an input token with a keyword.  If the two strings
//  match up to the significant length, this routine returns 1, otherwise it
//  returns 0.  This routine assumes that the keyword is all lowercase
//  letters and that the string is uppercase or lowercase letters only.  This
//  will be true if GetToken() does its job correctly.  Furthermore if the
//  string matches the key until the string ends, then this function will
//  still return 1.  That is, keyeq("amb", "ambient") == 1.
****************************************************************************/

boolean  keyeq  (string, key)
   char *string;	/* Input Token */
   char *key;		/* First KEYSIG digits of keyword. */
{
   register ushort  ii;	/* String Index */

   for (ii=0;  (ii < KEYSIG) && key[ii] && string[ii];  ++ii)
      if (key[ii] != (string[ii] | 0x20))
         return false;

   return true;
}



/****************************************************************************
//  This procedure processes attribute definitions.  Attribute definitions
//  consist of the keyword `attributes', followed by an attribute alias,
//  and then the attribute fields.
****************************************************************************/

void  DoAttributes  ()
{
   auto AttrName *anptr;	/* Attributes Alias Node Pointer */
   auto AttrName *newattrname;	/* New Attributes Alias Node */

   /* Read in the attribute alias. */

   GetToken (token, false);
   if ((chtype[token[0]] != WRD) && (chtype[token[0]] != NUM))
      Error ("Invalid attribute name (%s)\n", token);

   token[MAXATNAME] = 0;

   /* Ensure that the name is not a duplicate of an earlier name. */

   for (anptr = attrnamelist;  anptr;  anptr = anptr->next)
      if (strrel(anptr->name, ==, token))
         break;

   if (anptr)
   {  printf ("Warning:  Attributes \"%s\" redefined at line %lu.\n",
         token, lcount);
      newattrname = anptr;
   }
   else
   {  newattrname = NEW (AttrName, 1);
      newattrname->next = attrnamelist;
      attrnamelist = newattrname;
      strcpy (newattrname->name, token);
   }

   /* Consume the opening parenthesis of the attribute description. */

   if (GetToken(token,false), token[0] != '(')
      Error ("Expected opening parenthesis (got '%s').", token);

   prevattr = newattrname->attr = ReadAttributes ();
}



/****************************************************************************
//  This routine reads in a description of a light source from the input
//  file.  At the calling point, the keyword "Light" has already been read.
//  The new light will be added to the light list.
****************************************************************************/

void  DoLight  ()
{
   static   Light *prev = &DefLight;	/* Previously Defined Light */
   register Light *light;		/* New Light Source */

   /* Gobble up the opening parenthesis. */

   if (GetToken(token,false), token[0] != '(')
      Error ("Missing opening parenthesis for light definition.", nil);

   light = NEW (Light,1);
   *light = *prev;

   while (GetToken(token,false), token[0] != ')')
   {
      if (keyeq (token, "color"))
         ReadColor (token, &light->color);
      else if (keyeq (token, "direc"))
      {  Read4Vec (token, light->u.dir);
         light->type = L_DIRECTIONAL;
      }
      else if (keyeq (token, "posit"))
      {  Read4Vec (token, light->u.pos);
         light->type = L_POINT;
      }
      else
         Error ("Invalid light subfield (%s).", token);
   }

   /* If the light is directional, then normalize the light direction. */

   if (light->type == L_DIRECTIONAL)
   {  if (! V4_Normalize (light->u.dir))
         Error ("Zero light direction vector.");
   }

   light->next = lightlist;
   lightlist = prev = light;
}



/****************************************************************************
//  This routine reads in a description of a hyperspherical object from the
//  input stream and adds it to the object list.  The field defaults are
//  defined by the DefSphere structure for the first sphere, and then by the
//  previous sphere.
****************************************************************************/

void  DoSphere  ()
{
   static   Sphere *prev = &DefSphere;	/* Previously Defined Sphere */
   register Sphere *snew;		/* New Sphere */

   /* Gobble up the opening parenthesis. */

   if (GetToken(token,false), token[0] != '(')
      Error ("Missing opening parenthesis for sphere definition.", nil);

   snew = NEW(Sphere,1);
   *snew = *prev;

   while (GetToken(token,false), token[0] != ')')
   {
      if (keyeq(token, "attri"))
      {  GetToken (token,false);
         snew->info.attr = (token[0] == '(') ? ReadAttributes()
                                             : FindAttributes(token);
      }
      else if (keyeq (token, "cente"))
         Read4Vec (token, snew->center);
      else if (keyeq (token, "radiu"))
         ReadReal (token, &snew->radius);
      else Error ("Invalid sphere subfield (%s).\n", token);
   }

   if (snew->radius < EPSILON)
      Error ("Sphere has non-positive radius.", nil);

   if (!snew->info.attr)
      Error ("Missing attributes for sphere description.", nil);

   snew->rsqrd = snew->radius * snew->radius;

   snew->info.next = objlist;
   objlist = (ObjInfo *)(prev = snew);
}



/****************************************************************************
//  This routine reads in a description of a 4D parallelepiped (defined by
//  four vertices) and adds it to the object list.
****************************************************************************/

void  DoParallelepiped ()
{
   static Parallelepiped *prev= &DefPllp; /* Previously Defined Tetrahedron*/
   register Parallelepiped *pnew;	  /* New Parallelepiped */

   /* Gobble up the opening parenthesis. */

   if (GetToken(token,false), token[0] != '(')
      Error ("Missing opening parenthesis for parallelepiped definition.",
         nil);

   pnew = NEW (Parallelepiped,1);
   *pnew = *prev;

   while (GetToken(token,false), token[0] != ')')
   {
      if (keyeq (token, "attri"))
      {  GetToken (token,false);
         pnew->info.attr = (token[0] == '(') ? ReadAttributes()
                                             : FindAttributes(token);
      }
      else if (keyeq (token, "verti"))
      {  Read4Vec (token, pnew->tp.vert[0]);
         Read4Vec (token, pnew->tp.vert[1]);
         Read4Vec (token, pnew->tp.vert[2]);
         Read4Vec (token, pnew->tp.vert[3]);
      }
      else
         Error ("Invalid parallelepiped subfield (%s).\n", token);
   }

   Process_TetPar (&pnew->tp);

   if (!pnew->info.attr)
      Error ("Missing attributes for parallelepiped description.", nil);

   pnew->info.next = objlist;
   objlist = (ObjInfo *)(prev = pnew);
}



/****************************************************************************
//  This routine reads in a description of a 4D tetrahedron with four
//  vertices and adds it to the object list.
****************************************************************************/

void  DoTetrahedron ()
{
   static   Tetrahedron *prev = &DefTetra;/* Previously Defined Tetrahedron*/
   register Tetrahedron *tnew;		  /* New Tetrahedron */

   /* Gobble up the opening parenthesis. */

   if (GetToken(token,false), token[0] != '(')
      Error ("Missing opening parenthesis for tetrahedron definition.", nil);

   tnew = NEW (Tetrahedron,1);
   *tnew = *prev;

   while (GetToken(token,false), token[0] != ')')
   {
      if (keyeq (token, "attri"))
      {  GetToken (token,false);
         tnew->info.attr = (token[0] == '(') ? ReadAttributes()
                                             : FindAttributes(token);
      }
      else if (keyeq (token, "verti"))
      {  Read4Vec (token, tnew->tp.vert[0]);
         Read4Vec (token, tnew->tp.vert[1]);
         Read4Vec (token, tnew->tp.vert[2]);
         Read4Vec (token, tnew->tp.vert[3]);
      }
      else
         Error ("Invalid tetrahedron subfield (%s).\n", token);
   }

   Process_TetPar (&tnew->tp);

   if (!tnew->info.attr)
      Error ("Missing attributes for tetrahedron description.", nil);

   tnew->info.next = objlist;
   objlist = (ObjInfo *)(prev = tnew);
}



/****************************************************************************
**  This subroutine reads in a triangle description.
****************************************************************************/

void  DoTriangle  ()
{
   static   Triangle *prev = &DefTriangle;/* Previously Defined Triangle */
   register Triangle *tnew;		  /* New Triangle */

   /* Gobble up the opening parenthesis. */

   if (GetToken(token,false), token[0] != '(')
      Error ("Missing opening parenthesis for triangle definition.", nil);

   tnew = NEW (Triangle,1);
   *tnew = *prev;

   while (GetToken(token,false), token[0] != ')')
   {
      if (keyeq (token, "attri"))
      {  GetToken (token,false);
         tnew->info.attr = (token[0] == '(') ? ReadAttributes()
                                             : FindAttributes(token);
      }
      else if (keyeq (token, "verti"))
      {  Read4Vec (token, tnew->vert[0]);
         Read4Vec (token, tnew->vert[1]);
         Read4Vec (token, tnew->vert[2]);
      }
      else
         Error ("Invalid triangle subfield (%s).\n", token);
   }

   /* Compute the two vectors from vertex 0 to vertices 1 and 2. */

   V4_3Vec (tnew->vec1, =, tnew->vert[1], -, tnew->vert[0]);
   V4_3Vec (tnew->vec2, =, tnew->vert[2], -, tnew->vert[0]);

   if (!tnew->info.attr)
      Error ("Missing attributes for triangle description.", nil);

   tnew->info.next = objlist;
   objlist = (ObjInfo *)(prev = tnew);
}



/****************************************************************************
//  This routine reads in the viewing parameters for the scene.
****************************************************************************/

void  DoView  ()
{
   /* Gobble up the opening parenthesis. */

   if (GetToken(token,false), token[0] != '(')
      Error ("Missing opening parenthesis for view definition.", nil);

   while (GetToken(token,false), token[0] != ')')
   {
           if (keyeq (token, "from" ))  Read4Vec (token, Vfrom);
      else if (keyeq (token, "to"   ))  Read4Vec (token, Vto);
      else if (keyeq (token, "up"   ))  Read4Vec (token, Vup);
      else if (keyeq (token, "over" ))  Read4Vec (token, Vover);
      else if (keyeq (token, "angle"))  ReadReal (token, &Vangle);
      else Error ("Invalid view subfield (%s).", token);
   }
}



/****************************************************************************
//  This routine initializes the physical data fields common to both the
//  tetrahedron and parallelepiped structures.
****************************************************************************/

void  Process_TetPar  (tp)
   TetPar *tp;		/* Tetrahedron / Parallelepiped Data */
{
   /* Calculate the vectors from vertex 0 to vertices 1, 2, and 3. */

   V4_3Vec (tp->vec1, =, tp->vert[1], -, tp->vert[0]);
   V4_3Vec (tp->vec2, =, tp->vert[2], -, tp->vert[0]);
   V4_3Vec (tp->vec3, =, tp->vert[3], -, tp->vert[0]);

   /* Calculate the parallelepiped's surface normal. */

   {
      auto ushort  dominant1, dominant2;	/* Dominant Axes */

      V4_Cross(tp->normal, tp->vec1,tp->vec2,tp->vec3);

      if (! V4_Normalize (tp->normal))
         Error ("Degenerate parallelepiped/tetrahedron; not 3D.");

      /* Find the dominant axis of the normal vector and load up the
      // ax1, ax2 and ax3 fields accordingly. */

      dominant1 = (fabs(tp->normal[X]) > fabs(tp->normal[Y])) ? X : Y;
      dominant2 = (fabs(tp->normal[Z]) > fabs(tp->normal[W])) ? Z : W;
      if (fabs(tp->normal[dominant1]) > fabs(tp->normal[dominant2]))
      {  tp->ax1 = (dominant1 == X) ? Y : X;
         tp->ax2 = Z;
         tp->ax3 = W;
      }
      else
      {  tp->ax1 = X;
         tp->ax2 = Y;
         tp->ax3 = (dominant2 == Z) ? W : Z;
      }
   }

   /* Calculate the hyperplane constant. */

   tp->planeConst = - V4_Dot (tp->normal, tp->vert[0]);

#  if (DB_TETPAR >= 1)
   {  auto   Lfloat  NdotV;      /* Normal Dot Vertex */

      NdotV = V4_Dot (tp->normal, tp->vert[0]);
      if (EPSILON < fabs (NdotV + tp->planeConst))
      {  printf ("ERROR:  planeConst %10.8lf, Normal dot V0 %10.8lf.\n",
            tp->planeConst, NdotV);
         Halt ("Aborting.", nil);
      }

      NdotV = V4_Dot (tp->normal, tp->vert[1]);
      if (EPSILON < fabs (NdotV + tp->planeConst))
      {  printf ("ERROR:  planeConst %10.8lf, Normal dot V1 %10.8lf.\n",
            tp->planeConst, NdotV);
         Halt ("Aborting.", nil);
      }

      NdotV = V4_Dot (tp->normal, tp->vert[2]);
      if (EPSILON < fabs (NdotV + tp->planeConst))
      {  printf ("ERROR:  planeConst %10.8lf, Normal dot V2 %10.8lf.\n",
            tp->planeConst, NdotV);
         Halt ("Aborting.", nil);
      }

      NdotV = V4_Dot (tp->normal, tp->vert[3]);
      if (EPSILON < fabs (NdotV + tp->planeConst))
      {  printf ("ERROR:  planeConst %10.8lf, Normal dot V3 %10.8lf.\n",
            tp->planeConst, NdotV);
         Halt ("Aborting.", nil);
      }
   }
#  endif

   /* Calculate the divisor for Cramer's Rule used to determine the
   // barycentric coordinates of intersection points.  */

   {
      auto Lfloat  M11,M12,M13, M21,M22,M23, M31,M32,M33;

      M11 = tp->vec1[tp->ax1];
      M12 = tp->vec1[tp->ax2];
      M13 = tp->vec1[tp->ax3];

      M21 = tp->vec2[tp->ax1];
      M22 = tp->vec2[tp->ax2];
      M23 = tp->vec2[tp->ax3];

      M31 = tp->vec3[tp->ax1];
      M32 = tp->vec3[tp->ax2];
      M33 = tp->vec3[tp->ax3];

      tp->CramerDiv = M11 * (M22*M33 - M23*M32)
                    - M21 * (M12*M33 - M13*M32)
                    + M31 * (M12*M23 - M13*M22);
   }
}



/****************************************************************************
//  This function finds an attribute description with the given name and
//  returns a pointer to the attributes node.  If the name is not found,
//  this routine aborts after flagging the error.
****************************************************************************/

Attributes  *FindAttributes  (name)
   char *name;			/* Attributes Name */
{
   register AttrName *anptr;	/* Attribute Name Node Traversal Pointer */

   name[MAXATNAME] = 0;

   for (anptr=attrnamelist;  anptr;  anptr=anptr->next)
      if (strrel (name, ==, anptr->name))
         return anptr->attr;

   Error ("Can't find attribute definition (%s).", name);
}



/****************************************************************************
//  This routine reads in a description of object attributes, creates a new
//  attributes node, links it into the attributes list, and returns a pointer
//  to the new attributes node.
//
//  It is assumed that the opening parenthesis has already been consumed at
//  the time this routine is called.
****************************************************************************/

Attributes  *ReadAttributes  ()
{
   auto   Attributes *newattr;		/* New Attributes */

   /* Allocate the new attributes node, load it with the most recent named
   // attributes node, and add it to the attributes list. */

   newattr = NEW (Attributes, 1);
   *newattr = *prevattr;
   newattr->next = attrlist;
   attrlist = newattr;

   /* Clear the shortcut flags. */

   newattr->flags &= ~(AT_AMBIENT | AT_DIFFUSE | AT_SPECULAR | AT_TRANSPAR);

   /* Process each of the attributes fields. */

   while (GetToken(token,false), token[0] != ')')
   {
           if (keyeq (token, "ambie"))  ReadColor (token, &newattr->Ka);
      else if (keyeq (token, "diffu"))  ReadColor (token, &newattr->Kd);
      else if (keyeq (token, "specu"))  ReadColor (token, &newattr->Ks);
      else if (keyeq (token, "trans"))  ReadColor (token, &newattr->Kt);
      else if (keyeq (token, "shine"))  ReadReal  (token, &newattr->shine);

      else if (keyeq (token, "index"))
      {  ReadReal (token, &newattr->indexref);
         if (newattr->indexref <= 0.0)
            Error ("Non-positive index of refraction.", nil);
      }
      else if (keyeq (token, "refle"))
      {  auto   ushort  scratch;
         ReadUint16 (token, &scratch);
         if (scratch == 1)
            newattr->flags |=  AT_REFLECT;
         else if (scratch == 0)
            newattr->flags &= ~AT_REFLECT;
         else
            Error ("Invalid `reflect' argument; should be 0 or 1.", nil);
      }
      else
         Error ("Invalid attributes field (%s).", token);
   }

   if ((newattr->Ka.r + newattr->Ka.g + newattr->Ka.b) > EPSILON)
      newattr->flags |= AT_AMBIENT;

   if ((newattr->Kd.r + newattr->Kd.g + newattr->Kd.b) > EPSILON)
      newattr->flags |= AT_DIFFUSE;

   if ((newattr->Ks.r + newattr->Ks.g + newattr->Ks.b) > EPSILON)
      newattr->flags |= AT_SPECULAR;

   if ((newattr->Kt.r + newattr->Kt.g + newattr->Kt.b) > EPSILON)
      newattr->flags |= AT_TRANSPAR;

   return newattr;
}



/****************************************************************************
//  This routine reads in a color vector from the input stream and stuffs it
//  in the location given in the parameter list.
****************************************************************************/

void  ReadColor  (token, color)
   char  *token;	/* Token We're Reading For */
   Color *color;	/* Destination Color */
{
   auto   char  inbuff[MAXTLEN+1];      /* Input Value Buffer */

   GetToken (inbuff, false);
   if (chtype[*inbuff] != NUM)
      Error ("Missing real number for red component of '%s'.", token);
   color->r = atof (inbuff);

   GetToken (inbuff, false);
   if (chtype[*inbuff] != NUM)
      Error ("Missing real number for green component of '%s'.", token);
   color->g = atof (inbuff);

   GetToken (inbuff, false);
   if (chtype[*inbuff] != NUM)
      Error ("Missing real number for blue component of '%s'.", token);
   color->b = atof (inbuff);
}



/****************************************************************************
**  This routine reads in a real-valued number from the input stream and
**  stores it in the location given in the parameter list.
****************************************************************************/

void  ReadReal  (token, num)
   char   *token;	/* Target Token */
   Lfloat *num;		/* Destination Value */
{
   auto   char  inbuff[MAXTLEN+1];      /* Input Value Buffer */

   GetToken (inbuff, false);
   if (chtype[*inbuff] != NUM)
      Error ("Missing real number argument for '%s'.", token);
   *num = atof (inbuff);
}



/****************************************************************************
**  This procedure reads in a 16-bit unsigned integer from the input stream
**  and stores it in the location given in the parameter list.
****************************************************************************/

void  ReadUint16  (token, num)
   char   *token;	/* Target Token */
   ushort *num;		/* Destination Value */
{
   auto   char  inbuff[MAXTLEN+1];      /* Input Value Buffer */

   GetToken (inbuff, false);
   if (chtype[*inbuff] != NUM)
      Error ("Missing integer argument for '%s'.", token);
   *num = atoi (inbuff);
}



/****************************************************************************
**  This procedure reads in a 4-vector from the input stream and stores it
**  into the specified location.
****************************************************************************/

void  Read4Vec  (token, vec)
   char    *token;	/* Target Token */
   Vector4  vec;	/* Destination Vector */
{
   auto   char  inbuff[MAXTLEN+1];      /* Input Value Buffer */

   GetToken (inbuff, false);
   if (chtype[*inbuff] != NUM)
      Error ("Missing real number for X component of '%s'.", token);
   vec[0] = atof (inbuff);

   GetToken (inbuff, false);
   if (chtype[*inbuff] != NUM)
      Error ("Missing real number for Y component of '%s'.", token);
   vec[1] = atof (inbuff);

   GetToken (inbuff, false);
   if (chtype[*inbuff] != NUM)
      Error ("Missing real number for Z component of '%s'.", token);
   vec[2] = atof (inbuff);

   GetToken (inbuff, false);
   if (chtype[*inbuff] != NUM)
      Error ("Missing real number for W component of '%s'.", token);
   vec[3] = atof (inbuff);
}



/****************************************************************************
**  This routine handles errors in the input stream.  It prints out the
**  current line number of the input stream and prints the error message and
**  the optional printf()-like argument.  After printing the message it halts
**  execution of the raytracer.
****************************************************************************/

void  Error  (fmt, arg)
   char *fmt;
   char *arg;
{
   auto   AttrName *attrname;	/* Attribute Name Node Pointer */

   printf ("Input Error [Line %lu]:  ", lcount);
   printf (fmt, arg);
   print  ("\n");

   /* Kill the attributes name list. */

   while (attrname = attrnamelist, attrname)
   {  attrnamelist = attrname->next;
      DELETE (attrname);
   }

   /* Halt the program. */

   Halt ("Aborting.");
}
