QT_VERSION = 5
QMAKE_CXXFLAGS += -std=c++11 -W -Wall -Wextra
QT       += core concurrent gui opengl network serialport widgets positioning

VERSION = 1.8.0.0

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

INCLUDEPATH = $$PWD

linux {
    PKGCONFIG += libavcodec libavformat libavutil libswscale libusb-1.0 libusb
    CONFIG += link_pkgconfig
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

