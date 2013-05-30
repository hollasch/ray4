##  Beta-Tester's Makefile for `wire4' and `r4tosgi'.
##  wire4:    Interactive 4D wireframe program
##  r4tosgi:  Ray4 image cube display program


MYINCL = .

    #  The GL_LEVEL describes the implementation level of the GL language
    #  on the target platform.  The higher the number, the higher the
    #  level of the GL supported:
    #
    #  2:  Supports GOURAUD shading along line segments.
    #  1:  Supports bgnline(), v2i(), endline(), subpixel() ...
    #  0:  Early GL library (e.g. for SGI 3130 workstations).
    #
    #  If depthcueing doesn't work properly (constant intensity line
    #  segments), reduce GL_LEVEL to 1.  If you get link errors, set
    #  GL_LEVEL to 0.

GL_LEVEL = -DGL_LEVEL=2

    #  On the SGI 3130 platforms (there may be others), the transcendental
    #  functions are defined as returning and taking floats, rather than
    #  doubles.  I don't know what blow-to-the-head inspired this, but the
    #  following definition provides a workaround.  If FLOAT_TRAN is not
    #  defined, then transcendental functions are declared normally.

#TrFnType= -DFLOAT_TRAN		# For SGI 3130's.
 TrFnType=			# Standard Machines.

    #  The following compiler and linker options are dependent on the
    #  target platform.  Select the ones that best describe your platform
    #  and disable the others.  Note that if you wish to use single-precision
    #  math rather than double-precision, add -DReal=float to the CCOPTS.
    
#CCOPTS   = $(GL_LEVEL) $(TrFnType) -I$(MYINCL) -Zf	# For the SGI 3130.
 CCOPTS   = $(GL_LEVEL) $(TrFnType) -I$(MYINCL)		# For other machines.

#GL_LIB   = -lgl_s		# For shared GL libraries.
 GL_LIB   = -lgl		# For IBM6000 or unshared GL libraries.

.c.o:
	cc -c -o $*.o $*.c $(CCOPTS)

all:  wire4 psplot

psplot:  psplot.c
	cc -o psplot psplot.c

wire4:	w4_main.o w4_parse.o w4_geometry.o w4_display.o strcmpic.o \
		v3cross.o v4cross.o
	cc -o wire4 w4_main.o w4_parse.o w4_geometry.o \
		w4_display.o strcmpic.o v3cross.o v4cross.o $(GL_LIB) -lm

w4_main.o:	w4_main.c     wire4.h $(MYINCL)/vector.h
w4_parse.o:	w4_parse.c    wire4.h $(MYINCL)/vector.h
w4_geometry.o:	w4_geometry.c wire4.h $(MYINCL)/vector.h
w4_display.o:	w4_display.c  wire4.h $(MYINCL)/vector.h
strcmpic.o:	strcmpic.c
v3cross.o:	v3cross.c  $(MYINCL)/vector.h
v4cross.o:	v4cross.c  $(MYINCL)/vector.h

clean:
	-rm -f wire4 psplot
	-rm -f *.o

space:
	-rm -f *.o
