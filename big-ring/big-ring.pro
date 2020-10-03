TARGET = ../bin/big-ring
TEMPLATE = app

include(../config.pri)

SOURCES += main.cpp

RC_ICONS = BigRingIcon.ico

LIBS += -L$$OUT_PWD/../mainlib/ -lmainlib

INCLUDEPATH += $$PWD/../mainlib
DEPENDPATH += $$PWD/../mainlib

PRE_TARGETDEPS += $$OUT_PWD/../mainlib/libmainlib.a

