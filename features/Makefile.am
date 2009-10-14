noinst_LIBRARIES = librlgo_features.a

librlgo_features_a_SOURCES = \
RlCompoundFeatures.cpp \
RlLocalShape.cpp \
RlLocalShapeFeatures.cpp \
RlLocalShapeSet.cpp \
RlLocalShapeShare.cpp \
RlLocalShapeTracker.cpp \
RlManualFeatures.cpp \
RlProductFeatures.cpp \
RlSharedFeatures.cpp \
RlStageFeatures.cpp \
RlSumFeatures.cpp \
RlToPlayFeatures.cpp

noinst_HEADERS = \
RlCompoundFeatures.h \
RlLocalShape.h \
RlLocalShapeFeatures.h \
RlLocalShapeSet.h \
RlLocalShapeShare.h \
RlLocalShapeTracker.h \
RlManualFeatures.h \
RlProductFeatures.h \
RlSharedFeatures.h \
RlStageFeatures.h \
RlSumFeatures.h \
RlToPlayFeatures.h

librlgo_features_a_CPPFLAGS = \
$(BOOST_CPPFLAGS) \
-I@fuego_dir@ \
-I@fuego_dir@/smartgame \
-I@fuego_dir@/go \
-I@top_srcdir@/utils \
-I@top_srcdir@/rlgo

DISTCLEANFILES = *~
