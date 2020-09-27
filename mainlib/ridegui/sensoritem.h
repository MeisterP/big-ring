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

#ifndef SENSORITEM_H
#define SENSORITEM_H

#include <QObject>
#include <QtGui/QFont>
#include <QtWidgets/QGraphicsItem>

#include "generalgui/quantityprinter.h"
#include "model/simulation.h"

class SensorItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    explicit
    SensorItem(const QuantityPrinter::Quantity, QObject *parent = 0);

    virtual QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
public slots:
    virtual void setValue(const QVariant &value);
private:
    const QuantityPrinter* _quantityPrinter;
    const QuantityPrinter::Quantity _quantity;
    int _textItemWidth;
    int _unitItemWidth;

    QGraphicsTextItem* _textItem;
    QGraphicsTextItem* _unitItem;
};

#endif // SENSORITEM_H
