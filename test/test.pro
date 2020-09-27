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
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../mainlib/release/ -lmainlib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../mainlib/debug/ -lmainlib
else:unix: LIBS += -L$$OUT_PWD/../mainlib/ -lmainlib

INCLUDEPATH += $$PWD/../mainlib
DEPENDPATH += $$PWD/../mainlib

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../mainlib/release/libmainlib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../mainlib/debug/libmainlib.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../mainlib/release/mainlib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../mainlib/debug/mainlib.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../mainlib/libmainlib.a

