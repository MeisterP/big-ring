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

#include "newvideowidget.h"

#include <functional>
#include <cmath>

#include <QtCore/QtDebug>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QUrl>
#include <QtOpenGL/QGLWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtGui/QResizeEvent>


#include "config/bigringsettings.h"
#include "clockgraphicsitem.h"
#include "informationboxgraphicsitem.h"
#include "messagepanelitem.h"
#include "profileitem.h"
#include "rollingaveragesensoritem.h"
#include "sensoritem.h"
#include "model/simulation.h"
#include "util/screensaverblocker.h"
#include "video/videoplayer.h"


NewVideoWidget::NewVideoWidget(bool showDebugOutput, QWidget *parent) :
    QGraphicsView(parent),
    _screenSaverBlocker(new indoorcycling::ScreenSaverBlocker(this)),
    _mouseIdleTimer(new QTimer(this))
{
    setMinimumSize(800, 600);
    setFocusPolicy(Qt::StrongFocus);
    QGLWidget* viewPortWidget = new QGLWidget(QGLFormat(QGL::SampleBuffers));
    setViewport(viewPortWidget);
    setFrameShape(QFrame::NoFrame);

    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    QGraphicsScene* scene = new QGraphicsScene(this);
    setScene(scene);

    setSizeAdjustPolicy(QGraphicsView::AdjustIgnored);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);

    addClock(scene);
    addMessagePanel(scene);
    addSensorItems(scene);

    _profileItem = new ProfileItem;
    scene->addItem(_profileItem);

    _frameRateItem = new SensorItem(QuantityPrinter::Quantity::FramesPerSecond);
    _frameRateItem->setValue(QVariant::fromValue(0));
    _frameRateItem->setVisible(showDebugOutput);
    scene->addItem(_frameRateItem);

    setupVideoPlayer(viewPortWidget);

    _mouseIdleTimer->setInterval(500);
    _mouseIdleTimer->setSingleShot(true);
    connect(_mouseIdleTimer, &QTimer::timeout, _mouseIdleTimer, []() {
        QApplication::setOverrideCursor(Qt::BlankCursor);
    });
    _aspectRatioMode = BigRingSettings().videoAspectRatio();
}

void NewVideoWidget::setupVideoPlayer(QGLWidget* paintWidget)
{
    _videoPlayer = new VideoPlayer(paintWidget, this);

    connect(_videoPlayer, &VideoPlayer::videoLoaded, this, [this](qint64 numberOfFrames) {
        qDebug() << "total number of frames" << numberOfFrames;
        this->_rlv.setNumberOfFrames(numberOfFrames);
        if (this->_course.isValid()) {
            this->seekToStart(_course);
        }
    });
    connect(_videoPlayer, &VideoPlayer::seekDone, this, [this]() {
        emit readyToPlay(true);
    });
    connect(_videoPlayer, &VideoPlayer::updateVideo, this, [this]() {
        this->viewport()->update(rect());
    });
    connect(_videoPlayer, &VideoPlayer::frameRateChanged, this, [this](const int frameRate) {
        this->_frameRateItem->setValue(frameRate);
    });
}

void NewVideoWidget::addClock(QGraphicsScene* scene)
{
    _clockItem = new ClockGraphicsItem;
    scene->addItem(_clockItem);
}

void NewVideoWidget::addMessagePanel(QGraphicsScene *scene)
{
    _messagePanelItem = new MessagePanelItem;
    _messagePanelItem->hide();
    scene->addItem(_messagePanelItem);
}

NewVideoWidget::~NewVideoWidget()
{
    // empty
}

bool NewVideoWidget::isReadyToPlay()
{
    return _videoPlayer->isReadyToPlay();
}

bool NewVideoWidget::handleKeyPress(QKeyEvent *event)
{
    switch(event->key()) {
    case Qt::Key_D:
        _frameRateItem->setVisible(!_frameRateItem->isVisible());
        return true;
    }
    return false;
}

void NewVideoWidget::setRealLifeVideo(RealLifeVideo rlv)
{
    Q_EMIT(readyToPlay(false));
    _rlv = rlv;
    _profileItem->setRlv(rlv);
    _videoPlayer->stop();
    _videoPlayer->loadVideo(rlv.videoFilename());
}

void NewVideoWidget::setCourse(Course &course)
{
    _course = course;
    _profileItem->setCourse(course);
    seekToStart(_course);
}

void NewVideoWidget::setCourseIndex(int index)
{
    if (!_rlv.isValid()) {
        return;
    }
    Course course = _rlv.courses()[qMax(0, index)];
    setCourse(course);
}

void NewVideoWidget::setDistance(float distance)
{
    _videoPlayer->stepToFrame(_rlv.frameForDistance(distance));
}

void NewVideoWidget::displayMessage(const QString &message)
{
    if (message.isEmpty()) {
        QPropertyAnimation *animation = new QPropertyAnimation(_messagePanelItem, "opacity", this);
        animation->setDuration(1000);
        animation->setStartValue(1.0);
        animation->setEndValue(0.0);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        _messagePanelItem->setMessage(message);
        QRectF rect = _messagePanelItem->boundingRect();
        rect.moveCenter(QPointF(sceneRect().width() / 2, sceneRect().height() / 2));
        _messagePanelItem->setPos(rect.topLeft());
        QPropertyAnimation *animation = new QPropertyAnimation(_messagePanelItem, "opacity", this);
        animation->setDuration(1000);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
        _messagePanelItem->show();
    }
}

void NewVideoWidget::displayInformationBox(const InformationBox &informationBox)
{
    InformationBoxGraphicsItem *informationBoxItem = new InformationBoxGraphicsItem(this);
    scene()->addItem(informationBoxItem);
    informationBoxItem->setInformationBox(informationBox);
    const qreal height = informationBoxItem->boundingRect().height();
    const qreal width = informationBoxItem->boundingRect().width();

    informationBoxItem->setX(-width);
    informationBoxItem->setY(sceneRect().height() * 27 / 32 - height);

    QPropertyAnimation* animation = new QPropertyAnimation(informationBoxItem, "x", this);
    animation->setDuration(5000);
    qreal centered = sceneRect().center().x() - (width / 2);
    animation->setKeyValueAt(0.10, centered);
    animation->setKeyValueAt(0.9, centered);
    animation->setKeyValueAt(1.0, sceneRect().width());

    connect(animation, &QPropertyAnimation::finished, this, [this, informationBoxItem]() {
        scene()->removeItem(informationBoxItem);
        informationBoxItem->deleteLater();
    });
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    informationBoxItem->show();
}

void NewVideoWidget::setSimulation(const Simulation& simulation)
{
    connect(&simulation, &Simulation::runTimeChanged, _clockItem, &ClockGraphicsItem::setTime);

    const Cyclist& cyclist = simulation.cyclist();
    _profileItem->setCyclist(&cyclist);
    connect(&cyclist, &Cyclist::powerChanged, this, [this](int power) {
        _powerItem->setValue(QVariant::fromValue(power));
    });
    connect(&cyclist, &Cyclist::cadenceChanged, this, [this](int cadence) {
        _cadenceItem->setValue(QVariant::fromValue(cadence));
    });
    connect(&cyclist, &Cyclist::speedChanged, this, [this](float speed) {
        _speedItem->setValue(QVariant::fromValue(speed));
    });
    connect(&simulation, &Simulation::slopeChanged, this, [this](float grade) {
        _gradeItem->setValue(QVariant::fromValue(grade));
    });
    connect(&cyclist, &Cyclist::heartRateChanged, this, [this](int heartRate) {
        _heartRateItem->setValue(QVariant::fromValue(heartRate));
    });
    connect(&cyclist, &Cyclist::distanceChanged, this, [this](float distance) {
        float distanceDifference = _course.end() - distance;
        _distanceItem->setValue(QVariant::fromValue(std::fabs(distanceDifference)));
    });

}

void NewVideoWidget::goToFullscreen()
{
    showFullScreen();
}

void NewVideoWidget::resizeEvent(QResizeEvent *resizeEvent)
{
    setSceneRect(viewport()->rect());
    QRectF clockItemRect = _clockItem->boundingRect();
    clockItemRect.moveCenter(QPointF(sceneRect().width() / 2, 0.0));
    clockItemRect.moveTop(0.0);
    _clockItem->setPos(clockItemRect.topLeft());

    qreal bottom = mapToScene(0, height()).y();
    _cadenceItem->setPos(0, bottom - _cadenceItem->boundingRect().height());
    _heartRateItem->setPos(0, _cadenceItem->scenePos().y() - _heartRateItem->boundingRect().height());
    _powerItem->setPos(0, _heartRateItem->scenePos().y() - _powerItem->boundingRect().height());

    QPointF leftOfRightItems = mapToScene(width(), height());
    leftOfRightItems = QPointF(leftOfRightItems.x() - _speedItem->boundingRect().width(), leftOfRightItems.y());
    qreal left = leftOfRightItems.x();
    _gradeItem->setPos(left, bottom - _gradeItem->boundingRect().height());
    _distanceItem->setPos(left, _gradeItem->scenePos().y() - _distanceItem->boundingRect().height());
    _speedItem->setPos(left, _distanceItem->scenePos().y() - _speedItem->boundingRect().height());

    qreal profileItemLeft = _cadenceItem->boundingRect().width();
    qreal profileItemWidth = sceneRect().width() - 2 * profileItemLeft;
    qreal profileItemTop = _powerItem->scenePos().y();
    _profileItem->setGeometry(QRectF(profileItemLeft, profileItemTop, profileItemWidth, bottom - profileItemTop));

    _frameRateItem->setPos(mapToScene(0, 0));

    resizeEvent->accept();
}

void NewVideoWidget::enterEvent(QEvent *)
{
    QApplication::setOverrideCursor(Qt::BlankCursor);
    _mouseIdleTimer->start();
}

void NewVideoWidget::leaveEvent(QEvent *)
{
    QApplication::setOverrideCursor(Qt::ArrowCursor);
    _mouseIdleTimer->stop();
}

void NewVideoWidget::mouseMoveEvent(QMouseEvent *)
{
    QApplication::setOverrideCursor(Qt::ArrowCursor);
    _mouseIdleTimer->stop();
    _mouseIdleTimer->start();
}

void NewVideoWidget::closeEvent(QCloseEvent *)
{
    QApplication::setOverrideCursor(Qt::ArrowCursor);
}

/*!
 *
 * \brief Draw background by telling the video sink to update the complete background.
 * \param painter the painter used for drawing.
 */
void NewVideoWidget::drawBackground(QPainter *painter, const QRectF &)
{
    QPointF topLeft = mapToScene(viewport()->rect().topLeft());
    QPointF bottemRight = mapToScene(viewport()->rect().bottomRight());
    const QRectF r = QRectF(topLeft, bottemRight);
    _videoPlayer->displayCurrentFrame(painter, r, _aspectRatioMode);

}

void NewVideoWidget::seekToStart(Course &course)
{
    quint32 frame = _rlv.frameForDistance(course.start());
    _videoPlayer->seekToFrame(frame);
}

void NewVideoWidget::addSensorItems(QGraphicsScene *scene)
{
    const int powerAveragingMilliseconds = BigRingSettings().powerAveragingForDisplayMilliseconds();
    const int powerUpdateInterval = (powerAveragingMilliseconds == 0) ? 0 : 1000;
    _powerItem = new RollingAverageSensorItem(QuantityPrinter::Quantity::Power, powerAveragingMilliseconds, powerUpdateInterval);
    scene->addItem(_powerItem);
    _heartRateItem = new SensorItem(QuantityPrinter::Quantity::HeartRate);
    scene->addItem(_heartRateItem);
    _cadenceItem = new RollingAverageSensorItem(QuantityPrinter::Quantity::Cadence, 1000, 1000);
    scene->addItem(_cadenceItem);
    _speedItem = new RollingAverageSensorItem(QuantityPrinter::Quantity::Speed, 1000, 1000);
    scene->addItem(_speedItem);
    _distanceItem = new SensorItem(QuantityPrinter::Quantity::Distance);
    scene->addItem(_distanceItem);
    _gradeItem = new SensorItem(QuantityPrinter::Quantity::Grade);
    scene->addItem(_gradeItem);
}

