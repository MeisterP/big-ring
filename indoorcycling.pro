TEMPLATE = subdirs
CONFIG += debug
include(config.pri)

SUBDIRS += \
    mainlib \
    indoorcycling2 \
    antlib \


#!isEmpty(GOOGLE_TEST_SOURCE_DIR) {
#    SUBDIRS += test google_test
#    test.depends = mainlib antlib indoorcycling1 indoorcycling2 google_test
#}

indoorcycling2.depends = antlib mainlib

