/*
 * Copyright (c) 2015 Ilja Booij (ibooij@gmail.com)
 *
 * This file is part of Big Ring Indoor Video Cycling
 *
 * Big Ring Indoor Video Cycling is free software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Big Ring Indoor Video Cycling  is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Big Ring Indoor Video Cycling.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef ADDSENSORCDIALOG_H
#define ADDSENSORCDIALOG_H

#include <QtCore/QSet>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QDialog>

#include "ant/antsensortype.h"
#include "config/sensorconfiguration.h"

namespace indoorcycling {
class AntCentralDispatch;
}
namespace Ui {
class AddSensorConfigurationDialog;
}

class AddSensorConfigurationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddSensorConfigurationDialog(indoorcycling::AntCentralDispatch* antCentralDispatch,
                                          QWidget *parent = 0);
    virtual ~AddSensorConfigurationDialog();

private slots:
    void fillUsbStickPresentLabel(bool present);
    void sensorFound(indoorcycling::AntSensorType sensorType, int deviceNumber);
    void sensorNotFound(indoorcycling::AntSensorType sensorType);
    void handleSensorValue(const indoorcycling::SensorValueType sensorValueType,
                     const indoorcycling::AntSensorType sensorType,
                     const QVariant& sensorValue);

    void on_searchSensorsButton_clicked();
    void performSearch(indoorcycling::AntSensorType sensorType);
    void on_buttonBox_clicked(QAbstractButton *button);

    void on_lineEdit_textEdited(const QString &arg1);

    void on_directPowerButton_toggled(bool checked);

    void on_virtualPowerButton_toggled(bool checked);

    void on_directSpeedButton_toggled(bool checked);

    void on_fixedPowerButton_toggled(bool checked);

protected:
    virtual void closeEvent(QCloseEvent *) override;
    virtual void hideEvent(QHideEvent *) override;
private:
    void fillVirtualPowerOptions();
    void updateRow(indoorcycling::AntSensorType sensorType, bool found,
                   int deviceNumber = -1);
    void fillSensorTypeRow(indoorcycling::AntSensorType sensorType);
    int rowForSensorType(indoorcycling::AntSensorType);

    void setConfigurationName(const QString& name);
    void saveConfiguration();
    // Checks which simulation options are available and sets options in GUI where appropriate.
    void updateSimulationSettings();
    void updateWheelCircumferenceChooser();

    Ui::AddSensorConfigurationDialog *_ui;
    indoorcycling::AntCentralDispatch* const _antCentralDispatch;
    QSet<indoorcycling::AntSensorType> _currentSearches;
    QString _configurationName;
    QMap<indoorcycling::AntSensorType,indoorcycling::SensorConfiguration> _configurations;
    SimulationSetting _simulationSetting;
};

#endif // ADDSENSORCDIALOG_H
