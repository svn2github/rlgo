noinst_LIBRARIES = librlgo_policies.a

librlgo_policies_a_SOURCES = \
RlEpsilonPolicy.cpp \
RlGibbs.cpp \
RlMCPolicy.cpp \
RlPriorityPolicy.cpp \
RlSearch.cpp \
RlTwoStagePolicy.cpp \
RlFuegoPolicy.cpp

noinst_HEADERS = \
RlEpsilonPolicy.h \
RlGibbs.h \
RlMCPolicy.h \
RlPriorityPolicy.h \
RlSearch.h \
RlTwoStagePolicy.h \
RlFuegoPolicy.h

librlgo_policies_a_CPPFLAGS = \
$(BOOST_CPPFLAGS) \
-I@fuego_dir@ \
-I@fuego_dir@/smartgame \
-I@fuego_dir@/go \
-I@fuego_dir@/gouct \
-I@top_srcdir@/utils \
-I@top_srcdir@/rlgo

DISTCLEANFILES = *~
