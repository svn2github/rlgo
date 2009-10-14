TESTS = rlgo_unittest
check_PROGRAMS = rlgo_unittest

rlgo_unittest_SOURCES = \
RlActiveSetTest.cpp \
RlEvaluatorTest.cpp \
RlTDTest.cpp \
RlLocalShapeTest.cpp \
RlTestMain.cpp \
RlTestUtil.cpp

noinst_HEADERS = \
RlTestUtil.h

rlgo_unittest_LDFLAGS = $(BOOST_LDFLAGS)

rlgo_unittest_LDADD = \
@fuego_dir@/smartgame/libfuego_smartgame.a \
@fuego_dir@/go/libfuego_go.a \
@fuego_dir@/gouct/libfuego_gouct.a \
@top_builddir@/utils/librlgo_utils.a \
@top_builddir@/rlgo/librlgo_rlgo.a \
@top_builddir@/policies/librlgo_policies.a \
@top_builddir@/learning/librlgo_learning.a \
@top_builddir@/features/librlgo_features.a \
$(BOOST_FILESYSTEM_LIB) \
$(BOOST_SYSTEM_LIB) \
$(BOOST_THREAD_LIB) \
$(BOOST_UNIT_TEST_FRAMEWORK_LIB)

rlgo_unittest_DEPENDENCIES = \
@top_builddir@/utils/librlgo_utils.a \
@top_builddir@/rlgo/librlgo_rlgo.a \
@top_builddir@/policies/librlgo_policies.a \
@top_builddir@/learning/librlgo_learning.a \
@top_builddir@/features/librlgo_features.a

rlgo_unittest_CPPFLAGS = \
-DDATADIR='"@datadir@"' \
-DABS_TOP_SRCDIR='"@abs_top_srcdir@"' \
$(BOOST_CPPFLAGS) \
-I@fuego_dir@ \
-I@fuego_dir@/smartgame \
-I@fuego_dir@/go \
-I@fuego_dir@/gouct \
-I@top_srcdir@/utils \
-I@top_srcdir@/rlgo \
-I@top_srcdir@/policies \
-I@top_srcdir@/learning \
-I@top_srcdir@/features

DISTCLEANFILES = *~
