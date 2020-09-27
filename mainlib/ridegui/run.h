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

#ifndef RUN_H
#define RUN_H

#include <QtCore/QObject>
#include <QtCore/QSettings>

#include "model/reallifevideo.h"
#include "model/cyclist.h"
#include "model/simulation.h"

namespace indoorcycling {
class Actuators;
class AntCentralDispatch;
}

class NewVideoWidget;
class RideSampler;

class Run : public QObject
{
    Q_OBJECT
public:
    explicit Run(indoorcycling::AntCentralDispatch *antCentralDispatch, RealLifeVideo& rlv, Course& course, QObject *parent = 0);
    virtual ~Run();

    const Simulation& simulation() const;

    /** Time it took from start to current time */
    QTime time() const;

    const Cyclist &cyclist() const;
signals:
    void startedRiding();
    void stopped();
    void riding();
    void paused();
    void finished();
    void newInformationMessage(const InformationBox &message);
public slots:
    void start();
    void play();
    void stop();
    void pause();

    /**
     * Handler for stopping run
     */
    bool handleStopRun(QWidget* parent);
private slots:
    void distanceChanged(float distance);
    void speedChanged(float speed);
private:
    QString saveRideFile(QWidget *parent);
    enum State {
        BEFORE_START, STARTING, RIDING, PAUSED, FINISHED
    };
    void setState(State newState);

    indoorcycling::AntCentralDispatch* const _antCentralDispatch;
    RealLifeVideo _rlv;
    Course _course;
    Cyclist* _cyclist;
    Simulation* _simulation;
    State _state;
    indoorcycling::Actuators *_actuators;
    QTimer _informationMessageTimer;
    InformationBox _lastInformationMessage;
    RideSampler *_rideFileSampler;
};

#endif // RUN_H
