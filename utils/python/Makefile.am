pythondir = $(prefix)/lib/python1.5
pythonbindir = $(prefix)/lib/python1.5/lib-dynload


INCLUDES = @STRIP_BEGIN@ \
        -I$(top_srcdir) \
        -I$(top_srcdir)/include/newt \
	-I/usr/include/python1.5 \
@STRIP_END@

lib_LTLIBRARIES = libgsnack.la

libgsnack_la_LIBADD = @STRIP_BEGIN@ \
	../common/dialogboxes.lo \
	-lpopt \
	-lgnewt \
@STRIP_END@ 

libgsnack_la_SOURCES = @STRIP_BEGIN@ \
	gsnackmodule.c \
@STRIP_END@

all-am: Makefile $(LTLIBRARIES) _gsnackmodule.so

_gsnackmodule.so: libgsnack.la
	cp -f .libs/libgsnack.so.0.0.0 _gsnackmodule.so
	rm -f .libs/lib* 

CLEANFILES=_gsnackmodule.so

install-libLTLIBRARIES:
	[ -d $(instroot)/$(pythonbindir) ] || \
		install -m 755 -d $(pythonbindir)
	install -m 755 _gsnackmodule.so $(pythonbindir)
	install -m 755 gsnack.py $(pythondir)

	
