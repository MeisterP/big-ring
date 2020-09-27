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
#include "altitudeprofilewidget.h"
#include "ui_altitudeprofilewidget.h"

#include <QtGui/QPainter>

#include "generalgui/profilepainter.h"

AltitudeProfileWidget::AltitudeProfileWidget(QWidget *parent) :
    QWidget(parent), _profilePainter(new ProfilePainter(this)),
    _startDistance(-1), _endDistance(-1),
    ui(new Ui::AltitudeProfileWidget)
{
    ui->setupUi(this);
}

AltitudeProfileWidget::~AltitudeProfileWidget()
{
    delete ui;
}


void AltitudeProfileWidget::setVideo(RealLifeVideo &rlv)
{
    _currentRlv = rlv;
    _startDistance = -1;
    _endDistance = -1;
    update();
}

void AltitudeProfileWidget::setCourseIndex(const int courseIndex)
{
    if (courseIndex >= 0 && _currentRlv.isValid()) {
        const Course& course = _currentRlv.courses()[courseIndex];
        _startDistance = course.start();
        _endDistance = course.end();
    } else {
        _startDistance = -1;
    }
    update();
}

void AltitudeProfileWidget::setStartAndEndDistance(qreal startDistance, qreal endDistance)
{
    _startDistance = startDistance;
    _endDistance = endDistance;
    update();
}

void AltitudeProfileWidget::paintEvent(QPaintEvent *paintEvent)
{
    QPainter painter(this);

    QPixmap profilePixmap;
    if (_startDistance > 0 || _endDistance < _currentRlv.totalDistance()) {
        profilePixmap = _profilePainter->paintProfileWithHighLight(_currentRlv, _startDistance, _endDistance,
                                                                   this->rect(), palette().highlight());
    } else {
        profilePixmap = _profilePainter->paintProfile(_currentRlv, this->rect(), true);
    }
    painter.drawPixmap(rect(), profilePixmap);
    QWidget::paintEvent(paintEvent);
}

