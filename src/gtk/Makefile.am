

	
INCLUDES = @STRIP_BEGIN@ \
	-I$(top_srcdir) \
	-I$(top_srcdir)/include/newt \
	-I$(top_srcdir)/include/gnewt \
@STRIP_END@

lib_LTLIBRARIES = libgnewt.la 

libgnewt_la_LDFLAGS = @STRIP_BEGIN@ \
	-version-info $(MICRO_VERSION) \
	-release $(MAJOR_VERSION).$(MINOR_VERSION) \
@STRIP_END@

libgnewt_la_LIBADD = @STRIP_BEGIN@ \
        @top_srcdir@/src/common/buttonbar.lo \
        @top_srcdir@/src/common/scrollbar.lo \
        @top_srcdir@/src/common/grid.lo \
        @top_srcdir@/src/common/windows.lo \
@STRIP_END@

libgnewt_la_SOURCES = @STRIP_BEGIN@ \
	button.c \
	checkbox.c \
	checkboxtree.c \
	entry.c \
	form.c \
	label.c \
	listbox.c \
	newt.c \
	scale.c \
	textbox.c \
	extended.c \
@STRIP_END@

	

