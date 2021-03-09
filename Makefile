# Build rules for the presentations.

# What to build.
TARGETS=lsp-course-labs.pdf lsp-course-lecture-notes.pdf

# --------------------------------------------------------------------------
# Do not modify below this line.

OUT=out

all: $(TARGETS)

$(OUT):
	mkdir $(OUT)

%.pdf: %.latex | Makefile $(OUT)
	lualatex -output-directory=$(OUT) $^
	lualatex -output-directory=$(OUT) $^
	mv $(OUT)/$@ $@

clean:
	rm -fr $(OUT)
	rm -fr $(TARGETS)
