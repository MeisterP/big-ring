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
#include "generalgui/profilepainter.h"
#include "generalgui/quantityprinter.h"
#include "videoitemdelegate.h"
#include "videolistmodel.h"
#include <QtWidgets/QApplication>
#include <QtCore/QtDebug>
#include <QtGui/QPainter>
#include <QtGui/QPixmapCache>


VideoItemDelegate::VideoItemDelegate(QObject *parent):
    QAbstractItemDelegate(parent), _profilePainter(new ProfilePainter(this)),
    _quantityPrinter(new QuantityPrinter(this))
{
    // empty
}

void VideoItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QBrush rectBrush = (option.state & QStyle::State_Selected) ? option.palette.highlight() :option.palette.window();
    QPen pen(((option.state & QStyle::State_Selected) ? option.palette.highlightedText() : option.palette.text()).color());
    painter->setBrush(rectBrush);
    painter->drawRoundedRect(option.rect, 3, 3);
    QVariant rlvData = index.data(VideoDataRole);
    RealLifeVideo video = rlvData.value<RealLifeVideo>();

    QSize rectSize = option.rect.size();
    QRect profileRect(option.rect.topLeft(), QSize(rectSize).scaled(.9 * rectSize.width(), .9 * rectSize.height(),
                                                                    Qt::KeepAspectRatio));
    profileRect.moveCenter(option.rect.center());
    paintProfile(painter, profileRect, video);

    painter->setPen(pen);

    QString text = QString("%1\n%2 %3\n%4").arg(video.name()).arg(_quantityPrinter->printDistance(video.totalDistance()))
            .arg(_quantityPrinter->unitForDistance(QuantityPrinter::Precision::NonPrecise, QVariant::fromValue(video.totalDistance())))
            .arg(realLifeVideoFileTypeName(video.fileType()));
    QRect optionRect = option.rect;
    QRect textRect = optionRect.adjusted(5, 5, 0, 0);
    painter->drawText(textRect, text);
}

QSize VideoItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(200, 100);
}

void VideoItemDelegate::paintProfile(QPainter *painter, QRect &rect, RealLifeVideo &rlv) const
{
    QPixmap profilePixmap = _profilePainter->paintProfile(rlv, rect, false);
    if (!profilePixmap.isNull()) {
        painter->drawPixmap(rect, profilePixmap);
    }
}
