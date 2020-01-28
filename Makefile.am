AUTOMAKE_OPTIONS=foreign
SUBDIRS = src
SUBDIRS += test
CURRENTPATH=$(shell /bin/pwd)
AM_CPPFLAGS = -I$(CURRENTPATH)/include/
AM_CPPFLAGS += -I$(CURRENTPATH)/include/log/
export AM_CPPFLAGS
dist_doc_DATA = README.md