AUTOMAKE_OPTIONS=foreign
SUBDIRS = src
SUBDIRS += test
KLIBTOPDIR=$(shell cd $(top_srcdir); /bin/pwd;)
export KLIBTOPDIR
AM_CPPFLAGS = -I$(KLIBTOPDIR)/include/
AM_CPPFLAGS += -I$(KLIBTOPDIR)/include/log/
export AM_CPPFLAGS
dist_doc_DATA = README.md