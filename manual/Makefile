all: manual.pdf i2c-quickstart.pdf

# run xelatex 3 times to get the references & layout right
manual.pdf: manual.tex
	xelatex -interaction=nonstopmode -halt-on-error manual.tex
	xelatex -interaction=nonstopmode -halt-on-error manual.tex
	xelatex -interaction=nonstopmode -halt-on-error manual.tex

i2c-quickstart.pdf: i2c-quickstart.tex
	xelatex -interaction=nonstopmode -halt-on-error i2c-quickstart.tex
	xelatex -interaction=nonstopmode -halt-on-error i2c-quickstart.tex
