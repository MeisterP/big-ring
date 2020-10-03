TEMPLATE = lib

include(../config.pri)

RESOURCES += \
    $$IN_PWD/video/shaders.qrc \
    $$IN_PWD/icons.qrc

target = mainlib
target.path = ../lib
CONFIG += staticlib
INCLUDEPATH += thirdparty/include

ANT_HEADERS += \
    ant/antdevice.h \
    ant/antdevicefinder.h \
    ant/antmessagegatherer.h \
    ant/usb2antdevice.h \
    ant/antmessage2.h \
    ant/antcentraldispatch.h \
    ant/antchannelhandler.h \
    ant/antsensortype.h \
    ant/antheartratechannelhandler.h \
    ant/antpowerchannelhandler.h \
    ant/antsmarttrainerchannelhandler.h \
    ant/antspeedandcadencechannelhandler.h \
    ant/unixserialusbant.h \

ANT_SOURCES += \
    ant/antdevice.cpp \
    ant/antdevicefinder.cpp \
    ant/antmessagegatherer.cpp \
    ant/usb2antdevice.cpp \
    ant/antmessage2.cpp \
    ant/antcentraldispatch.cpp \
    ant/antchannelhandler.cpp \
    ant/antheartratechannelhandler.cpp \
    ant/antpowerchannelhandler.cpp \
    ant/antsmarttrainerchannelhandler.cpp \
    ant/antspeedandcadencechannelhandler.cpp \
    ant/unixserialusbant.cpp \

linux {
    ANT_SOURCES += thirdparty/libusb-compat/core.c
    ANT_HEADERS += thirdparty/libusb-compat/usb.h \
                   thirdparty/libusb-compat/usbi.h
}

CONFIG_HEADERS += \
    config/bigringsettings.h \
    config/sensorconfiguration.h

CONFIG_SOURCES += \
    config/bigringsettings.cpp \
    config/sensorconfiguration.cpp

GENERALGUI_HEADERS += \
    generalgui/profilepainter.h \
    generalgui/quantityprinter.h

GENERALGUI_SOURCES += \
    generalgui/profilepainter.cpp \
    generalgui/quantityprinter.cpp

IMPORTER_HEADERS += \
    importer/gpxfileparser.h \
    importer/virtualtrainingfileparser.h \
    importer/reallifevideocache.h \
    importer/reallifevideoimporter.h \
    importer/rlvfileparser.h

IMPORTER_SOURCES += \
    importer/gpxfileparser.cpp \
    importer/virtualtrainingfileparser.cpp \
    importer/reallifevideocache.cpp \
    importer/reallifevideoimporter.cpp \
    importer/rlvfileparser.cpp

MAINGUI_HEADERS +=\
    maingui/addsensorconfigurationdialog.h \
    maingui/altitudeprofilewidget.h \
    maingui/createnewcoursedialog.h \
    maingui/mainwindow.h \
    maingui/videoitemdelegate.h \
    maingui/videolistmodel.h \
    maingui/videolistview.h \
    maingui/settingsdialog.h \
    maingui/videodetails.h \
    maingui/videoqlistview.h \
    maingui/videoscreenshotwidget.h

MAINGUI_SOURCES +=\
    maingui/addsensorconfigurationdialog.cpp \
    maingui/altitudeprofilewidget.cpp \
    maingui/createnewcoursedialog.cpp \
    maingui/mainwindow.cpp \
    maingui/videoitemdelegate.cpp \
    maingui/videolistmodel.cpp \
    maingui/videolistview.cpp \
    maingui/settingsdialog.cpp \
    maingui/videodetails.cpp \
    maingui/videoqlistview.cpp \
    maingui/videoscreenshotwidget.cpp


MAINGUI_FORMS +=\
    maingui/aboutdialog.ui \
    maingui/addsensorconfigurationdialog.ui \
    maingui/altitudeprofilewidget.ui \
    maingui/createnewcoursedialog.ui \
    maingui/settingsdialog.ui \
    maingui/videodetails.ui \
    maingui/videoscreenshotwidget.ui

MODEL_HEADERS += \
    model/cyclist.h \
    model/distanceentrycollection.h \
    model/distancemappingentry.h \
    model/geoposition.h \
    model/profile.h \
    model/reallifevideo.h \
    model/ridefile.h \
    model/ridesampler.h \
    model/rollingaveragecalculator.h \
    model/simulation.h \
    model/unitconverter.h \
    model/videoinformation.h \
    model/virtualpower.h

MODEL_SOURCES += \
    model/cyclist.cpp \
    model/distancemappingentry.cpp \
    model/geoposition.cpp \
    model/profile.cpp \
    model/reallifevideo.cpp \
    model/ridefile.cpp \
    model/ridesampler.cpp \
    model/rollingaveragecalculator.cpp \
    model/simulation.cpp \
    model/unitconverter.cpp \
    model/videoinformation.cpp \
    model/virtualpower.cpp

NETWORK_HEADERS += \
    network/versionchecker.h

NETWORK_SOURCES += \
    network/versionchecker.cpp

RIDE_HEADERS += \
    ride/actuators.h \
    ride/ridefilewriter.h \
    ride/sensors.h

RIDE_SOURCES += \
    ride/actuators.cpp \
    ride/ridefilewriter.cpp \
    ride/sensors.cpp

RIDEGUI_HEADERS += \
    ridegui/clockgraphicsitem.h \
    ridegui/informationboxgraphicsitem.h \
    ridegui/messagepanelitem.h \
    ridegui/newvideowidget.h \
    ridegui/profileitem.h \
    ridegui/rollingaveragesensoritem.h \
    ridegui/run.h \
    ridegui/sensoritem.h

RIDEGUI_SOURCES += \
    ridegui/clockgraphicsitem.cpp \
    ridegui/informationboxgraphicsitem.cpp \
    ridegui/messagepanelitem.cpp \
    ridegui/newvideowidget.cpp \
    ridegui/profileitem.cpp \
    ridegui/rollingaveragesensoritem.cpp \
    ridegui/run.cpp \
    ridegui/sensoritem.cpp

UTIL_HEADERS += \
    util/screensaverblocker.h \
    util/util.h

UTIL_SOURCES += \
    util/screensaverblocker.cpp

VIDEO_HEADERS += \
    video/framebuffer.h \
    video/genericvideoreader.h \
    video/openglpainter2.h \
    video/thumbnailcreatingvideoreader.h \
    video/framecopyingvideoreader.h \
    video/thumbnailer.h \
    video/videoinforeader.h \
    video/videoplayer.h

VIDEO_SOURCES += \
    video/genericvideoreader.cpp \
    video/openglpainter2.cpp \
    video/thumbnailcreatingvideoreader.cpp \
    video/framecopyingvideoreader.cpp \
    video/thumbnailer.cpp \
    video/videoinforeader.cpp \
    video/videoplayer.cpp


HEADERS += \
    $$ANT_HEADERS \
    $$CONFIG_HEADERS \
    $$GENERALGUI_HEADERS \
    $$IMPORTER_HEADERS \
    $$MAINGUI_HEADERS \
    $$MODEL_HEADERS \
    $$NETWORK_HEADERS \
    $$RIDE_HEADERS \
    $$RIDEGUI_HEADERS \
    $$UTIL_HEADERS \
    $$VIDEO_HEADERS

SOURCES += \
    $$ANT_SOURCES \
    $$CONFIG_SOURCES \
    $$GENERALGUI_SOURCES \
    $$IMPORTER_SOURCES \
    $$MAINGUI_SOURCES \
    $$MODEL_SOURCES \
    $$NETWORK_SOURCES \
    $$RIDE_SOURCES \
    $$RIDEGUI_SOURCES \
    $$UTIL_SOURCES \
    $$VIDEO_SOURCES

FORMS += \
    $$MAINGUI_FORMS

LIBS +=  -lusb-1.0

