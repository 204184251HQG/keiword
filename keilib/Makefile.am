AUTOMAKE_OPTIONS=foreign
SUBDIRS = src
KLIBTOPDIR=$(shell cd $(top_srcdir); /bin/pwd;)
export KLIBTOPDIR
AM_CPPFLAGS = -I$(KLIBTOPDIR)/include/
export AM_CPPFLAGS
dist_doc_DATA = README.md