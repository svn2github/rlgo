noinst_LIBRARIES = librlgo_learning.a

librlgo_learning_a_SOURCES = \
RlMCRules.cpp \
RlTDRules.cpp

noinst_HEADERS = \
RlMCRules.h \
RlTDRules.h

librlgo_learning_a_CPPFLAGS = \
$(BOOST_CPPFLAGS) \
-I@fuego_dir@ \
-I@fuego_dir@/smartgame \
-I@fuego_dir@/go \
-I@top_srcdir@/utils \
-I@top_srcdir@/rlgo \
-I@top_srcdir@

DISTCLEANFILES = *~
