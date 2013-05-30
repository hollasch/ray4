
##  Makefile for Hollasch's Thesis:  "Four-Space Visualization of 4D Objects"
##
##  This makefile contains several targets, including the spell-check output,
##  nroff-output text files, printed files, troff raw output (for creation of
##  the PostScript files).
##
##  Note that the TPRINT macro should contain the path and arguments of the
##  troff print tool.  The RAWOUT macro contains the path and arguments for
##  the tool that creates the raw troff output files.

CHAPS  = chapter1 chapter2 chapter3 chapter4 chapter5 chapter6
MACROS = thesis.macros 4dv.macros
TPRINT = ptroff
RAWOUT = roffcat

info:
	@echo " "
	@echo "Makefile Targets:"
	@echo "    spell.out"
	@echo "    {title,abstract,body,end,all}.view"
	@echo "    {title,abstract,body,end,all}.print"
	@echo "    raw
	@echo "    clean"
	@echo " "

spell.out:  title abstract $(CHAPS) references appendix
	cat title abstract chapter? references appendix \
	| spell -s \
	| sort -uf >spell.out

title.view: title
	nroff title > title.view

abstract.view: abstract
	nroff abstract > abstract.view

body.view: $(MACROS) $(CHAPS) references appendix
	neqn $(MACROS) $(CHAPS) references appendix \
	| nroff 1> body.view 2> pageinfo

end.view: thesis.macros references appendix
	nroff thesis.macros references appendix > end.view

toc.view: toc.macros lof.macros pageinfo
	nroff toc.macros pageinfo > toc.view
	nroff lof.macros pageinfo > lof.view

all.view: title.view abstract.view body.view toc.view

title.print: title
	$(TPRINT) title

abstract.print: abstract
	$(TPRINT) abstract

body.print: $(MACROS) $(CHAPS)
	eqn $(MACROS) $(CHAPS) references appendix | $(TPRINT) 2> pageinfo

end.print: thesis.macros references appendix
	$(TPRINT) thesis.macros references appendix

toc.print: toc.macros lof.macros pageinfo
	cat toc.macros pageinfo | $(TPRINT)
	cat lof.macros pageinfo | $(TPRINT)

all.print:  title.print abstract.print body.print toc.print

raw: title abstract $(MACROS) $(CHAPS) toc.macros lof.macros
	$(RAWOUT) title > title.raw
	$(RAWOUT) abstract > abstract.raw
	eqn $(MACROS) $(CHAPS) references appendix | $(RAWOUT) > body.raw
	$(RAWOUT) toc.macros pageinfo > toc.raw
	$(RAWOUT) lof.macros pageinfo > lof.raw

clean:
	rm -f *.view spell.out
