/*
 * Copyright (c) 2012-2015 Ilja Booij (ibooij@gmail.com)
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

#include "run.h"
#include "newvideowidget.h"
#include "ant/antcentraldispatch.h"
#include "config/bigringsettings.h"
#include "config/sensorconfiguration.h"
#include "model/ridesampler.h"
#include "ride/actuators.h"
#include "ride/ridefilewriter.h"
#include "ride/sensors.h"

#include <QtCore/QTimer>
#include <QtCore/QtDebug>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>

using indoorcycling::Actuators;
using indoorcycling::AntCentralDispatch;
using indoorcycling::NamedSensorConfigurationGroup;
using indoorcycling::Sensors;

Run::Run(indoorcycling::AntCentralDispatch *antCentralDispatch, RealLifeVideo& rlv, Course& course, QObject* parent) :
    QObject(parent), _antCentralDispatch(antCentralDispatch), _rlv(rlv), _course(course), _state(State::BEFORE_START)
{
    BigRingSettings settings;
   _cyclist = new Cyclist(settings.userWeight(), settings.bikeWeight(), this);

   connect(_cyclist, &Cyclist::distanceChanged, this, &Run::distanceChanged);
   connect(_cyclist, &Cyclist::speedChanged, this, &Run::speedChanged);

    NamedSensorConfigurationGroup sensorConfigurationGroup =
            NamedSensorConfigurationGroup::selectedConfigurationGroup();

    const int powerForElevationCorrectionPercentage = settings.powerForElevationCorrection();
   _simulation = new Simulation(sensorConfigurationGroup.simulationSetting(), *_cyclist, powerForElevationCorrectionPercentage * 0.01, this);

    _simulation->rlvSelected(rlv);
    _simulation->courseSelected(course);

    indoorcycling::Sensors* sensors = new indoorcycling::Sensors(_antCentralDispatch,
                                                                 sensorConfigurationGroup,
                                                                 this);

    connect(sensors, &Sensors::heartRateBpmMeasured, _simulation, &Simulation::setHeartRate);
    connect(sensors, &Sensors::cadenceRpmMeasured, _simulation, &Simulation::setCadence);
    connect(sensors, &Sensors::powerWattsMeasured, _simulation, &Simulation::setPower);
    connect(sensors, &Sensors::wheelSpeedMpsMeasured, _simulation, &Simulation::setWheelSpeed);
    sensors->initialize();

    _actuators = new indoorcycling::Actuators(_cyclist, _antCentralDispatch, this);
    connect(_simulation, &Simulation::slopeChanged, _actuators, &Actuators::setSlope);
    _actuators->initialize();

    _lastInformationMessage = _rlv.informationBoxForDistance(_cyclist->distance());
    _informationMessageTimer.setInterval(1000);
    connect(&_informationMessageTimer, &QTimer::timeout, this, [this]() {
        const InformationBox &informationMessage = _rlv.informationBoxForDistance(_cyclist->distance());
        if (!(informationMessage == _lastInformationMessage)) {
            _lastInformationMessage = informationMessage;
            emit newInformationMessage(informationMessage);
        }
    });
    _informationMessageTimer.start();

    _rideFileSampler = new RideSampler(_rlv.name(), _course.name(), *_simulation, this);
}

Run::~Run()
{
    _antCentralDispatch->closeAllChannels();
}

const Simulation &Run::simulation() const
{
    return *_simulation;
}

QTime Run::time() const
{
    return _simulation->runTime();
}

void Run::start()
{
    play();
    _actuators->setSlope(_rlv.slopeForDistance(_cyclist->distance()));
    _state = State::STARTING;
}

void Run::play()
{
    _simulation->play(true);
    _rideFileSampler->start();
}

void Run::stop()
{
    _simulation->play(false);
    _actuators->setSlope(0.0);
    emit stopped();
}

void Run::pause()
{
    _simulation->play(false);
    _rideFileSampler->stop();
}

bool Run::handleStopRun(QWidget *parent)
{
    pause();

    const QString savePath = RideFileWriter().determineFilePath(_rideFileSampler->rideFile());

    QMessageBox stopRunMessageBox(parent);
    stopRunMessageBox.setText(tr("Save ride?"));
    stopRunMessageBox.setIcon(QMessageBox::Question);
    stopRunMessageBox.setInformativeText(tr("Ride file will be written to %1.").arg(savePath));
    stopRunMessageBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    stopRunMessageBox.setDefaultButton(QMessageBox::Save);

    switch(stopRunMessageBox.exec()) {
    case QMessageBox::Save:
        saveRideFile(parent);
        // fallthrough
    case QMessageBox::Discard:
        stop();
        return true;
    default:
        play();
        return false;
    }
}

void Run::distanceChanged(float distance)
{
    if (_state == State::RIDING && distance > _course.end()) {
        setState(State::FINISHED);
    }
}

void Run::speedChanged(float speed)
{
    if ((_state == State::STARTING || _state == State::PAUSED) && speed > 0) {
        setState(State::RIDING);
    } else if (_state == State::RIDING && speed < 0.01) {
        setState(State::PAUSED);
    }
}

QString Run::saveRideFile(QWidget *parent)
{
    RideFileWriter writer;

    QProgressDialog progressDialog("Saving...", QString(), 0, 100, parent);
    progressDialog.setWindowModality(Qt::WindowModal);

    // write the file. We'll use a lambda function to let the progress dialog update it's progress bar if there's
    // a need for it (if the operation takes too long.
    return writer.writeRideFile(_rideFileSampler->rideFile(), [&progressDialog](int percent) {
        progressDialog.setValue(percent);
    });
}

void Run::setState(State newState)
{
    _state = newState;
    switch(_state) {
    case State::RIDING:
        emit riding();
        break;
    case State::PAUSED:
        emit paused();
        break;
    case State::FINISHED:
        emit finished();
        break;
    default:
        // NOOP
        break;
    }
}



