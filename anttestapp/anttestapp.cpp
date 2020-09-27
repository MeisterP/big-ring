#include <QtWidgets/QApplication>
#include <QtWidgets/QStyleFactory>
#include <QtCore/QtDebug>
#include "ant/antsensortype.h"
#include "ant/antcentraldispatch.h"
#include "anttestappmainwindow.h"

using indoorcycling::AntCentralDispatch;
using indoorcycling::AntTestAppMainWindow;
int main(int argc, char** argv)
{
    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("Fusion"));

    AntTestAppMainWindow mainWindow;
    AntCentralDispatch acd;

    QObject::connect(&acd, &AntCentralDispatch::antUsbStickScanningFinished, &mainWindow,
                     &AntTestAppMainWindow::antUsbStickFound);
    QObject::connect(&acd, &AntCentralDispatch::initializationFinished, &mainWindow,
                     &AntTestAppMainWindow::initializationFinished);
//    QObject::connect(&acd, &AntCentralDispatch::initializationFinished, &acd, [&acd](bool success) {
//        if (success) {
//            acd.searchForSensorType(indoorcycling::CHANNEL_TYPE_HR);
//        }
//    });
    QObject::connect(&mainWindow, &AntTestAppMainWindow::startSearch, &acd, &AntCentralDispatch::searchForSensorType);
    QObject::connect(&mainWindow, &AntTestAppMainWindow::openMasterChannel, &acd, &AntCentralDispatch::openMasterChannel);
    QObject::connect(&mainWindow, &AntTestAppMainWindow::sensorValue, &acd, &AntCentralDispatch::sendSensorValue);
    QObject::connect(&mainWindow, &AntTestAppMainWindow::slopeChanged, &acd, &AntCentralDispatch::setSlope);

    QObject::connect(&acd, &AntCentralDispatch::searchStarted, &mainWindow, &AntTestAppMainWindow::searchStarted);
    QObject::connect(&acd, &AntCentralDispatch::sensorNotFound, &mainWindow,
                     &AntTestAppMainWindow::searchTimedOut);
    QObject::connect(&acd, &AntCentralDispatch::sensorFound, &mainWindow,
                     &AntTestAppMainWindow::setSensor);
    QObject::connect(&acd, &AntCentralDispatch::sensorValue, &mainWindow,
                     &AntTestAppMainWindow::setSensorValue);
    acd.initialize();

    mainWindow.show();
    return a.exec();
}
