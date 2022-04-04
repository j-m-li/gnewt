INCLUDES = @STRIP_BEGIN@ \
        -I$(top_srcdir) \
        -I$(top_srcdir)/include/newt \
        -I$(top_srcdir)/include/gnewt \
@STRIP_END@


noinst_PROGRAMS = test test_ja testtree testgrid

test_SOURCES = test.c
#test_LDADD = ../src/gtk/.libs/libgnewt.a /usr/lib/libefence.a
test_LDADD = -lnewt

test_ja_SOURCES = test_ja.c
test_ja_LDADD = ../src/gtk/.libs/libgnewt.a
#test_ja_LDADD = -lgnewt

testtree_SOURCES = testtree.c
testtree_LDADD = ../src/gtk/.libs/libgnewt.a

testgrid_SOURCES = testgrid.c
testgrid_LDADD = ../src/gtk/.libs/libgnewt.a


