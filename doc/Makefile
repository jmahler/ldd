
all: main.pdf

# XXX - does not generate bibtex entries correctly
#%.pdf: %.tex
#	rubber -v --pdf $<

# If an errors occurs it will hang.
# Manually run the command again to see the error.
#prelim.dvi: prelim.tex
#	latex $< 1>/dev/null
#	bibtex prelim.aux 1>/dev/null
#	latex $< 1>/dev/null

#prelim.pdf: prelim.dvi
#	dvipdf $<

DEPS = hierarchy/hier.pdf

# pdf, with links
main.pdf: main.tex references.bib $(DEPS)
	pdflatex $< 1>/dev/null
	#bibtex main.aux 1>/dev/null
	biber main 1>/dev/null
	#makeindex main.idx 1>/dev/null
	pdflatex $< 1>/dev/null

clean:
	-rm -f *.aux *.log *.blg *.bbl *.toc *.lof *.bcf
	-rm -f *.out *.pdf *-blx.bib *.xml
	-rm -f *.idx *.ilg *.ind *.idl
