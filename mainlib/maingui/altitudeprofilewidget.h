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
#ifndef ALTITUDEPROFILEWIDGET_H
#define ALTITUDEPROFILEWIDGET_H

#include <QWidget>

#include "model/reallifevideo.h"

class ProfilePainter;

namespace Ui {
class AltitudeProfileWidget;
}

class AltitudeProfileWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AltitudeProfileWidget(QWidget *parent = 0);
    ~AltitudeProfileWidget();
public slots:
    void setVideo(RealLifeVideo& rlv);
    void setCourseIndex(const int courseIndex);
    void setStartAndEndDistance(qreal startDistance, qreal endDistance);
protected:
    virtual void paintEvent(QPaintEvent *) override;
private:
    ProfilePainter* _profilePainter;
    RealLifeVideo _currentRlv;
    qreal _startDistance;
    qreal _endDistance;
    Ui::AltitudeProfileWidget *ui;
};

#endif // ALTITUDEPROFILEWIDGET_H
