all: manual.pdf

# run xelatex 3 times to get the references & layout right
manual.pdf: manual.tex
	xelatex -interaction=nonstopmode -halt-on-error manual.tex
	xelatex -interaction=nonstopmode -halt-on-error manual.tex
	xelatex -interaction=nonstopmode -halt-on-error manual.tex
