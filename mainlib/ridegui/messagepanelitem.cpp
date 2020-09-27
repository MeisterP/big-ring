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
#include "messagepanelitem.h"

#include <QtGui/QFont>
#include <QtGui/QPainter>

MessagePanelItem::MessagePanelItem(QObject *parent) :
    QObject(parent)
{
    QFont font = QFont("Liberation Mono");
    font.setBold(true);
    font.setPointSize(24);

    _textItem = new QGraphicsTextItem(this);
    _textItem->setFont(font);

    _textItem->setDefaultTextColor(Qt::white);

    _textItem->setPlainText("Empty");
    _textItem->setPos(10, 5);
    _textItem->setOpacity(0.65);
}

QRectF MessagePanelItem::boundingRect() const
{
    QRectF internalRect = _textItem->boundingRect();

    return QRectF(0, 0, internalRect.width() + 20, internalRect.height() + 20);
}

void MessagePanelItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QPen pen(Qt::green);
    pen.setWidth(2);
    painter->setPen(pen);
    painter->setBrush(Qt::black);
    painter->setOpacity(0.65 * opacity());
    painter->drawRoundedRect(0, -10, boundingRect().width(), boundingRect().height(), 3, 3);
}

void MessagePanelItem::setMessage(const QString &text)
{
    _textItem->setHtml(text);
    _textItem->show();
}
