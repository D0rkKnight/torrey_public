version := 0.1.0

ifndef PREFIX
PREFIX := /usr
endif

ifdef MINGW
bin_suffix := .exe
endif

make_cflags := -Wall $(CFLAGS) -I$(PREFIX)/include
make_ldflags := $(LDFLAGS) -L$(PREFIX)/lib

ifndef NDEBUG
make_cflags := $(make_cflags) -g
make_ldflags := $(make_ldflags) -g
endif

ifndef libpng_config
libpng_config := libpng-config
endif

build_files := Makefile
info_files := AUTHORS COPYING INSTALL README TODO
header_files := *.hpp
source_files := pngpptest.cpp
test_files := test/
dist_files := $(build_files) $(info_files) \
  $(header_files) $(source_files) \
  $(test_files)


target_test := pngpptest$(bin_suffix)
targets := $(target_test)

all: $(targets)

install:
	mkdir -p $(PREFIX)/include/png++
	cp *.hpp $(PREFIX)/include/png++

dist:
	tar -zcf png++-$(version).tar.gz --exclude=.svn $(dist_files)

clean:
	rm -f $(targets)

thorough-clean: clean test-clean

test: $(target_test)
	./$(target_test) test/*.png

test-clean:
	rm -f *.png

.PHONY: all install dist clean thorough-clean test test-clean

$(target_test): pngpptest.cpp *.hpp Makefile
	g++ -o $@ $< $(make_cflags) $(make_ldflags) \
	  `$(libpng_config) --cflags --ldflags --libs`
