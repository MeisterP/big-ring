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
#include "addsensorconfigurationdialog.h"
#include "ui_addsensorconfigurationdialog.h"
#include "virtualpower.h"

#include <QtCore/QtDebug>
#include <QtGui/QCloseEvent>
#include <QtGui/QHideEvent>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>

using indoorcycling::AntCentralDispatch;
using indoorcycling::AntSensorType;
using indoorcycling::NamedSensorConfigurationGroup;
using indoorcycling::SimulationSetting;

namespace
{
const char* SEARCHING = "Searching";
const char* FOUND = "Found";
const char* NOT_FOUND = "Not Found";

const int sensorTypeRole = Qt::UserRole + 1;
const int sensorDeviceNumberRole = sensorTypeRole + 1;
const int virtualPowerRole = sensorDeviceNumberRole + 1;

enum class SearchTableColumn {
    NAME,
    BUTTON,
    STATE,
    DEVICE_NUMBER,
    VALUE
};
int columnNumber(SearchTableColumn column) {
    return static_cast<int>(column);
}
}
AddSensorConfigurationDialog::AddSensorConfigurationDialog(
        AntCentralDispatch* antCentralDispatch, QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::AddSensorConfigurationDialog),
    _antCentralDispatch(antCentralDispatch)
{
    _ui->setupUi(this);
    _ui->searchTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    updateSimulationSettings();
    fillVirtualPowerOptions();
    _ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
    fillSensorTypeRow(indoorcycling::SENSOR_TYPE_HR);
    fillSensorTypeRow(indoorcycling::SENSOR_TYPE_POWER);
    fillSensorTypeRow(indoorcycling::SENSOR_TYPE_SPEED_AND_CADENCE);
    fillSensorTypeRow(indoorcycling::SENSOR_TYPE_CADENCE);
    fillSensorTypeRow(indoorcycling::SENSOR_TYPE_SPEED);

    connect(_antCentralDispatch, &AntCentralDispatch::sensorFound, this,
            &AddSensorConfigurationDialog::sensorFound);
    connect(_antCentralDispatch, &AntCentralDispatch::sensorNotFound, this,
            &AddSensorConfigurationDialog::sensorNotFound);
    connect(_antCentralDispatch, &AntCentralDispatch::sensorValue, this,
            &AddSensorConfigurationDialog::handleSensorValue);

}

AddSensorConfigurationDialog::~AddSensorConfigurationDialog()
{
    delete _ui;
}

void AddSensorConfigurationDialog::fillSensorTypeRow(indoorcycling::AntSensorType sensorType)
{
    int row = _ui->searchTableWidget->rowCount();
    _ui->searchTableWidget->setRowCount(row + 1);
    QTableWidgetItem* nameColumn = new QTableWidgetItem(
                indoorcycling::ANT_SENSOR_TYPE_STRINGS[sensorType]);
    nameColumn->setData(sensorTypeRole, QVariant::fromValue(static_cast<int>(sensorType)));
    _ui->searchTableWidget->setItem(row, columnNumber(SearchTableColumn::NAME), nameColumn);
    QPushButton* button = new QPushButton(tr("Search"));
    connect(button, &QPushButton::clicked, button, [this, sensorType]() {
        performSearch(sensorType);
    });
    _ui->searchTableWidget->setCellWidget(row, columnNumber(SearchTableColumn::BUTTON), button);
    QProgressBar* bar = new QProgressBar;
    bar->setRange(0, 0);
    bar->setMaximum(10);
    bar->setFormat(tr(NOT_FOUND));
    _ui->searchTableWidget->setCellWidget(row, columnNumber(SearchTableColumn::STATE), bar);

    QTableWidgetItem* deviceNumberColumn = new QTableWidgetItem("-");
    _ui->searchTableWidget->setItem(row, columnNumber(SearchTableColumn::DEVICE_NUMBER),
                                    deviceNumberColumn);
    QTableWidgetItem* currentValueColumn = new QTableWidgetItem("-");
    _ui->searchTableWidget->setItem(row, columnNumber(SearchTableColumn::VALUE),
                                    currentValueColumn);
}

int AddSensorConfigurationDialog::rowForSensorType(indoorcycling::AntSensorType typeToFind)
{
    for (int row = 0; row < _ui->searchTableWidget->rowCount(); ++row) {
        QTableWidgetItem* nameItem = _ui->searchTableWidget->item(row, 0);
        indoorcycling::AntSensorType sensorType =
                static_cast<indoorcycling::AntSensorType>(nameItem->data(sensorTypeRole).toInt());
        if (sensorType == typeToFind) {
            return row;
        }
    }
    return -1;
}

void AddSensorConfigurationDialog::setConfigurationName(const QString &name)
{
    _configurationName = name;
    _ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(!_configurationName.isEmpty());
}

void AddSensorConfigurationDialog::saveConfiguration()
{
    indoorcycling::NamedSensorConfigurationGroup group(_configurationName,
                                                       _configurations,
                                                       _simulationSetting);
    if (_simulationSetting == SimulationSetting::FIXED_POWER) {
        int fixedPower = _ui->powerSpinBox->value();
        group.setFixedPower(fixedPower);
    } else if (_simulationSetting == SimulationSetting::VIRTUAL_POWER) {
        indoorcycling::VirtualPowerTrainer trainer =
                static_cast<indoorcycling::VirtualPowerTrainer>(
                    _ui->virtualPowerChooser->currentData().toInt());
        group.setTrainer(trainer);
    }

    indoorcycling::NamedSensorConfigurationGroup::addNamedSensorConfigurationGroup(group);

    // Make sure this new configuration is automatically selected.
    NamedSensorConfigurationGroup::saveSelectedConfigurationGroup(_configurationName);
}

void AddSensorConfigurationDialog::updateSimulationSettings()
{
    bool powerSensorPresent = _configurations.contains(AntSensorType::SENSOR_TYPE_POWER);
    bool speedSensorPresent = _configurations.contains(AntSensorType::SENSOR_TYPE_SPEED) ||
            _configurations.contains(AntSensorType::SENSOR_TYPE_SPEED_AND_CADENCE);
    _ui->directPowerButton->setEnabled(powerSensorPresent);
    _ui->directSpeedButton->setEnabled(speedSensorPresent);
    _ui->virtualPowerButton->setEnabled(speedSensorPresent);
    _ui->fixedPowerButton->setEnabled(true);

    if (powerSensorPresent) {
        _ui->directPowerButton->setChecked(true);
    } else if (speedSensorPresent) {
        _ui->virtualPowerButton->setChecked(true);
    } else {
        _ui->fixedPowerButton->setChecked(true);
        _ui->powerSpinBox->setEnabled(true);
    }

}

void AddSensorConfigurationDialog::on_searchSensorsButton_clicked()
{
    _antCentralDispatch->closeAllChannels();
    _ui->searchSensorsButton->setEnabled(false);
    performSearch(indoorcycling::SENSOR_TYPE_HR);
    performSearch(indoorcycling::SENSOR_TYPE_POWER);
    performSearch(indoorcycling::SENSOR_TYPE_SPEED_AND_CADENCE);
    performSearch(indoorcycling::SENSOR_TYPE_CADENCE);
    performSearch(indoorcycling::SENSOR_TYPE_SPEED);
}

void AddSensorConfigurationDialog::sensorFound(indoorcycling::AntSensorType sensorType, int deviceNumber)
{
    _configurations[sensorType] = indoorcycling::SensorConfiguration(sensorType, deviceNumber);
    updateRow(sensorType, true, deviceNumber);
    updateSimulationSettings();
}

void AddSensorConfigurationDialog::sensorNotFound(indoorcycling::AntSensorType sensorType)
{
    updateRow(sensorType, false);
}

void AddSensorConfigurationDialog::handleSensorValue(const indoorcycling::SensorValueType sensorValueType, const indoorcycling::AntSensorType sensorType, const QVariant &sensorValue)
{
    int row = rowForSensorType(sensorType);
    if (row >= 0) {
        QTableWidgetItem* item = _ui->searchTableWidget->item(row, columnNumber(SearchTableColumn::VALUE));
        QString text = QString("%1 %2").arg(QString::number(sensorValue.toInt()))
                .arg(indoorcycling::SENSOR_VALUE_TYPE_STRINGS[sensorValueType]);
        item->setText(text);
    }
}

void AddSensorConfigurationDialog::performSearch(indoorcycling::AntSensorType sensorType)
{
    _ui->simulationSettingsGroupBox->setEnabled(false);
    int row = rowForSensorType(sensorType);
    if (row >= 0) {
        _configurations.remove(sensorType);
        // disable push button
        _ui->searchTableWidget->cellWidget(row, columnNumber(SearchTableColumn::BUTTON))
                ->setEnabled(false);
        // start search bar
        QProgressBar* bar = static_cast<QProgressBar*>(
                    _ui->searchTableWidget->cellWidget(row, columnNumber(SearchTableColumn::STATE)));
        bar->setRange(0, 0);
        bar->setFormat(tr(SEARCHING));
        bar->setTextVisible(true);
        bar->setValue(0);

        _currentSearches.insert(sensorType);
        _antCentralDispatch->searchForSensorType(sensorType);
    }
}

void AddSensorConfigurationDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    switch(_ui->buttonBox->buttonRole(button)) {
    case QDialogButtonBox::AcceptRole:
        qDebug() << "Save";
        saveConfiguration();
        QDialog::accept();
        break;
    default:
        QDialog::reject();
        break;
    }
}

void AddSensorConfigurationDialog::on_lineEdit_textEdited(const QString &text)
{
    setConfigurationName(text);
}

void AddSensorConfigurationDialog::updateRow(indoorcycling::AntSensorType sensorType,
                                             bool found, int deviceNumber)
{
    int row = rowForSensorType(sensorType);
    if (row >= 0) {
        _currentSearches.remove(sensorType);
        _ui->searchTableWidget->cellWidget(row, columnNumber(SearchTableColumn::BUTTON))
                ->setEnabled(true);
        QProgressBar* bar = static_cast<QProgressBar*>(
                    _ui->searchTableWidget->cellWidget(row, columnNumber(SearchTableColumn::STATE)));
        bar->setMaximum(10);
        if (found) {
            bar->setFormat(tr(FOUND));
        } else {
            bar->setFormat(tr(NOT_FOUND));
        }

        QTableWidgetItem* const deviceNumberItem =
                _ui->searchTableWidget->item(row, columnNumber(SearchTableColumn::DEVICE_NUMBER));
        if (found) {
            deviceNumberItem->setText(QString::number(deviceNumber));
            _ui->searchTableWidget->item(row, columnNumber(SearchTableColumn::NAME))
                    ->setData(sensorDeviceNumberRole, QVariant::fromValue(deviceNumber));
        } else {
            deviceNumberItem->setText("-");
        }
    }
    if (_currentSearches.isEmpty()) {
        _ui->searchSensorsButton->setEnabled(true);
        _ui->simulationSettingsGroupBox->setEnabled(true);
    }
}

void AddSensorConfigurationDialog::on_directPowerButton_toggled(bool checked)
{
    if (checked) {
        _simulationSetting = SimulationSetting::DIRECT_POWER;
    }
}

void AddSensorConfigurationDialog::on_virtualPowerButton_toggled(bool checked)
{
    if (checked) {
        _simulationSetting = SimulationSetting::VIRTUAL_POWER;
    }
}

void AddSensorConfigurationDialog::on_directSpeedButton_toggled(bool checked)
{
    if (checked) {
        _simulationSetting = SimulationSetting::DIRECT_SPEED;
    }
}

void AddSensorConfigurationDialog::on_fixedPowerButton_toggled(bool checked)
{
    if (checked) {
        _simulationSetting = SimulationSetting::FIXED_POWER;
    }
}

void AddSensorConfigurationDialog::closeEvent(QCloseEvent *closeEvent)
{
    qDebug() << "Close Event called";
    if (_antCentralDispatch->areAllChannelsClosed()) {
        qDebug() << "All ANT+ channels are closed. Closing dialog.";
        closeEvent->accept();
    } else {
        qDebug() << "Not all ANT+ channels are closed. Waiting for them to be closed.";
        connect(_antCentralDispatch, &AntCentralDispatch::allChannelsClosed,
                this, [this]() {
            close();
        });
        _antCentralDispatch->closeAllChannels();
        closeEvent->ignore();
    }
}

void AddSensorConfigurationDialog::hideEvent(QHideEvent *hideEvent)
{
    if (!isVisible()) {
        if (_antCentralDispatch->areAllChannelsClosed()) {
            qDebug() << "All ANT+ channels are closed. Hiding dialog.";
            hideEvent->accept();
        } else {
            qDebug() << "Not all ANT+ channels are closed. Waiting for them to be closed.";
            connect(_antCentralDispatch, &AntCentralDispatch::allChannelsClosed,
                    this, [this]() {
                hide();
            });
            _antCentralDispatch->closeAllChannels();
            hideEvent->ignore();
        }
    }
}

void AddSensorConfigurationDialog::fillVirtualPowerOptions()
{
    for(indoorcycling::VirtualPowerTrainer trainer: indoorcycling::VIRTUAL_POWER_TRAINERS.keys()) {
        const QString& name = indoorcycling::VIRTUAL_POWER_TRAINERS[trainer];

        _ui->virtualPowerChooser->addItem(name, QVariant::fromValue(
                                              static_cast<int>(trainer)));
    }
}
