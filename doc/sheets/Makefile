SHEETS = JBit-IO.pdf

all: $(SHEETS)

clean:
	rm -f *.aux *.log *.pdf

%.pdf: %.tex
	pdflatex $<
