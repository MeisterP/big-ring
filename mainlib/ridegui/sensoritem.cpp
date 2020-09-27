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

#include "sensoritem.h"

#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>

SensorItem::SensorItem(const QuantityPrinter::Quantity quantity, QObject *parent) :
    QObject(parent), _quantityPrinter(new QuantityPrinter(this)), _quantity(quantity)
{
    QFont font = QFont("Liberation Mono");
    font.setBold(true);
    font.setPointSize(24);

    _textItem = new QGraphicsTextItem(this);
    _textItem->setFont(font);
    _textItem->setDefaultTextColor(Qt::white);
    _textItem->setOpacity(0.6);

    _textItem->hide();
    _textItem->setPlainText(QString("00000"));
    _textItemWidth = _textItem->boundingRect().width();
    _textItem->setPos(10, 5);

    QFont unitFont = QFont("Liberation Mono");
    unitFont.setBold(true);
    unitFont.setPointSize(16);
    _unitItem = new QGraphicsTextItem(this);
    _unitItem->setFont(unitFont);
    _unitItem->setDefaultTextColor(Qt::white);
    _unitItem->setOpacity(0.6);
    _unitItem->setPlainText("WWW");
    _unitItemWidth = _unitItem->boundingRect().width();
    _unitItem->setPos(_textItem->boundingRect().width() + 5, _textItem->boundingRect().bottom() - _unitItem->boundingRect().height() - 3);
    _unitItem->setPlainText(QString("%1").arg(_quantityPrinter->unitString(quantity, QuantityPrinter::Precision::Precise), 3));
}

QRectF SensorItem::boundingRect() const
{
    return QRectF(0, 0, _textItemWidth + 5 + _unitItemWidth + 20, _textItem->boundingRect().height());
}

void SensorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QPen pen(Qt::green);
    pen.setWidth(2);
    painter->setPen(pen);
    painter->setBrush(Qt::black);
    painter->setOpacity(0.65);
    painter->drawRoundedRect(boundingRect(), 5, 5);
}


void SensorItem::setValue(const QVariant &value)
{
    _unitItem->setPlainText(_quantityPrinter->unitString(_quantity, QuantityPrinter::Precision::Precise, value));
    _textItem->setPlainText(_quantityPrinter->print(value, _quantity, QuantityPrinter::Precision::Precise));
    _textItem->show();
}


