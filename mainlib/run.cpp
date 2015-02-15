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

#include "antcontroller.h"
#include "newvideowidget.h"

#include <QtCore/QTimer>
#include <QtCore/QtDebug>

Run::Run(const ANTController& antController, RealLifeVideo& rlv, Course& course, QObject* parent) :
    QObject(parent), _antController(antController), _rlv(rlv), _course(course)
{
    QSettings settings;
    const int weight = settings.value("cyclist.weight", QVariant::fromValue(82)).toInt();
   _cyclist = new Cyclist(weight, this);

   _simulation = new Simulation(*_cyclist, this);

    qDebug() << "new run";
    _simulation->rlvSelected(rlv);
    _simulation->courseSelected(course);

    connect(&antController, &ANTController::heartRateMeasured, &_simulation->cyclist(), &Cyclist::setHeartRate);
    connect(&antController, &ANTController::cadenceMeasured, &_simulation->cyclist(), &Cyclist::setCadence);
    connect(&antController, &ANTController::powerMeasured, &_simulation->cyclist(), &Cyclist::setPower);
}

Run::~Run()
{
    // empty
}

const Simulation &Run::simulation() const
{
    return *_simulation;
}

bool Run::isRunning() const
{
    return _running;
}

void Run::start()
{
    _running = true;
    _simulation->play(true);
    QSettings settings;
    if (settings.value("useRobot", QVariant::fromValue(false)).toBool()) {
        startRobot(settings);
    }
    QTimer* saveTimer = new QTimer(this);
    saveTimer->setInterval(1000);
    connect(saveTimer, &QTimer::timeout, this, [=]() {
        saveRun();
    });
    saveTimer->start();
}

void Run::stop()
{
    _simulation->play(false);
    emit stopped();
}

void Run::pause()
{
    _simulation->play(false);
}

void Run::startRobot(const QSettings& settings)
{
    const int powerValue = settings.value("robotPower").toInt();

    QTimer* startTimer = new QTimer(this);
    connect(startTimer, &QTimer::timeout, startTimer, [=]() {
        _cyclist->setPower(powerValue);
        _cyclist->setCadence(80);
        _cyclist->setHeartRate(150);
    });
    startTimer->setSingleShot(true);
    startTimer->start(1000);
}

void Run::saveRun()
{
    _rlv.setUnfinishedRun(_cyclist->distance());
    QSettings settings;
    settings.beginGroup("unfinished_runs");
    const QString key = _rlv.name();
    settings.setValue(key, QVariant::fromValue(_cyclist->distance()));
    settings.endGroup();
}
