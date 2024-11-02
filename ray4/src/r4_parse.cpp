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

//==================================================================================================
// r4_parse.cpp
//
// This file contains procedures that are used to parse the input file and build the scene for the
// raytrace.
//==================================================================================================

#include <string.h>
#include <stdarg.h>

#include "ray4.h"

const int EOFC = -1;


// Defined Constants

const int MAXTLEN   = 80;  // Maximum Token Length in Input File
const int KEYSIG    = 5;   // Significant Chars of a Keyword (max 255)
const int MAXATNAME = 15;  // Maximum Attribute Name Length

const char ERR = 0;  // Erroneous Character
const char SPC = 1;  // Whitespace Character
const char WRD = 2;  // Keyword Character
const char NUM = 3;  // Number Character
const char OTH = 4;  // Other Character

const Color BLACK = { 0.000, 0.000, 0.000 };

    // The following array is used to determine character types.

char chtype[256] = {
    ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR, ERR,SPC,SPC,SPC, SPC,SPC,ERR,ERR,  // 0X
    ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR, ERR,ERR,ERR,ERR,  // 1X
    SPC,OTH,OTH,OTH, OTH,OTH,OTH,OTH, OTH,OTH,OTH,OTH, SPC,NUM,NUM,OTH,  // 2X
    NUM,NUM,NUM,NUM, NUM,NUM,NUM,NUM, NUM,NUM,OTH,OTH, OTH,OTH,OTH,OTH,  // 3X
    OTH,WRD,WRD,WRD, WRD,WRD,WRD,WRD, WRD,WRD,WRD,WRD, WRD,WRD,WRD,WRD,  // 4X
    WRD,WRD,WRD,WRD, WRD,WRD,WRD,WRD, WRD,WRD,WRD,SPC, OTH,SPC,OTH,WRD,  // 5X
    OTH,WRD,WRD,WRD, WRD,WRD,WRD,WRD, WRD,WRD,WRD,WRD, WRD,WRD,WRD,WRD,  // 6X
    WRD,WRD,WRD,WRD, WRD,WRD,WRD,WRD, WRD,WRD,WRD,SPC, OTH,SPC,OTH,OTH   // 7X
};

inline char CType (int c) {
    if (0 <= c && c <= 0x7f)
        return chtype[c];
    return ERR;
}


enum class VarType { Other, Vec4, UInt16, Real, Color, End };

void DoAttributes();
void DoLight();
void DoParallelepiped();
void DoSphere();
void DoTetrahedron();
void DoTriangle();
void DoView();

struct {
    char     keyword[KEYSIG+1];
    VarType  vtype;
    char    *address;
} Globals[] = {
    { "attri", VarType::Other,  (char*)  DoAttributes    },
    { "ambie", VarType::Color,  (char*) &ambient         },
    { "backg", VarType::Color,  (char*) &background      },
    { "index", VarType::Real,   (char*) &global_indexref },
    { "light", VarType::Other,  (char*)  DoLight         },
    { "maxde", VarType::UInt16, (char*) &maxdepth        },
    { "paral", VarType::Other,  (char*)  DoParallelepiped},
    { "spher", VarType::Other,  (char*)  DoSphere        },
    { "tetra", VarType::Other,  (char*)  DoTetrahedron   },
    { "trian", VarType::Other,  (char*)  DoTriangle      },
    { "view",  VarType::Other,  (char*)  DoView          },
    { "",      VarType::End,    nullptr                  }
};


struct AttrName {
    AttrName   *next;               // Next Attribute Name Node
    char        name[MAXATNAME+1];  // Attribute Name
    Attributes *attr;               // Attributes
};


// Default Structures

Light DefLight = {
    nullptr,                // Next Light Source
    { 1.0, 1.0, 1.0 },      // Light Color
    LightType::Directional  // Light Type
};

Attributes DefAttributes = {
    nullptr,                     // Next Attribute Node
    BLACK, BLACK, BLACK, BLACK,  // Ambient, Diffuse, Specular, Transpar.
    1.0,                         // Shine
    1.0,                         // Index of Refraction
    0                            // Attribute Flags
};

Sphere DefSphere = {
    {
        nullptr,          // Next Pointer
        nullptr,          // Attributes
        ObjType::Sphere,  // Object Type
        0,                // Object Flags
        HitSphere         // Sphere-Intersection Function
    }
};

Tetrahedron DefTetra = {
    {
        nullptr,               // Next Pointer
        nullptr,               // Attributes
        ObjType::Tetrahedron,  // Object Type
        0,                     // Object Flags
        HitTetPar              // Tetrahedron-Intersection Function
    }
};

Parallelepiped DefPllp = {
    {
        nullptr,                  // Next Pointer
        nullptr,                  // Attributes
        ObjType::Parallelepiped,  // Object Type
        0,                        // Object Flags
        HitTetPar                 // Parallelepiped-Intersection Function
    }
};

Triangle DefTriangle = {
    {
        nullptr,            // Next Pointer
        nullptr,            // Attributes
        ObjType::Triangle,  // Object Type
        0,                  // Object Flags
        HitTriangle         // Triangle-Intersection Function
    }
};


// Global Variables

static AttrName   *attrnamelist = nullptr;    // Attribute Name List
static bool        eofflag = false;           // Non-Zero If EOF Input File
static ulong       lcount = 1;                // Input Line Counter
static Attributes *prevattr= &DefAttributes;  // Previously Named Attribute
static char        token[MAXTLEN+1];          // Input Token



//__________________________________________________________________________________________________

void Error (const char *format, ...) {
    // This routine handles errors in the input stream. It prints out the current line number of the
    // input stream and prints the error message and the optional printf()-like argument. After
    // printing the message it halts execution of the raytracer.

    va_list args;  // List of Optional Arguments

    va_start(args, format);

    printf("Input Error [Line %lu]:  ", lcount);
    vprintf(format, args);
    print("\n");

    va_end(args);

    // Kill the attributes name list.

    AttrName *attrname;  // Attribute Name Node Pointer
    while (attrname = attrnamelist, attrname) {
        attrnamelist = attrname->next;
        DELETE (attrname);
    }

    // Halt the program.

    Halt ("Aborting.");
}

//__________________________________________________________________________________________________

char *GetToken  (
    char *buff,   // Token Buffer
    bool  eofok)  // If 1, EOF is OK, otherwise error
{
    // This routine reads the input stream and returns the next token. It's basically the lexical
    // analyzer. It returns the destination buffer given as the single parameter.

    int cc = ' ';  // Input Character

    if (cc == EOFC) {
        eofflag = true;
        if (eofok)
            return nullptr;
        Error ("Unexpected end-of-file.");
    }

    char   *ptr = buff;  // Destination Buffer Pointer
    ushort  nn  = 0;     // Length of Destination String

    // Skip past comments and whitespace.

    for (;;) {
        if (cc == '>') {
            while (cc=ReadChar(), (cc != '\n') && (cc != '\r')) {
                if (cc == EOFC) {
                    eofflag = true;
                    if (eofok)
                        return nullptr;
                    Error ("Unexpected end-of-file.");
                }
            }
        }

        while (CType(cc) == SPC) {
            if (cc == '\n')  ++lcount;

            if (EOFC == (cc = ReadChar ())) {
                eofflag = true;
                if (eofok)
                    return nullptr;
                Error ("Unexpected end-of-file.");
            }
        }

        if (cc != '>')
            break;
    }

    // If the next character is a punctuation character, then just return that character as a single
    // token.

    if (CType(cc) == OTH) {
        *ptr++ = static_cast<char>(cc);
        *ptr   = 0;
        return buff;
    }

    // If the next character starts an alphanumeric word, then read the word. Words begin with an
    // alphabetic character or an underbar, and may also contain numbers, periods and dashes in the
    // body.

    if (CType(cc) == WRD) {
        for (;;) {
            if (++nn > MAXTLEN) {
                *ptr = 0;
                Error ("Input token is too long (%s).", buff);
            }
            *ptr++ = static_cast<char>(cc);

            cc = ReadChar ();

            if ((cc == EOFC) || ((CType(cc) != WRD) && (CType(cc) != NUM))) {
                UnreadChar(cc);
                break;
            }
        }

        *ptr = 0;
        if (buff[0])
            return buff;

        eofflag = true;
        if (eofok)
            return nullptr;

        Error ("Unexpected end-of-file.");
    }

    if (CType(cc) == NUM) {
        bool eflag = false;  // True After 'e' Character Is Read
        bool dflag = false;  // True After '.' Character Is Read

        if (cc == '.')   dflag = true;

        for (;;) {
            if (++nn > MAXTLEN) {
                *ptr = 0;
                Error ("Input token is too long (%s).", buff);
            }
            *ptr++ = static_cast<char>(cc);

            if (EOFC == (cc = ReadChar ()))
                break;

            if (cc == '.') {
                if (dflag || eflag) {
                    break;
                } else {
                    dflag = 1;
                    continue;
                }
            } else if (cc == 'e') {
                if (eflag) {
                    break;
                } else {
                    eflag = 1;
                    continue;
                }
            } else if (cc == '-') {
                if ((ptr[-1] | 0x20) != 'e')
                    break;
                continue;
            }

            if (CType(cc) != NUM) {
                UnreadChar(cc);
                break;
            }
        }

        *ptr = 0;
        if (buff[0])
            return buff;

        eofflag = true;
        if (eofok)
            return nullptr;

        Error ("Unexpected end-of-file.");
    }

    // Unexpected character. Print out the hexadecimal value and halt.

    Error ("Unexpected character in input stream (0x%02x).", static_cast<int>(cc & 0xFF));
    return nullptr;
}

//__________________________________________________________________________________________________

bool keyeq (
    const char *string,  // Input Token
    const char *key)     // First KEYSIG digits of keyword.
{
    // This routine compares an input token with a keyword. If the two strings match up to the
    // significant length, this routine returns 1, otherwise it returns 0. This routine assumes that
    // the keyword is all lowercase letters and that the string is uppercase or lowercase letters
    // only. This will be true if GetToken() does its job correctly. Furthermore if the string
    // matches the key until the string ends, then this function will still return 1. That is,
    // keyeq("amb", "ambient") == 1.

    for (auto i=0;  (i < KEYSIG) && key[i] && string[i];  ++i)
        if (key[i] != (string[i] | 0x20))
            return false;

    return true;
}

//__________________________________________________________________________________________________

void ReadColor (char *ctoken, Color *color) {
    // This routine reads in a color vector from the input stream and stuffs it in the location
    // given in the parameter list.

    char inbuff[MAXTLEN+1];  // Input Value Buffer

    GetToken (inbuff, false);
    if (CType(*inbuff) != NUM)
        Error ("Missing real number for red component of '%s'.", ctoken);
    color->r = atof (inbuff);

    GetToken (inbuff, false);
    if (CType(*inbuff) != NUM)
        Error ("Missing real number for green component of '%s'.", ctoken);
    color->g = atof (inbuff);

    GetToken (inbuff, false);
    if (CType(*inbuff) != NUM)
        Error ("Missing real number for blue component of '%s'.", ctoken);
    color->b = atof (inbuff);
}

//__________________________________________________________________________________________________

void ReadReal (char *ctoken, double *num) {
    // This routine reads in a real-valued number from the input stream and stores it in the
    // location given in the parameter list.

    char inbuff[MAXTLEN+1];  // Input Value Buffer

    GetToken (inbuff, false);
    if (CType(*inbuff) != NUM)
        Error ("Missing real number argument for '%s'.", ctoken);
    *num = atof (inbuff);
}

//__________________________________________________________________________________________________

void ReadUint16 (char *itoken, ushort *num) {
    // This procedure reads in a 16-bit unsigned integer from the input stream and stores it in the
    // location given in the parameter list.

    char inbuff[MAXTLEN+1];  // Input Value Buffer

    GetToken (inbuff, false);
    if (CType(*inbuff) != NUM)
        Error ("Missing integer argument for '%s'.", itoken);
    *num = static_cast<ushort>(atoi (inbuff));
}

//__________________________________________________________________________________________________

void ReadVector4 (char *vtoken, Vector4 &vec) {
    // This procedure reads in a 4-vector from the input stream and stores it into the specified
    // location.

    char inbuff[MAXTLEN+1];  // Input Value Buffer

    GetToken (inbuff, false);
    if (CType(*inbuff) != NUM)
        Error ("Missing real number for X component of '%s'.", vtoken);
    vec[0] = atof (inbuff);

    GetToken (inbuff, false);
    if (CType(*inbuff) != NUM)
        Error ("Missing real number for Y component of '%s'.", vtoken);
    vec[1] = atof (inbuff);

    GetToken (inbuff, false);
    if (CType(*inbuff) != NUM)
        Error ("Missing real number for Z component of '%s'.", vtoken);
    vec[2] = atof (inbuff);

    GetToken (inbuff, false);
    if (CType(*inbuff) != NUM)
        Error ("Missing real number for W component of '%s'.", vtoken);
    vec[3] = atof (inbuff);
}

//__________________________________________________________________________________________________

void ReadPoint4 (char *vtoken, Point4 &p) {
    Vector4 v;
    ReadVector4(vtoken, v);

    p[0] = v[0];
    p[1] = v[1];
    p[2] = v[2];
    p[3] = v[3];
}

//__________________________________________________________________________________________________

void ParseInput () {
    // This routine parses the input scene description, and sets up the global raytrace variables
    // and the object lists.

    while (GetToken(token, true)) {
        int i;

        for (i=0;  Globals[i].vtype != VarType::End;  ++i) {
            if (keyeq(token, Globals[i].keyword))
                break;
        }

        if (Globals[i].vtype == VarType::End)
            Error ("Unknown keyword (%s).", token);

        switch (Globals[i].vtype) {
            case VarType::Color:
                ReadColor (token, reinterpret_cast<Color*>(Globals[i].address));
                break;

            case VarType::UInt16:
                ReadUint16 (token, reinterpret_cast<ushort*>(Globals[i].address));
                break;

            case VarType::Real:
                ReadReal (token, reinterpret_cast<double*>(Globals[i].address));
                break;

            case VarType::Other:
                bool (*func)();  // Function Pointer
                func = reinterpret_cast<bool(*)()>(Globals[i].address);
                (*func)();
                break;

            default:
                Halt ("Internal Error (Globals vtype switch).");
        }
    }

    // Kill the attributes alias list.

    AttrName *attrname;  // Attribute Name Node Pointer
    while (attrname=attrnamelist, attrname) {
        attrnamelist = attrname->next;
        DELETE (attrname);
    }
}

//__________________________________________________________________________________________________

Attributes *ReadAttributes () {
    // This routine reads in a description of object attributes, creates a new attributes node,
    // links it into the attributes list, and returns a pointer to the new attributes node.
    //
    // It is assumed that the opening parenthesis has already been consumed at the time this routine
    // is called.

    // Allocate the new attributes node, load it with the most recent named attributes node, and add
    // it to the attributes list.

    Attributes *newattr = NEW (Attributes, 1);  // New Attributes
    *newattr = *prevattr;
    newattr->next = attrlist;
    attrlist = newattr;

    // Clear the shortcut flags.

    newattr->flags &= ~(AT_AMBIENT | AT_DIFFUSE | AT_SPECULAR | AT_TRANSPAR);

    // Process each of the attributes fields.

    while (GetToken(token,false), token[0] != ')') {
        if (keyeq (token, "ambie")) {
            ReadColor (token, &newattr->Ka);
        } else if (keyeq (token, "diffu")) {
            ReadColor (token, &newattr->Kd);
        } else if (keyeq (token, "specu")) {
            ReadColor (token, &newattr->Ks);
        } else if (keyeq (token, "trans")) {
            ReadColor (token, &newattr->Kt);
        } else if (keyeq (token, "shine")) {
            ReadReal  (token, &newattr->shine);
        } else if (keyeq (token, "index")) {
            ReadReal (token, &newattr->indexref);
            if (newattr->indexref <= 0.0)
                Error ("Non-positive index of refraction.");
        } else if (keyeq (token, "refle")) {
            ushort  scratch;
            ReadUint16 (token, &scratch);
            if (scratch == 1)
                newattr->flags |=  AT_REFLECT;
            else if (scratch == 0)
                newattr->flags &= ~AT_REFLECT;
            else
                Error ("Invalid `reflect' argument; should be 0 or 1.");
        } else {
            Error ("Invalid attributes field (%s).", token);
        }
    }

    if ((newattr->Ka.r + newattr->Ka.g + newattr->Ka.b) > epsilon)
        newattr->flags |= AT_AMBIENT;

    if ((newattr->Kd.r + newattr->Kd.g + newattr->Kd.b) > epsilon)
        newattr->flags |= AT_DIFFUSE;

    if ((newattr->Ks.r + newattr->Ks.g + newattr->Ks.b) > epsilon)
        newattr->flags |= AT_SPECULAR;

    if ((newattr->Kt.r + newattr->Kt.g + newattr->Kt.b) > epsilon)
        newattr->flags |= AT_TRANSPAR;

    return newattr;
}

//__________________________________________________________________________________________________

Attributes *FindAttributes (char *name) {
    // This function finds an attribute description with the given name and returns a pointer to the
    // attributes node. If the name is not found, this routine aborts after flagging the error.

    name[MAXATNAME] = 0;

    AttrName *anptr;  // Attribute Name Node Traversal Pointer
    for (anptr=attrnamelist;  anptr;  anptr=anptr->next)
        if (strcmp(name, anptr->name) == 0)
            break;

    if (!anptr)
        Error ("Can't find attribute definition (%s).", name);

    return anptr->attr;
}

//__________________________________________________________________________________________________

void DoAttributes () {
    // This procedure processes attribute definitions. Attribute definitions consist of the keyword
    // `attributes', followed by an attribute alias, and then the attribute fields.

    // Read in the attribute alias.

    GetToken (token, false);
    if ((CType(token[0]) != WRD) && (CType(token[0]) != NUM))
        Error ("Invalid attribute name (%s)\n", token);

    token[MAXATNAME] = 0;

    // Ensure that the name is not a duplicate of an earlier name.

    AttrName *anptr;  // Attributes Alias Node Pointer
    for (anptr = attrnamelist;  anptr;  anptr = anptr->next)
        if (strcmp(anptr->name, token) == 0)
            break;

    AttrName *newattrname;  // New Attributes Alias Node
    if (anptr) {
        printf ("Warning:  Attributes \"%s\" redefined at line %lu.\n",
        token, lcount);
        newattrname = anptr;
    } else {
        newattrname = NEW (AttrName, 1);
        newattrname->next = attrnamelist;
        attrnamelist = newattrname;
        strcpy (newattrname->name, token);
    }

    // Consume the opening parenthesis of the attribute description.

    GetToken (token, false);

    if (token[0] != '(')
        Error ("Expected opening parenthesis (got '%s').", token);

    prevattr = newattrname->attr = ReadAttributes ();
}

//__________________________________________________________________________________________________

void DoLight () {
    // This routine reads in a description of a light source from the input file. At the calling
    // point, the keyword "Light" has already been read. The new light will be added to the light
    // list.

    static Light *prev = &DefLight;  // Previously Defined Light

    // Gobble up the opening parenthesis.

    if (GetToken(token,false), token[0] != '(')
        Error ("Missing opening parenthesis for light definition.");

    Light *light = NEW (Light,1);
    *light = *prev;

    while (GetToken(token,false), token[0] != ')') {
        if (keyeq (token, "color")) {
            ReadColor (token, &light->color);
        } else if (keyeq (token, "direc")) {
            ReadVector4 (token, light->u.dir);
            light->type = LightType::Directional;
        } else if (keyeq (token, "posit")) {
            ReadPoint4 (token, light->u.pos);
            light->type = LightType::Point;
        } else {
            Error ("Invalid light subfield (%s).", token);
        }
    }

    // If the light is directional, then normalize the light direction.

    if (light->type == LightType::Directional) {
        if (!light->u.dir.normalize())
            Error ("Zero light direction vector.");
    }

    light->next = lightlist;
    lightlist = prev = light;
}

//__________________________________________________________________________________________________

void DoSphere () {
    // This routine reads in a description of a hyperspherical object from the input stream and adds
    // it to the object list. The field defaults are defined by the DefSphere structure for the
    // first sphere, and then by the previous sphere.

    static Sphere *prev = &DefSphere;  // Previously Defined Sphere

    // Gobble up the opening parenthesis.

    if (GetToken(token,false), token[0] != '(')
        Error ("Missing opening parenthesis for sphere definition.");

    Sphere *snew = NEW(Sphere,1);
    *snew = *prev;

    while (GetToken(token,false), token[0] != ')') {
        if (keyeq(token, "attri")) {
            GetToken (token,false);
            snew->info.attr = (token[0] == '(') ? ReadAttributes() : FindAttributes(token);
        } else if (keyeq (token, "cente")) {
            ReadPoint4 (token, snew->center);
        } else if (keyeq (token, "radiu")) {
            ReadReal (token, &snew->radius);
        } else {
            Error ("Invalid sphere subfield (%s).\n", token);
        }
    }

    if (snew->radius < epsilon)
        Error ("Sphere has non-positive radius.");

    if (!snew->info.attr)
        Error ("Missing attributes for sphere description.");

    snew->rsqrd = snew->radius * snew->radius;

    snew->info.next = objlist;
    objlist = reinterpret_cast<ObjInfo *>(prev = snew);
}

//__________________________________________________________________________________________________

void Process_TetPar (TetPar *tp) {
    // This routine initializes the physical data fields common to both the tetrahedron and
    // parallelepiped structures.

    // Calculate the vectors from vertex 0 to vertices 1, 2, and 3.

    tp->vec1 = tp->vert[1] - tp->vert[0];
    tp->vec2 = tp->vert[2] - tp->vert[0];
    tp->vec3 = tp->vert[3] - tp->vert[0];

    // Calculate the parallelepiped's surface normal.

    {
        tp->normal = cross(tp->vec1, tp->vec2, tp->vec3);

        if (!tp->normal.normalize())
            Error ("Degenerate parallelepiped/tetrahedron; not 3D.");

        // Find the dominant axis of the normal vector and load up the ax1, ax2 and ax3 fields
        // accordingly.

        int dominant1 = (fabs(tp->normal.x) > fabs(tp->normal.y)) ? 0 : 1;
        int dominant2 = (fabs(tp->normal.z) > fabs(tp->normal.w)) ? 2 : 3;
        if (fabs(tp->normal[dominant1]) > fabs(tp->normal[dominant2])) {
            tp->ax1 = (dominant1 == 0) ? 1 : 0;
            tp->ax2 = 2;
            tp->ax3 = 3;
        } else {
            tp->ax1 = 0;
            tp->ax2 = 1;
            tp->ax3 = (dominant2 == 2) ? 3 : 2;
        }
    }

    // Calculate the hyperplane constant.

    tp->planeConst = - dot(tp->normal, tp->vert[0].toVector());

    // Calculate the divisor for Cramer's Rule used to determine the barycentric coordinates of
    // intersection points.

    {
        double M11 = tp->vec1[tp->ax1];
        double M12 = tp->vec1[tp->ax2];
        double M13 = tp->vec1[tp->ax3];

        double M21 = tp->vec2[tp->ax1];
        double M22 = tp->vec2[tp->ax2];
        double M23 = tp->vec2[tp->ax3];

        double M31 = tp->vec3[tp->ax1];
        double M32 = tp->vec3[tp->ax2];
        double M33 = tp->vec3[tp->ax3];

        tp->CramerDiv = M11 * (M22*M33 - M23*M32)
                      - M21 * (M12*M33 - M13*M32)
                      + M31 * (M12*M23 - M13*M22);
    }
}

//__________________________________________________________________________________________________

void DoParallelepiped () {
    // This routine reads in a description of a 4D parallelepiped (defined by four vertices) and
    // adds it to the object list.

    static Parallelepiped *prev= &DefPllp;  // Previously Defined Tetrahedron

    // Gobble up the opening parenthesis.

    if (GetToken(token,false), token[0] != '(')
        Error ("Missing opening parenthesis for parallelepiped definition.");

    Parallelepiped *pnew = NEW (Parallelepiped,1);
    *pnew = *prev;

    while (GetToken(token,false), token[0] != ')') {
        if (keyeq (token, "attri")) {
            GetToken (token,false);
            pnew->info.attr = (token[0] == '(') ? ReadAttributes() : FindAttributes(token);
        } else if (keyeq (token, "verti")) {
            ReadPoint4 (token, pnew->tp.vert[0]);
            ReadPoint4 (token, pnew->tp.vert[1]);
            ReadPoint4 (token, pnew->tp.vert[2]);
            ReadPoint4 (token, pnew->tp.vert[3]);
        } else {
            Error ("Invalid parallelepiped subfield (%s).\n", token);
        }
    }

    Process_TetPar (&pnew->tp);

    if (!pnew->info.attr)
        Error ("Missing attributes for parallelepiped description.");

    pnew->info.next = objlist;
    objlist = reinterpret_cast<ObjInfo *>(prev = pnew);
}

//__________________________________________________________________________________________________

void DoTetrahedron () {
    // This routine reads in a description of a 4D tetrahedron with four vertices and adds it to the
    // object list.

    static Tetrahedron *prev = &DefTetra;  // Previously Defined Tetrahedron

    // Gobble up the opening parenthesis.

    if (GetToken(token,false), token[0] != '(')
        Error ("Missing opening parenthesis for tetrahedron definition.");

    Tetrahedron *tnew = NEW (Tetrahedron,1);  // New Tetrahedron
    *tnew = *prev;

    while (GetToken(token,false), token[0] != ')') {
        if (keyeq (token, "attri")) {
            GetToken (token,false);
            tnew->info.attr = (token[0] == '(') ? ReadAttributes() : FindAttributes(token);
        } else if (keyeq (token, "verti")) {
            ReadPoint4 (token, tnew->tp.vert[0]);
            ReadPoint4 (token, tnew->tp.vert[1]);
            ReadPoint4 (token, tnew->tp.vert[2]);
            ReadPoint4 (token, tnew->tp.vert[3]);
        } else {
            Error ("Invalid tetrahedron subfield (%s).\n", token);
        }
    }

    Process_TetPar (&tnew->tp);

    if (!tnew->info.attr)
        Error ("Missing attributes for tetrahedron description.");

    tnew->info.next = objlist;
    objlist = reinterpret_cast<ObjInfo *>(prev = tnew);
}

//__________________________________________________________________________________________________

void DoTriangle () {
    // This subroutine reads in a triangle description.

    static Triangle *prev = &DefTriangle;  // Previously Defined Triangle

    // Gobble up the opening parenthesis.

    if (GetToken(token,false), token[0] != '(')
        Error ("Missing opening parenthesis for triangle definition.");

    Triangle *tnew = NEW (Triangle,1);
    *tnew = *prev;

    while (GetToken(token,false), token[0] != ')') {
        if (keyeq (token, "attri")) {
            GetToken (token,false);
            tnew->info.attr = (token[0] == '(') ? ReadAttributes() : FindAttributes(token);
        } else if (keyeq (token, "verti")) {
            ReadPoint4 (token, tnew->vert[0]);
            ReadPoint4 (token, tnew->vert[1]);
            ReadPoint4 (token, tnew->vert[2]);
        } else {
            Error ("Invalid triangle subfield (%s).\n", token);
        }
    }

    // Compute the two vectors from vertex 0 to vertices 1 and 2.

    tnew->vec1 = tnew->vert[1] - tnew->vert[0];
    tnew->vec2 = tnew->vert[2] - tnew->vert[0];

    if (!tnew->info.attr)
        Error ("Missing attributes for triangle description.");

    tnew->info.next = objlist;
    objlist = reinterpret_cast<ObjInfo *>(prev = tnew);
}

//__________________________________________________________________________________________________

void DoView () {
    // This routine reads in the viewing parameters for the scene.

    // Gobble up the opening parenthesis.

    if (GetToken(token,false), token[0] != '(')
        Error ("Missing opening parenthesis for view definition.");

    while (GetToken(token,false), token[0] != ')') {
        if (keyeq (token, "from"))
            ReadPoint4 (token, Vfrom);
        else if (keyeq (token, "to"))
            ReadPoint4 (token, Vto);
        else if (keyeq (token, "up"))
            ReadVector4 (token, Vup);
        else if (keyeq (token, "over"))
            ReadVector4 (token, Vover);
        else if (keyeq (token, "angle"))
            ReadReal (token, &Vangle);
        else
            Error ("Invalid view subfield (%s).", token);
    }
}

