#-------------------------------------------------
#
# Project created by QtCreator 2014-09-08T14:25:01
#
#-------------------------------------------------

QT       -= core gui

include(../config.pri)
TARGET = google_test
TEMPLATE = lib
CONFIG += staticlib

DEFINES += GOOGLE_TEST_LIBRARY

SOURCES += $$GOOGLE_TEST_SOURCE_DIR/src/gtest-all.cc
INCLUDEPATH += $$GOOGLE_TEST_SOURCE_DIR

