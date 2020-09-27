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

#ifndef NEWVIDEOWIDGET_H
#define NEWVIDEOWIDGET_H

#include <QtCore/QScopedPointer>
#include <QtCore/QTimer>
#include <QtOpenGL/QGLWidget>
#include <QtWidgets/QGraphicsView>

#include "model/reallifevideo.h"
#include "profileitem.h"

class Simulation;
class VideoPlayer;

namespace indoorcycling {
class ScreenSaverBlocker;
}

class InformationBoxGraphicsItem;
class MessagePanelItem;
class RollingAverageSensorItem;
class SensorItem;
class ClockGraphicsItem;

class NewVideoWidget : public QGraphicsView
{
    Q_OBJECT
public:
    /**
     * Create a new NewVideoWidget.
     * @param showDebugOutput if true, debug output is shown during the ride.
     * @param parent parent widget.
     */
    explicit NewVideoWidget(bool showDebugOutput, QWidget *parent = 0);
    ~NewVideoWidget();

    bool isReadyToPlay();

    bool handleKeyPress(QKeyEvent*);
signals:
    void readyToPlay(bool ready);

public slots:
    void setRealLifeVideo(RealLifeVideo rlv);
    void setCourse(Course& course);
    void setCourseIndex(int index);
    void setDistance(float distance);
    void displayMessage(const QString &message);
    void displayInformationBox(const InformationBox &informationBox);
    void setSimulation(const Simulation &cyclist);

    void goToFullscreen();
protected:
    void resizeEvent(QResizeEvent *) override;
    virtual void enterEvent(QEvent *) override;
    virtual void leaveEvent(QEvent *) override;
    virtual void mouseMoveEvent(QMouseEvent *) override;
    virtual void closeEvent(QCloseEvent*) override;
    virtual void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    void setupVideoPlayer(QGLWidget* paintWidget);

    void seekToStart(Course& course);
    void step(int stepSize);

    void addSensorItems(QGraphicsScene* scene);

    void addClock(QGraphicsScene* scene);
    void addMessagePanel(QGraphicsScene *scene);

    RealLifeVideo _rlv;
    Course _course;
    VideoPlayer* _videoPlayer;

    ClockGraphicsItem* _clockItem;
    MessagePanelItem *_messagePanelItem;
    InformationBoxGraphicsItem *_informationBoxItem;
    QTimer *_informationBoxHideTimer;
    RollingAverageSensorItem* _powerItem;
    SensorItem* _heartRateItem;
    RollingAverageSensorItem* _cadenceItem;
    RollingAverageSensorItem* _speedItem;
    SensorItem* _distanceItem;
    SensorItem* _gradeItem;
    ProfileItem* _profileItem;
    SensorItem *_frameRateItem;
    indoorcycling::ScreenSaverBlocker* _screenSaverBlocker;
    QTimer* _mouseIdleTimer;
    Qt::AspectRatioMode _aspectRatioMode;
};

#endif // NEWVIDEOWIDGET_H
