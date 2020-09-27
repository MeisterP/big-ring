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
#include "videoscreenshotwidget.h"
#include "ui_videoscreenshotwidget.h"

#include <QtGui/QPainter>

#include "video/thumbnailer.h"
VideoScreenshotWidget::VideoScreenshotWidget(QWidget *parent) :
    QWidget(parent), _thumbnailer(new Thumbnailer(this)), _distance(0),
    ui(new Ui::VideoScreenshotWidget)
{
    ui->setupUi(this);

    connect(_thumbnailer, &Thumbnailer::pixmapUpdated, this, [=](const RealLifeVideo& rlv, const qreal distance, QPixmap) {
        if (rlv == _rlv && qFuzzyCompare(distance, _distance)) {
            update();
        }
    });
}

VideoScreenshotWidget::~VideoScreenshotWidget()
{
    delete ui;
}


void VideoScreenshotWidget::setVideo(RealLifeVideo& rlv)
{
    _rlv = rlv;
    _distance = 0;
    update();
}

void VideoScreenshotWidget::setDistance(qreal distance)
{
    _distance = distance;
    update();
}

void VideoScreenshotWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), palette().background());

    QPixmap videoScreenshot = _thumbnailer->thumbnailFor(_rlv, _distance);
    if (!videoScreenshot.isNull()) {
        QPixmap scaled = videoScreenshot.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QRect scaledRect = scaled.rect();
        scaledRect.moveCenter(rect().center());

        painter.drawPixmap(scaledRect, scaled);
    }
}

