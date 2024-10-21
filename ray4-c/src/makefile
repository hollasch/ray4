##  Beta-Tester's Makefile for ray4 (a 4D raytracer).
##  ray4   :  4D raytracer
##  r4tosgi:  Ray4 image cube display program


MYINCL = .

    #  On the SGI 3130 platforms (there may be others), the transcendental
    #  functions are defined as returning and taking floats, rather than
    #  doubles.  I don't know what blow-to-the-head inspired this, but the
    #  following definition provides a workaround.  If FLOAT_TRAN is not
    #  defined, then transcendental functions are declared normally.

#TrFnType= -DFLOAT_TRAN		# For SGI 3130's.
 TrFnType=			# Standard Machines.

#Real=float			# Single-Precision Arithmetic.
 Real=double			# Double-Precision Arithmetic.

    #      The following compiler and linker options are dependent on the
    #  target platform.  Select the ones that best describe your platform
    #  and disable the others.

 CCOPTS = -DReal=$(Real) -I$(MYINCL) $(TrFnType)
#CCOPTS = $(CCOPTS) -Zf		# Floating-Point Accel. for SGI 3130

#GLLIB  = -lgl
 GLLIB  = -lgl_s

.c.o:
	cc -c -o $*.o $*.c $(CCOPTS)

all:  ray4 r4tosgi

ray4:  r4_main.o r4_hit.o r4_io.o r4_parse.o r4_trace.o v4cross.o v4normalize.o
	cc -o ray4 r4_main.o r4_hit.o r4_io.o r4_parse.o r4_trace.o \
		v4cross.o v4normalize.o $(CCOPTS) -lm

r4tosgi:  r4tosgi.c  r4_image.h
	cc -o r4tosgi r4tosgi.c $(CCOPTS) $(GLLIB) -lm

r4_main.o:	r4_main.c  ray4.h $(MYINCL)/vector.h r4_image.h
r4_hit.o:	r4_hit.c   ray4.h $(MYINCL)/vector.h
r4_io.o:	r4_io.c    ray4.h
r4_parse.o:	r4_parse.c ray4.h $(MYINCL)/vector.h
r4_trace.o:	r4_trace.c ray4.h $(MYINCL)/vector.h

v4normalize.o:	v4normalize.c $(MYINCL)/vector.h
v4cross.o:	v4cross.c     $(MYINCL)/vector.h

clean:
	-rm -f ray4 r4tosgi
	-rm -f *.o

space:
	-rm -f *.o
