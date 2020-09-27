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
#ifndef VIDEOLISTVIEW_H
#define VIDEOLISTVIEW_H

#include <QtCore/QSortFilterProxyModel>
#include <QtGui/QPainter>
#include <QtWidgets/QWidget>
#include <QtWidgets/QListView>
#include <QtWidgets/QLineEdit>

class VideoListModel;
class VideoDetails;

namespace indoorcycling {
class AntCentralDispatch;
}
#include "model/reallifevideo.h"

class VideoListView : public QWidget
{
    Q_OBJECT

public:
    explicit VideoListView(QWidget *parent = 0);

signals:
    void videoSelected(RealLifeVideo& rlv, int courseNr);

public slots:
    void setVideos(RealLifeVideoList& rlvs);

private slots:
    void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

private:
    void listViewContentChanged();

    QLineEdit* _filterLineEdit;
    QListView* _listView;
    QSortFilterProxyModel* _filterProxyModel;
    VideoDetails* _detailsWidget;
    VideoListModel* _videoListModel;

};

#endif // VIDEOLISTVIEW_H
