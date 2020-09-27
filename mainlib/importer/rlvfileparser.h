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

#ifndef RLVFILEPARSER_H
#define RLVFILEPARSER_H

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTextCodec>
#include <QtCore/QtDebug>
#include <QtCore/QStringList>

#include "model/reallifevideo.h"

namespace tacxfile {
struct headerBlock;
struct infoBlock;
struct generalRlvBlock;
struct programBlock;
struct generalProfileBlock;
struct informationBox;
struct InformationBoxCommand;

class DistanceMappingEntry;
}

class TacxFileParser
{
protected:
    tacxfile::headerBlock readHeaderBlock(QFile& rlvFile);
    tacxfile::infoBlock readInfoBlock(QFile& rlvFile);

};

class RlvFileParser: public TacxFileParser
{
public:
    RlvFileParser(const QList<QFileInfo>& pgmfFiles, const QList<QFileInfo>& videoFiles);

    RealLifeVideo parseRlvFile(QFile& rlvFile);

    tacxfile::generalRlvBlock readGeneralRlvBlock(QFile& rlvFile);
    std::vector<Course> readCourseInformation(QFile& rlvFile, qint32 count);
    std::vector<DistanceMappingEntry> readFrameDistanceMapping(QFile& rlvFile, qint32 count);

private:
    QString findVideoFilename(const QList<QFileInfo> &videoFiles, const QString& rlvVideoFilename);
    QFileInfo findVideoFileInfo(const QList<QFileInfo> &videoFiles, const QString& rlvVideoFilename);
    QFileInfo findCommandListFileInfo(const QDir &rlvCommandListDir);
    std::vector<tacxfile::InformationBoxCommand> readInfoBoxCommands(const QFileInfo &commandListFileInfo, const QDir &infoBoxRootDir);
    QFileInfo findPgmfFile(QFile& rlvFile);
    std::vector<DistanceMappingEntry> calculateRlvDistanceMappings(const std::vector<tacxfile::DistanceMappingEntry> &tacxDistanceMappings) const;
    std::vector<tacxfile::informationBox> readTacxInformationBoxes(QFile &rlvFile, qint32 count);
    std::vector<InformationBox> readInformationBoxesContent(const std::vector<tacxfile::informationBox> &informationBoxes,
                                                            const QDir &infoBoxRootDir, const QString &rlvName);

    const std::map<QString,QFileInfo> _pgmfFiles;
    const QList<QFileInfo> _videoFiles;
};

class PgmfFileParser: public TacxFileParser
{
public:
    Profile readProfile(QFile& pgmfFile);

private:
    tacxfile::generalProfileBlock readGeneralPgmfInfo(QFile& pgmfFile);
    std::vector<tacxfile::programBlock> readProgram(QFile& pgmfFile, quint32 count);

};

#endif // RLVFILEPARSER_H
