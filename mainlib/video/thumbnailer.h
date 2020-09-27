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

#ifndef THUMBNAILER_H
#define THUMBNAILER_H

#include <QtCore/QDir>
#include <QtCore/QFuture>
#include <QtCore/QFutureWatcher>
#include <QtGui/QPixmap>
#include "model/reallifevideo.h"

class ThumbnailCreatingVideoReader;
/**
 * @brief Class that handles the making of thumbnails for videos by
 * opening the video files and taking the first frame. That frame
 * is converted to a pixmap object.
 */
class Thumbnailer : public QObject
{
    Q_OBJECT
public:
    explicit Thumbnailer(QObject* parent = 0);
    virtual ~Thumbnailer();

    QPixmap thumbnailFor(RealLifeVideo &rlv, const qreal distance = 0.0);

signals:
    void pixmapUpdated(const RealLifeVideo& rlv, const qreal distance, QPixmap pixmap);

private slots:
    void setNewFrame(const RealLifeVideo &rlv, const qreal distance, const QImage& frame);
private:
    static QDir thumbnailDirectory();
    QPixmap createTextPixmap(const QString& text) const;
    QPixmap createEmptyPixmap() const;
    QPixmap createInvalidPixmap() const;
    QString cacheFilePathFor(const RealLifeVideo& rlv, const qreal distance);

    /*!
     * \brief Create the cache directory in which the thumbnails will be stored, if it does not exist yet.
     */
    void createCacheDirectoryIfNotExists();

    /**
     * @brief check if a thumnail exists for a RealLifeVideo for a given distance.
     * @param rlv the video..
     * @param distance distance in the video.
     * @return true if there is a thumbnail.
     */
    bool doesThumbnailExistsFor(const RealLifeVideo& rlv, const qreal distance);

    /**
     * Load the thumbnail for a RealLifeVideo.
     */
    QPixmap loadThumbnailFor(const RealLifeVideo& rlv, const qreal distance);

    /**
     * @brief The directory where thumbnails are cached.
     */
    QDir _cacheDirectory;

    QPixmap _emptyPixmap;

    ThumbnailCreatingVideoReader* const _videoReader;
    QThread* const _videoReaderThread;
};

#endif // THUMBNAILER_H
