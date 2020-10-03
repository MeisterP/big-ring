TEMPLATE = app
CONFIG += testcase
include(../config.pri)
QT += testlib
TARGET = tests

SOURCES += \
    antmessage2test.cpp \
    main.cpp \
    virtualpowertest.cpp \
    virtualtrainingfileparsertest.cpp \
    profiletest.cpp \
    rollingaveragecalculatortest.cpp \
    reallifevideocachetest.cpp \
    ridefilewritertest.cpp \
    distanceentrycollectiontest.cpp

HEADERS += \
    antmessage2test.h \
    common.h \
    virtualpowertest.h \
    virtualtrainingfileparsertest.h \
    profiletest.h \
    rollingaveragecalculatortest.h \
    reallifevideocachetest.h \
    ridefilewritertest.h \
    distanceentrycollectiontest.h


RESOURCES += \
    testfiles.qrc

# dependency on mainlib
LIBS += -L$$OUT_PWD/../mainlib/ -lmainlib

INCLUDEPATH += $$PWD/../mainlib
DEPENDPATH += $$PWD/../mainlib

PRE_TARGETDEPS += $$OUT_PWD/../mainlib/libmainlib.a

