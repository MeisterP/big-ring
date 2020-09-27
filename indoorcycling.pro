TEMPLATE = subdirs
include(config.pri)

SUBDIRS += \
    mainlib \
    big-ring \
    anttestapp \
    test

big-ring.depends = mainlib
anttestapp.depends = mainlib
test.depends = mainlib

RESOURCES += \
    mainlib/icons.qrc \
    big-ring/icons.qrc
