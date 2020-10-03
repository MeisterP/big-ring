TARGET = ../bin/anttestapp
TEMPLATE = app

include(../config.pri)


SOURCES += anttestapp.cpp \
    anttestappmainwindow.cpp

LIBS += -L$$OUT_PWD/../mainlib/ -lmainlib

INCLUDEPATH += $$PWD/../mainlib
DEPENDPATH += $$PWD/../mainlib

PRE_TARGETDEPS += $$OUT_PWD/../mainlib/libmainlib.a

FORMS += \
    anttestappmainwindow.ui

HEADERS += \
    anttestappmainwindow.h

