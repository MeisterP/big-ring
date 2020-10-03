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
#include "videolistmodel.h"

#include <QtCore/QtDebug>

VideoListModel::VideoListModel(QObject *parent) :
    QAbstractListModel(parent)
{
}

void VideoListModel::setVideos(RealLifeVideoList &rlvs)
{
    qDebug() << "settings videos in model";
    emit layoutAboutToBeChanged();
    beginInsertRows(QModelIndex(), 0, 0);
    _rlvs = rlvs;
    endInsertRows();
    emit layoutChanged();
}

int VideoListModel::rowCount(const QModelIndex &) const
{
    return _rlvs.size();
}

QVariant VideoListModel::data(const QModelIndex &index, int role) const
{
    if (index.row() <= _rlvs.size()) {
        const RealLifeVideo &rlv = _rlvs[index.row()];
        switch(role) {
        case Qt::DisplayRole:
            return QVariant::fromValue(rlv.name());
        case VideoDataRole:
            return QVariant::fromValue(rlv);
        }
    }
    return QVariant();
}
