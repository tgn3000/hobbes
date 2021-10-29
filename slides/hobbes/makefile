Filename="hobbes"
FullFilename=$(Filename).tex
TeXFLAGS=-interaction=batchmode

pdf:
	@echo "Running pdflatex on $(FullFilename)$\ twice ..."
	@pdflatex $(TeXFLAGS) $(FullFilename)
	@pdflatex $(TeXFLAGS) $(FullFilename)

clean:
	rm -rf *.log *.top *.tui *.synctex.gz *.aux *.idx *.blg *.mp *.snm *.out *.nav *.bbl *.toc *.vrb *~ *#
