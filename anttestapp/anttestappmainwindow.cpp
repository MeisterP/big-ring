#include "anttestappmainwindow.h"
#include "ui_anttestappmainwindow.h"

#include <QtCore/QTimer>
namespace indoorcycling
{

AntTestAppMainWindow::AntTestAppMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AntTestAppMainWindow), _timer(new QTimer(this))
{
    ui->setupUi(this);
    connect(_timer, &QTimer::timeout, _timer, [this]() {
        quint16 powerWatts = static_cast<quint16>(ui->spinBox->value());
        emit sensorValue(SensorValueType::POWER_WATT, AntSensorType::POWER, QVariant::fromValue(powerWatts));
    });
    _timer->setInterval(1000);
}

AntTestAppMainWindow::~AntTestAppMainWindow()
{
    delete ui;
}

void AntTestAppMainWindow::antUsbStickFound(bool found)
{
    if (found) {
        ui->searchStickLabel->setText("ANT+ USB Stick found");
        ui->InitializingLabel->setText("Initializing ANT+ Communication");
    } else {
        ui->searchStickLabel->setText("Error: No ANT+ USB Stick found");
    }
}

void AntTestAppMainWindow::initializationFinished(bool success)
{
    ui->pushButton->setEnabled(success);
    if (success) {
        ui->InitializingLabel->setText("Successfully initialized ANT+ USB Stick");
        ui->pushButton->setEnabled(true);
    } else {
        ui->InitializingLabel->setText("Could not Successfully initialize ANT+ USB Stick");
    }
}

void AntTestAppMainWindow::searchTimedOut(AntSensorType channelType)
{
    if (channelType == AntSensorType::HEART_RATE) {
        ui->hrSearchLabel->setText("Search Timed Out");
        ui->pushButton->setEnabled(true);
    }
}

void AntTestAppMainWindow::setSensor(AntSensorType channelType, int deviceNumber)
{
    if (channelType == AntSensorType::HEART_RATE) {
        ui->hrSearchLabel->setText("Sensor Found");
        ui->hrSensorId->setText(QString::number(deviceNumber));
    }
}

void AntTestAppMainWindow::setSensorValue(const SensorValueType valueType, const AntSensorType, const QVariant &sensorValue)
{
    switch(valueType) {
    case SensorValueType::HEARTRATE_BPM:
        ui->currentHrLabel->setText(QString::number(sensorValue.toInt()));
        break;
    case SensorValueType::CADENCE_RPM:
        ui->cadenceLabel->setText(QString::number(sensorValue.toInt()));
        break;
    case SensorValueType::POWER_WATT:
        ui->powerLabel->setText(QString::number(sensorValue.toInt()));
        break;
    default:
        ;// noop
    }

}

void AntTestAppMainWindow::searchStarted(AntSensorType channelType, int)
{
    if (channelType == AntSensorType::HEART_RATE) {
        ui->pushButton->setEnabled(false);
    }
}

void AntTestAppMainWindow::setHeartRate(int bpm)
{
    ui->currentHrLabel->setText(QString("%1 BPM").arg(QString::number(bpm)));
}
}

void indoorcycling::AntTestAppMainWindow::on_pushButton_clicked()
{
    emit startSearch(AntSensorType::HEART_RATE);
}

void indoorcycling::AntTestAppMainWindow::on_pushButton_2_clicked()
{
    emit startSearch(AntSensorType::POWER);
}

void indoorcycling::AntTestAppMainWindow::on_pushButton_3_clicked()
{
    emit openMasterChannel(AntSensorType::POWER);
    _timer->start();
}

void indoorcycling::AntTestAppMainWindow::on_hrMasterPushButton_clicked()
{
    emit openMasterChannel(AntSensorType::HEART_RATE);
    on_hrSpinBox_valueChanged(ui->hrSpinBox->value());
}

void indoorcycling::AntTestAppMainWindow::on_hrSpinBox_valueChanged(int hr)
{
    emit sensorValue(SensorValueType::HEARTRATE_BPM, AntSensorType::HEART_RATE, QVariant::fromValue(hr));
}

void indoorcycling::AntTestAppMainWindow::on_cadenceSpinBox_valueChanged(int arg1)
{
    emit sensorValue(SensorValueType::CADENCE_RPM, AntSensorType::POWER, QVariant::fromValue(arg1));
}

void indoorcycling::AntTestAppMainWindow::on_pushButton_4_clicked()
{
    emit startSearch(AntSensorType::SMART_TRAINER);
}

void indoorcycling::AntTestAppMainWindow::on_doubleSpinBox_valueChanged(double slopeInPercent)
{
    emit slopeChanged(slopeInPercent);
}
