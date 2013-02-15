CFLAGS = -g -O2 -Wall -Wextra
CPPFLAGS = -D_FILE_OFFSET_BITS=64

.PHONY: all
all: woodworm

.PHONY: clean
clean:
	rm -f woodworm

# vim:ts=4 sw=4 noet
