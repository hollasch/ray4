
MYINCL  = ../headers
MYLIBS  = ../lib
CCFLAGS = -I. -I$(MYINCL) -Zf -Zv

.c.o:
	cc -c $(CCOPTS) $(CCFLAGS) -o $*.o $*.c

all: ray4 r4tosgi

ray4:  r4_main.o r4_hit.o r4_io.o r4_parse.o r4_trace.o $(MYLIBS)/libsrh.a
	cc $(CCOPTS) -o ray4 r4_main.o r4_hit.o r4_io.o r4_parse.o r4_trace.o \
		-Zf -lm $(MYLIBS)/libsrh.a

r4tosgi:  r4tosgi.c  $(MYINCL)/steve.h  r4_image.h
	cc $(CCOPTS) $(CCFLAGS) -Zg -o r4tosgi r4tosgi.c

r4_main.o:   r4_main.c  ray4.h $(MYINCL)/steve.h $(MYINCL)/vector.h r4_image.h
r4_hit.o:    r4_hit.c   ray4.h $(MYINCL)/steve.h $(MYINCL)/vector.h
r4_io.o:     r4_io.c    ray4.h $(MYINCL)/steve.h
r4_parse.o:  r4_parse.c ray4.h $(MYINCL)/steve.h $(MYINCL)/vector.h
r4_trace.o:  r4_trace.c ray4.h $(MYINCL)/steve.h $(MYINCL)/vector.h

clean:
	-rm -f ray4 r4tosgi *.o 2> /dev/null

space:
	-rm -f *.o 2> /dev/null
