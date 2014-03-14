
#
# The following packages were required
# to build # on a Debian system:
#
#   biblatex
#   texlive-latex-extra
#   texlive-fonts-recommended
#

all: main.pdf

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
	pdflatex $<
	#bibtex main.aux
	biber main
	#makeindex main.idx
	pdflatex $<

clean:
	-rm -f *.aux *.log *.blg *.bbl *.toc *.lof *.bcf
	-rm -f *.out *.pdf *-blx.bib *.xml
	-rm -f *.idx *.ilg *.ind *.idl
