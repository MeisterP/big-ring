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
#include "videolistview.h"

#include <QtCore/QtDebug>
#include <QtCore/QItemSelection>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>

#include "ant/antcentraldispatch.h"
#include "videolistmodel.h"
#include "videoitemdelegate.h"
#include "videodetails.h"
#include "videoqlistview.h"

VideoListView::VideoListView(QWidget *parent) :
    QWidget(parent), _filterLineEdit(new QLineEdit(this)), _listView(new VideoQListView(this)), _filterProxyModel(new QSortFilterProxyModel(this)), _detailsWidget(new VideoDetails(this)), _videoListModel(new VideoListModel)
{
    QHBoxLayout* layout = new QHBoxLayout;
    _filterProxyModel->setSourceModel(_videoListModel);
    _listView->setModel(_filterProxyModel);
    _listView->setItemDelegate(new VideoItemDelegate(this));
    _listView->setMinimumWidth(300);
    _listView->setSpacing(5);
    connect(_listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &VideoListView::selectionChanged);
    connect(_listView->model(), &QAbstractItemModel::rowsInserted, _listView, [=](const QModelIndex&, int, int){
         listViewContentChanged();
    });
    connect(_listView->model(), &QAbstractItemModel::rowsRemoved, _listView, [=](const QModelIndex&, int, int){
         listViewContentChanged();
    });

    QFormLayout* filterFormLayout = new QFormLayout;
    QLabel* filterLabel = new QLabel("&Filter", this);
    filterLabel->setBuddy(_filterLineEdit);
    filterFormLayout->addRow(filterLabel, _filterLineEdit);

    QVBoxLayout* leftLayout = new QVBoxLayout;
    leftLayout->addLayout(filterFormLayout);
    leftLayout->addWidget(_listView);

    layout->addLayout(leftLayout, 1);
    layout->addWidget(_detailsWidget, 3);

    connect(_detailsWidget, &VideoDetails::playClicked, this, &VideoListView::videoSelected);
    connect(_filterLineEdit, &QLineEdit::textChanged, _filterLineEdit, [=](const QString& text) {
        _filterProxyModel->setFilterRegExp(QRegExp(text, Qt::CaseInsensitive, QRegExp::FixedString));
    });

    setLayout(layout);
}

void VideoListView::setVideos(RealLifeVideoList &rlvs)
{
    _videoListModel->setVideos(rlvs);
    _filterProxyModel->setSourceModel(_videoListModel);
}

void VideoListView::selectionChanged(const QItemSelection &selected, const QItemSelection &)
{
    RealLifeVideo rlv;
    if (!selected.isEmpty()) {
        rlv = selected.indexes()[0].data(VideoDataRole).value<RealLifeVideo>();
    }
    _detailsWidget->setVideo(rlv);
}

void VideoListView::listViewContentChanged()
{
    QModelIndex selectedIndex;
    if (_listView->selectionModel()->selectedIndexes().isEmpty()) {
        selectedIndex = _videoListModel->index(0);
    } else {
        selectedIndex = _listView->selectionModel()->selectedIndexes()[0];
    }
    if (selectedIndex.isValid()) {
        _listView->selectionModel()->setCurrentIndex(selectedIndex, QItemSelectionModel::Select);
        _listView->scrollTo(selectedIndex);
    }
}
