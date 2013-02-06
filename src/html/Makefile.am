lib_LTLIBRARIES = libhtmlcxx.la
libhtmlcxx_INC_DIR=../../include/htmlcxx/html
libhtmlcxx_la_SOURCES = $(libhtmlcxx_INC_DIR)/ParserSax.h $(libhtmlcxx_INC_DIR)/ParserSax.tcc ParserSax.cc \
	$(libhtmlcxx_INC_DIR)/DllApi.h \
	$(libhtmlcxx_INC_DIR)/ParserDom.h ParserDom.cc \
	$(libhtmlcxx_INC_DIR)/Extensions.h Extensions.cc \
	$(libhtmlcxx_INC_DIR)/Uri.h Uri.cc \
	$(libhtmlcxx_INC_DIR)/Node.h Node.cc \
	$(libhtmlcxx_INC_DIR)/utils.h utils.cc \
	$(libhtmlcxx_INC_DIR)/ci_string.h \
	$(libhtmlcxx_INC_DIR)/tree.h \
	tld.h \
	win_cstring.h \
	debug.h
libhtmlcxx_la_CXXFLAGS = -DHTMLCXX_LIB -I../../include
libhtmlcxx_la_LDFLAGS = -version-info 4:0:1

# bin_PROGRAMS = htmlcxx
# htmlcxx_LDADD = libhtmlcxx.la
# htmlcxx_SOURCES = htmlcxx.cc

includedir = $(prefix)/include/htmlcxx/html
include_HEADERS = $(libhtmlcxx_INC_DIR)/DllApi.h \
	$(libhtmlcxx_INC_DIR)/ParserSax.h \
	$(libhtmlcxx_INC_DIR)/ParserSax.tcc \
	$(libhtmlcxx_INC_DIR)/ParserDom.h \
	$(libhtmlcxx_INC_DIR)/utils.h \
	$(libhtmlcxx_INC_DIR)/tree.h \
	$(libhtmlcxx_INC_DIR)/Node.h \
	$(libhtmlcxx_INC_DIR)/Uri.h \
	$(libhtmlcxx_INC_DIR)/Extensions.h \
	$(libhtmlcxx_INC_DIR)/ci_string.h

EXTRA_DIST = gen_tld.pl tld.list
