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

#ifndef REALLIVEVIDEOPARSER_H
#define REALLIVEVIDEOPARSER_H

#include <QFile>
#include <QFutureWatcher>
#include <QObject>
#include <QSharedPointer>

#include "model/reallifevideo.h"

/**
 * @brief Importer for Tacx RLV files.
 *
 * This importer will asynchronously import files and emit the signal importReady when the import is finished.
 * Call ::parseRealLiveVideoFilesFromDir with a root directory. When ready, ::importReady will be emitted.
 *
 * This importer will search for files with the extension .rlv and, for each of those files, find the corresponding
 * .pgmf and .avi file. The result will be a list of RealLifeVideo objects. Only slope-based files will be found,
 * power-based rlv files will be ommitted for now.
 */
class RealLifeVideoImporter: public QObject
{
    Q_OBJECT
public:
    explicit RealLifeVideoImporter(QObject* parent = NULL);

    virtual ~RealLifeVideoImporter();

    /**
     * @brief parse the Real Life Videos from a root directory.
     * @param the root directory to search files from.
     */
    void importRealLiveVideoFilesFromDir();

signals:
    /**
      * the number of rlvs to be imported.
      */
    void rlvFilesFound(int numberOfRlvs);
    /**
     * emitted when an RLV has been imported.
     */
    void rlvImported();
    /**
     * @brief signal emitted when the import is finished.
     * @param rlvs list of RealLifeVideo objects.
     */
    void importFinished(RealLifeVideoList rlvs);

protected:
    virtual bool event(QEvent *event);
private:
    RealLifeVideoList importRlvFiles(const QStringList &rootFolders);
    void importReady(const RealLifeVideoList &rlvs);
};

#endif // REALLIVEVIDEOPARSER_H
