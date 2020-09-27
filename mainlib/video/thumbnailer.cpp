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

#include "thumbnailer.h"

#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/QtDebug>
#include <QtCore/QThread>
#include <QtGui/QFont>
#include <QtGui/QPainter>

#include "thumbnailcreatingvideoreader.h"

namespace
{
/**
 * @brief Size for default empty images.
 */
const QSize DEFAULT_IMAGE_SIZE(1920, 1080);
}

Thumbnailer::Thumbnailer(QObject *parent): QObject(parent), _videoReader(new ThumbnailCreatingVideoReader), _videoReaderThread(new QThread)
{
    _cacheDirectory = thumbnailDirectory();
    createCacheDirectoryIfNotExists();

    _emptyPixmap = createEmptyPixmap();

    // the video reader is running on a seperate thread, so it will not block the UI when decoding video frames.
    _videoReader->moveToThread(_videoReaderThread);

    connect(_videoReader, &ThumbnailCreatingVideoReader::newFrameReady, this, &Thumbnailer::setNewFrame);

    // Make sure that when the videoReaderThread is stopped and it is deleted
    connect(_videoReaderThread, &QThread::finished, _videoReaderThread, &QThread::deleteLater);
    connect(_videoReaderThread, &QThread::finished, _videoReader, &ThumbnailCreatingVideoReader::deleteLater);
    _videoReaderThread->start();
}

Thumbnailer::~Thumbnailer()
{
    _videoReaderThread->quit();
}

/**
 * @brief get the thumbnail for an rlv at a certain distance. If no thumbnail is ready yet, an empty thumbnail
 * will be returned. A generated thumbnail will later be emitted using a pixmapUpdated() signal.
 * @param rlv the rlv
 * @param distance the distance
 * @return a thumbnail, or an empty pixmap if no thumbnail is currently ready.
 */
QPixmap Thumbnailer::thumbnailFor(RealLifeVideo &rlv, const qreal distance)
{
    if (rlv.isValid()) {
        if (doesThumbnailExistsFor(rlv, distance)) {
            return loadThumbnailFor(rlv, distance);
        }

        _videoReader->createImageForFrame(rlv, distance);
    }
    return _emptyPixmap;
}

void Thumbnailer::setNewFrame(const RealLifeVideo& rlv, const qreal distance, const QImage &frame)
{
    QPixmap asPixmap;
    if (frame.isNull()) {
        asPixmap = createInvalidPixmap();
        asPixmap.save(cacheFilePathFor(rlv, distance));
    } else {
        frame.save(cacheFilePathFor(rlv, distance));
        asPixmap = QPixmap::fromImage(frame);
    }
    emit pixmapUpdated(rlv, distance, asPixmap);
}

QPixmap Thumbnailer::createTextPixmap(const QString &text) const
{
    QPixmap emptyPixmap(DEFAULT_IMAGE_SIZE);
    emptyPixmap.fill(Qt::black);

    QFont font;
    font.setPointSize(72);
    QPainter p(&emptyPixmap);

    p.setFont(font);
    QTextOption textOption;
    textOption.setAlignment(Qt::AlignCenter);
    p.setPen(Qt::white);
    p.drawText(emptyPixmap.rect(), QString(text), textOption);

    return emptyPixmap;
}

QPixmap Thumbnailer::createEmptyPixmap() const
{
    return createTextPixmap(tr("Loading screenshot"));
}

QPixmap Thumbnailer::createInvalidPixmap() const
{
    return createTextPixmap(tr("Unable to create screenshot"));
}

void Thumbnailer::createCacheDirectoryIfNotExists()
{
    QDir cacheDir(_cacheDirectory);
    if (!cacheDir.exists()) {
        cacheDir.mkpath(".");
    }
}

QString Thumbnailer::cacheFilePathFor(const RealLifeVideo &rlv, const qreal distance)
{
    QString filename = QString("%1_%2.jpg").arg(rlv.name()).arg(distance);
    return _cacheDirectory.absoluteFilePath(filename);
}

QDir Thumbnailer::thumbnailDirectory()
{
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::CacheLocation);
    if (paths.isEmpty()) {
        return QDir("/tmp/thumbnails");
    } else {
        return QDir(QString("%1/thumbnails").arg(paths[0]));
    }
}

bool Thumbnailer::doesThumbnailExistsFor(const RealLifeVideo &rlv, const qreal distance)
{
    QFile cacheFile(cacheFilePathFor(rlv, distance));
    return cacheFile.exists() && cacheFile.size() > 0;
}

QPixmap Thumbnailer::loadThumbnailFor(const RealLifeVideo &rlv, const qreal distance)
{
    QString path = cacheFilePathFor(rlv, distance);
    return QPixmap(path);
}

