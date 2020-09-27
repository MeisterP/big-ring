#ifndef ANTTESTAPPMAINWINDOW_H
#define ANTTESTAPPMAINWINDOW_H

#include <QtWidgets/QMainWindow>

#include "ant/antsensortype.h"

namespace Ui {
class AntTestAppMainWindow;
}

namespace indoorcycling {
class AntTestAppMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AntTestAppMainWindow(QWidget *parent = 0);
    ~AntTestAppMainWindow();
public slots:
    void antUsbStickFound(bool found);
    void initializationFinished(bool success);
    void searchTimedOut(AntSensorType channelType);
    void setSensor(AntSensorType channelType, int deviceNumber);
    void setSensorValue(const SensorValueType valueType, const AntSensorType,
                     const QVariant& sensorValue);
    void searchStarted(AntSensorType channelType, int deviceNumber);
    void setHeartRate(int bpm);
signals:
    void startSearch(AntSensorType channelType);
    void openMasterChannel(AntSensorType sensorType);
    void sensorValue(const SensorValueType sensorValueType, const AntSensorType sensorType, const QVariant& value);
    void slopeChanged(qreal slopeInPercent);
private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_hrMasterPushButton_clicked();

    void on_hrSpinBox_valueChanged(int arg1);

    void on_cadenceSpinBox_valueChanged(int arg1);

    void on_pushButton_4_clicked();

    void on_doubleSpinBox_valueChanged(double arg1);

private:
    Ui::AntTestAppMainWindow *ui;
    QTimer* _timer;
};
}
#endif // ANTTESTAPPMAINWINDOW_H
