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

#include "config/bigringsettings.h"
#include "importer/gpxfileparser.h"
#include "importer/rlvfileparser.h"
#include "importer/virtualtrainingfileparser.h"
#include "reallifevideoimporter.h"
#include "reallifevideocache.h"

#include <functional>

#include <QtCore/QCoreApplication>
#include <QtCore/QDirIterator>
#include <QtCore/QEvent>
#include <QtCore/QSettings>

#include <QStringList>
#include <QtConcurrent/QtConcurrentMap>
#include <QtConcurrent/QtConcurrentRun>
#include <QtDebug>

namespace
{
RealLifeVideo parseRealLiveVideoFile(QFile &rlvFile, const QList<QString> &videoFilePaths, const QList<QString> &pgmfFilePaths);
QSet<QString> findFiles(const QStringList &roots, const QStringList &patterns);
QSet<QString> findRlvFiles(const QStringList &roots);

const QEvent::Type NR_OF_RLVS_FOUND_TYPE = static_cast<QEvent::Type>(QEvent::User + 100);
const QEvent::Type RLV_IMPORTED_TYPE = static_cast<QEvent::Type>(NR_OF_RLVS_FOUND_TYPE + 101);

class NrOfRlvsFoundEvent: public QEvent
{
public:
    NrOfRlvsFoundEvent(int nr): QEvent(NR_OF_RLVS_FOUND_TYPE), _nr(nr) {}

    int _nr;
};
class RlvImportedEvent: public QEvent
{
public:
    RlvImportedEvent(): QEvent(RLV_IMPORTED_TYPE) {}
};
}

RealLifeVideoImporter::RealLifeVideoImporter(QObject* parent): QObject(parent)
{
    // empty
}

RealLifeVideoImporter::~RealLifeVideoImporter()
{
    qDebug() << "deleting RealLifeVideoImporter";
}

void RealLifeVideoImporter::importRealLiveVideoFilesFromDir()
{
    QFutureWatcher<RealLifeVideoList> *futureWatcher = new QFutureWatcher<RealLifeVideoList>();
    connect(futureWatcher, &QFutureWatcher<RealLifeVideoList>::finished, futureWatcher, [=]() {
        importReady(futureWatcher->future().result());
        futureWatcher->deleteLater();
    });

    futureWatcher->setFuture(QtConcurrent::run([this]() {
        return this->importRlvFiles(BigRingSettings().videoFolders());
    }));
}

bool RealLifeVideoImporter::event(QEvent *event)
{
    if (event->type() == NR_OF_RLVS_FOUND_TYPE) {
        emit rlvFilesFound(dynamic_cast<NrOfRlvsFoundEvent*>(event)->_nr);
        return true;
    } else if (event->type() == RLV_IMPORTED_TYPE) {
        emit rlvImported();
        return true;
    } else {
        return QObject::event(event);
    }
}

RealLifeVideoList RealLifeVideoImporter::importRlvFiles(const QStringList& rootFolders)
{

    const QSet<QString> rlvFiles = findRlvFiles(rootFolders);
    qDebug() << "rlv files" << rlvFiles;
    const QSet<QString> pgmfFiles = findFiles(rootFolders, { "*.pgmf" });
    const QSet<QString> aviFiles = findFiles(rootFolders, { "*.avi", "*.mp4" });

    QCoreApplication::postEvent(this, new NrOfRlvsFoundEvent(rlvFiles.size()));

    std::function<RealLifeVideo(const QFileInfo&)> importFunction([this, aviFiles, pgmfFiles](const QFileInfo& fileInfo) -> RealLifeVideo {
        QFile file(fileInfo.canonicalFilePath());
        RealLifeVideo rlv = parseRealLiveVideoFile(file, aviFiles.toList(), pgmfFiles.toList());
        QCoreApplication::postEvent(this, new RlvImportedEvent);
        return rlv;
    });

    return QtConcurrent::mapped(rlvFiles.begin(), rlvFiles.end(), importFunction).results();
}


void RealLifeVideoImporter::importReady(const RealLifeVideoList &rlvs)
{

    RealLifeVideoList validRlvs;

    for (const RealLifeVideo& rlv: rlvs) {
        if (rlv.isValid() && rlv.type() == ProfileType::SLOPE) {
            validRlvs.append(rlv);
        }
    }
    // sort rlv list by name
    qSort(validRlvs.begin(), validRlvs.end(), RealLifeVideo::compareByName);

    emit importFinished(validRlvs);
}

namespace
{

QList<QFileInfo> fromPaths(const QList<QString>& filePaths) {
    QList<QFileInfo> fileInfos;
    std::for_each(filePaths.constBegin(), filePaths.constEnd(), [&fileInfos](QString filePath) {
        fileInfos.append(QFileInfo(filePath));
    });
    return fileInfos;
}

RealLifeVideo parseRealLiveVideoFile(QFile &rlvFile, const QList<QString>& videoFilePaths,
                                     const QList<QString>& pgmfFilePaths)
{
    QDateTime start = QDateTime::currentDateTime();
    QList<QFileInfo> videoFiles = fromPaths(videoFilePaths);
    QList<QFileInfo> pgmfFiles = fromPaths(pgmfFilePaths);

    RealLifeVideo rlv;
    const auto fromCache = RealLifeVideoCache().load(rlvFile);
    if (fromCache) {
        rlv = *fromCache;
    } else if (rlvFile.fileName().endsWith(".rlv")) {
        rlv = RlvFileParser(pgmfFiles, videoFiles).parseRlvFile(rlvFile);
    } else if (rlvFile.fileName().endsWith(".xml")) {
        rlv = indoorcycling::VirtualTrainingFileParser(videoFiles).parseVirtualTrainingFile(rlvFile);
    } else if (rlvFile.fileName().endsWith(".gpx")) {
        rlv = indoorcycling::GpxFileParser(videoFiles).parseGpxFile(rlvFile);
    }
    if (rlv.isValid()) {
        // if there was no cache file, create it now.
        if (!fromCache) {
            RealLifeVideoCache().save(rlvFile, rlv);
        }

        QSettings settings;
        settings.beginGroup(QString("%1.custom_courses").arg(rlv.name()));
        QStringList customCourseNames = settings.allKeys();
        for (QString customCourseName: customCourseNames) {
            if (!customCourseName.endsWith("_end")) {
                int startDistance = settings.value(customCourseName).toInt();
                if (settings.contains(customCourseName + "_end")) {
                    int endDistance = settings.value(customCourseName + "_end").toInt();
                    rlv.addCustomCourse(startDistance, endDistance, customCourseName);
                } else {
                    rlv.addStartPoint(startDistance, customCourseName);
                }
            }
        }
        settings.endGroup();
    }
    QDateTime end = QDateTime::currentDateTime();
    qDebug() << "import of" << rlvFile.fileName() << "took" << start.msecsTo(end) << "ms";
    return rlv;
}

QSet<QString> findFiles(const QStringList& roots, const QStringList& patterns)
{
    QSet<QString> filePaths;
    for (const QString &root: roots) {
        QDirIterator it(root, patterns, QDir::NoFilter, QDirIterator::Subdirectories);

        while(it.hasNext()) {
            it.next();
            filePaths.insert(it.filePath());
        }
    }
    return filePaths;
}

QSet<QString> findRlvFiles(const QStringList& roots)
{
    return findFiles(roots, { "*.rlv", "*.xml", "*.gpx" } );
}

}
