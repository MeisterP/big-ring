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

#include "cyclist.h"
#include "profileitem.h"
#include "profilepainter.h"

#include <QtCore/QtDebug>
#include <QtGui/QPainter>

ProfileItem::ProfileItem(QGraphicsItem *parent):
    QGraphicsWidget(parent), _profilePainter(new ProfilePainter(this)), _dirty(false), _cyclist(nullptr)
{
    setOpacity(0.75);
    QFont font("Sans");
    font.setBold(false);
    font.setPointSize(16);
}

void ProfileItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QPen pen = QColor(Qt::green);
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(2);
    painter->setOpacity(0.5);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(pen);
    painter->setBrush(Qt::lightGray);
    painter->drawRoundedRect(boundingRect(), 5, 5);

    if (_rlv.isValid()) {
        if (_dirty) {
            _profilePixmap = _profilePainter->paintProfile(_rlv, _internalRect, true);
            _dirty = false;
        }
        if (!_profilePixmap.isNull()) {
            painter->drawPixmap(_internalRect, _profilePixmap);
            if (_course.isValid()) {
                if (_course.isValid()) {
                    paintArea(painter, 0, _course.start(), Qt::black);
                    paintArea(painter, _course.end(), _rlv.totalDistance(), Qt::black);
                }
            }
            if (_cyclist) {
                paintArea(painter, _course.start(), std::min(_cyclist->distance(), _rlv.totalDistance()), Qt::green);
            }
        }
    }

}

void ProfileItem::setGeometry(const QRectF &rect)
{
    _dirty = true;
    prepareGeometryChange();
    QGraphicsWidget::setGeometry(rect);
    _internalRect = QRect(1, 1, rect.width() - 2, rect.height() - 2);
    _dirty = true;
}

void ProfileItem::setRlv(const RealLifeVideo &rlv)
{
    _rlv = rlv;
    _dirty = true;
    update();
}

void ProfileItem::setCourse(const Course &course)
{
    _course = course;
    update();
}

void ProfileItem::setCyclist(const Cyclist *cylist)
{
    _cyclist = cylist;
}

void ProfileItem::paintArea(QPainter *painter, const float startDistance, const float endDistance, const QColor &color) const
{
    QBrush brush(color);
    QPen pen = color;
    painter->setOpacity(0.3);
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(2);
    painter->setPen(pen);
    painter->setBrush(brush);
    int left = (startDistance / _rlv.totalDistance()) * _internalRect.width();
    int width = ((endDistance - startDistance) / _rlv.totalDistance()) * _internalRect.width();
    painter->drawRect(left, _internalRect.top(), width, _internalRect.bottom());
}
