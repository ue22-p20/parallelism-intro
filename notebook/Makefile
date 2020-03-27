NOTEBOOK := $(shell git ls-files *.ipynb)
SPHINX := sphinx-build
BROWSER := firefox

all: sphinx

view: sphinx
	$(BROWSER) _build/html/index.html

sphinx: _build/html/index.html

_build/html/index.html: index.rst conf.py $(NOTEBOOKS)
	$(SPHINX) -b html . _build/html

clean:
	$(RM) -rf _build
	$(RM) -rf *_nbfiles

