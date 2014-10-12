NAME=proposal
all:
	latex $(NAME).tex ; bibtex $(NAME) ; latex $(NAME).tex ; latex $(NAME).tex ; dvipdf $(NAME).dvi
	rm -f *.bbl *.blg *.dvi *.aux *.log *.out
