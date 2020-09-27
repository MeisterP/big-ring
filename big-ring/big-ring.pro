#-------------------------------------------------
#
# Project created by QtCreator 2014-10-26T17:17:44
#
#-------------------------------------------------

TARGET = ../bin/big-ring
TEMPLATE = app

include(../config.pri)

SOURCES += main.cpp

RC_ICONS = BigRingIcon.ico

# added by QtCreator to add mainlib to dependencies.
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

win32 {
    DEPLOY_COMMAND = windeployqt
    TARGET_CUSTOM_EXT = .exe
    CONFIG( debug, debug|release ) {
        # debug
        DEPLOY_TARGET = $$shell_quote($$shell_path($${OUT_PWD}/debug/$${TARGET}$${TARGET_CUSTOM_EXT}))
    } else {
        # release
        DEPLOY_TARGET = $$shell_quote($$shell_path($${OUT_PWD}/release/$${TARGET}$${TARGET_CUSTOM_EXT}))
    }
    warning($${DEPLOY_COMMAND} $${DEPLOY_TARGET})

    # TODO Replace these with some (Batch) scripts. That would probably make it easier to work
    # with.
    QMAKE_PRE_LINK = if not exist \"$${OUT_PWD}\\bin\" md \"$${OUT_PWD}\\bin\"
    QMAKE_POST_LINK = copy /y \"$${LIBAV_DLL_PATH}\\*.dll\" \"$${OUT_PWD}\\bin\\\" && \
                     copy /y \"$${LIBUSB_DLL}\" \"$${OUT_PWD}\\bin\\libusb0.dll\" && \
                     copy /y \"$${QT_CREATOR_DIR}\\bin\\libeay32.dll\" \"$${OUT_PWD}\\bin\\libeay32.dll\" && \
                     copy /y \"$${QT_CREATOR_DIR}\\bin\\ssleay32.dll\" \"$${OUT_PWD}\\bin\\ssleay32.dll\" && \
                   $${DEPLOY_COMMAND} $${DEPLOY_TARGET}

}

