SUBDIRS = src/html

EXTRA_DIST = LGPL_V2 htmlcxx.pc.in

bin_PROGRAMS = htmlcxx
htmlcxx_SOURCES = htmlcxx.cc
htmlcxx_LDADD = src/html/libhtmlcxx.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = htmlcxx.pc
