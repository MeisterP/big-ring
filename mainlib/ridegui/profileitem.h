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

#ifndef PROFILEITEM_H
#define PROFILEITEM_H

#include <QtCore/QObject>
#include <QtWidgets/QGraphicsWidget>

#include "model/reallifevideo.h"

class Cyclist;
class ProfilePainter;

class ProfileItem : public QGraphicsWidget
{
    Q_OBJECT
public:
    explicit ProfileItem(QGraphicsItem *parent = 0);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    virtual void setGeometry(const QRectF &rect) override;
signals:

public slots:
    void setRlv(const RealLifeVideo& rlv);
    void setCourse(const Course &course);
    void setCyclist(const Cyclist* cylist);
private:
    void paintArea(QPainter *painter, const float startDistance, const float endDistance, const float totalDistance, const QColor &color) const;
    ProfilePainter* _profilePainter;
    QRect _internalRect;
    bool _dirty;
    RealLifeVideo _rlv;
    Course _course;
    const Cyclist* _cyclist;
    QPixmap _profilePixmap;
};

#endif // PROFILEITEM_H
