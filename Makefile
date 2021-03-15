# Build rules for the presentations.

# List of lectures
TARGETS_DOCS=$(shell find ./ -type f -name 'lsp-course-*.latex')

# What to build.
TARGETS=$(patsubst %.latex,%.pdf,$(TARGETS_DOCS))

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
