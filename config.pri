# if installed, point to the source directory of google test here.

QT_VERSION = 5
QMAKE_CXXFLAGS += -std=c++11 -W -Wall -Wextra
QT       += core concurrent gui opengl network serialport widgets positioning

VERSION = 1.8.0.0

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

INCLUDEPATH = $$PWD

linux {
    PKGCONFIG += libavcodec libavformat libavutil libswscale libusb-1.0
    CONFIG += link_pkgconfig
}
win32 {
    LIBAV_PATH=C:\development\libav-i686-w64-mingw32-11.2
    LIBAV_DLL_PATH = $$LIBAV_PATH\usr\bin

    INCLUDEPATH += $$LIBAV_PATH\usr\include
    LIBS += $$LIBAV_PATH\usr\bin\avcodec-56.dll
    LIBS += $$LIBAV_PATH\usr\bin\avformat-56.dll
    LIBS += $$LIBAV_PATH\usr\bin\avutil-54.dll
    LIBS += $$LIBAV_PATH\usr\bin\swscale-3.dll

    LIBUSB_PATH = C:\development\libusb-win32-bin-1.2.6.0

    INCLUDEPATH += $${LIBUSB_PATH}\include
    LIBUSB_DLL = $${LIBUSB_PATH}\bin\x86\libusb0_x86.dll
    LIBS += $${LIBUSB_DLL}

    QT_CREATOR_DIR = C:\Qt-5.5\Tools\QtCreator

    CONFIG += openssl-linked
}

#linux:debug {
#    # address sanitizer configuration. Uncomment this to build
#    # with address sanitizer.
#    QMAKE_CXXFLAGS+= -fsanitize=address -fno-omit-frame-pointer
#    QMAKE_CFLAGS+= -fsanitize=address -fno-omit-frame-pointer
#    QMAKE_LFLAGS+= -fsanitize=address
#    LIBS += -Wl,--no-as-needed -lasan -Wl,--as-needed
#}

# profiler configuration. Uncomment this to use google profiler.
#LIBS += -Wl,--no-as-needed -lprofiler -Wl,--as-needed
#LIBS += -Wl,--no-as-needed -ltcmalloc -Wl,--as-needed


