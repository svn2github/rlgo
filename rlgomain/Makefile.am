bin_PROGRAMS = rlgo

rlgo_SOURCES = \
RlCommands.cpp \
RlEngine.cpp \
RlForceLink.cpp \
RlMain.cpp

noinst_HEADERS = \
RlCommands.h \
RlEngine.h \
RlForceLink.h

rlgo_LDFLAGS = $(BOOST_LDFLAGS)

rlgo_LDADD = \
@fuego_dir@/gouct/libfuego_gouct.a \
@fuego_dir@/go/libfuego_go.a \
@fuego_dir@/smartgame/libfuego_smartgame.a \
@fuego_dir@/gtpengine/libfuego_gtpengine.a \
@top_srcdir@/utils/librlgo_utils.a \
@top_srcdir@/rlgo/librlgo_rlgo.a \
@top_srcdir@/policies/librlgo_policies.a \
@top_srcdir@/learning/librlgo_learning.a \
@top_srcdir@/features/librlgo_features.a \
$(BOOST_FILESYSTEM_LIB) \
$(BOOST_SYSTEM_LIB) \
$(BOOST_THREAD_LIB)

rlgo_DEPENDENCIES = \
@top_srcdir@/utils/librlgo_utils.a \
@top_srcdir@/rlgo/librlgo_rlgo.a \
@top_srcdir@/policies/librlgo_policies.a \
@top_srcdir@/learning/librlgo_learning.a \
@top_srcdir@/features/librlgo_features.a

rlgo_CPPFLAGS = \
-DDATADIR='"@datadir@"' \
-DABS_TOP_SRCDIR='"@abs_top_srcdir@"' \
$(BOOST_CPPFLAGS) \
-I@fuego_dir@ \
-I@fuego_dir@/gtpengine \
-I@fuego_dir@/smartgame \
-I@fuego_dir@/go \
-I@fuego_dir@/gouct \
-I@top_srcdir@/utils \
-I@top_srcdir@/rlgo \
-I@top_srcdir@/policies \
-I@top_srcdir@/learning \
-I@top_srcdir@/features \
-I@top_srcdir@

DISTCLEANFILES = *~
