
INCLUDES = @STRIP_BEGIN@ \
        -I$(top_srcdir) \
        -I$(top_srcdir)/include/newt \
@STRIP_END@

lib_LTLIBRARIES = libgnewtcl.la

libgnewtcl_la_LIBADD = @STRIP_BEGIN@ \
	../common/dialogboxes.lo \
	-L$(top_srcdir)/src/gtk/.libs \
	-lgnewt	 \
	-lpopt \
	-ltcl \
	-lm \
@STRIP_END@ 

libgnewtcl_la_SOURCES = @STRIP_BEGIN@ \
	gwhiptcl.c \
@STRIP_END@

all-am: Makefile $(LTLIBRARIES) gwhiptcl.so

gwhiptcl.so: libgnewtcl.la
	cp -f .libs/libgnewtcl.so.0.0.0 gwhiptcl.so
	rm -f .libs/lib* 

CLEANFILES=gwhiptcl.so

install-libLTLIBRARIES:
	install -m 755 gwhiptcl.so $(libdir)
	
