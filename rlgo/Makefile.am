noinst_LIBRARIES = librlgo_rlgo.a

librlgo_rlgo_a_SOURCES = \
RlAgent.cpp \
RlAgentLog.cpp \
RlAgentPlayer.cpp \
RlBinaryFeatures.cpp \
RlDirtySet.cpp \
RlEvaluator.cpp \
RlFuegoPlayout.cpp \
RlHistory.cpp \
RlLearningRule.cpp \
RlMoveFilter.cpp \
RlPolicy.cpp \
RlSetup.cpp \
RlSimulator.cpp \
RlState.cpp \
RlTex.cpp \
RlTrace.cpp \
RlTracker.cpp \
RlTrainer.cpp \
RlWeight.cpp \
RlWeightSet.cpp

noinst_HEADERS = \
RlActiveSet.h \
RlAgent.h \
RlAgentLog.h \
RlAgentPlayer.h \
RlBinaryFeatures.h \
RlDirtySet.h \
RlEvaluator.h \
RlFuegoPlayout.h \
RlHistory.h \
RlLearningRule.h \
RlMoveFilter.h \
RlPolicy.h \
RlSetup.h \
RlSimulator.h \
RlState.h \
RlTex.h \
RlTrace.h \
RlTracker.h \
RlTrainer.h \
RlWeight.h \
RlWeightSet.h

librlgo_rlgo_a_CPPFLAGS = \
$(BOOST_CPPFLAGS) \
-I@fuego_dir@ \
-I@fuego_dir@/smartgame \
-I@fuego_dir@/go \
-I@fuego_dir@/gouct \
-I@top_srcdir@/utils

DISTCLEANFILES = *~
