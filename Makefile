
OUT=out

all: lsp-course-labs.pdf lsp-course-lecture-notes.pdf

$(OUT):
	mkdir $(OUT)

%.pdf: %.latex | Makefile $(OUT)
	lualatex -output-directory=$(OUT) $^
	lualatex -output-directory=$(OUT) $^
	mv $(OUT)/$@ $@

clean:
	rm -fr $(OUT)
