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

#include "rlvfileparser.h"
#include "util/util.h"
#include <QDir>
#include <QFileInfo>
#include <QtGui/QImage>
#include <QtDebug>
#include <utility>

#include "model/distancemappingentry.h"
#include "model/videoinformation.h"

namespace
{
const QString INFO_BOX_IMAGE_SOURCE = "<img src=\"";
const QString INFO_BOX_TEXT = "TEXT(\"";

template <typename T>
T readBlock(QFile &file)
{
    T block;
    file.read(reinterpret_cast<char*>(&block), sizeof(block));
    return block;
}

std::map<QString,QFileInfo> toFileInfoMap(const QList<QFileInfo> &pgmfFiles)
{
    std::function<QString(QFileInfo)> keyFunction([](const QFileInfo& fileInfo) {
        return fileInfo.baseName();
    });

    return indoorcycling::toMap(std::vector<QFileInfo>(pgmfFiles.begin(), pgmfFiles.end()),
                                                           keyFunction);
}

}
namespace tacxfile
{

QString fromUtf16(const char* string, size_t size);

struct headerBlock{
    qint16 fingerprint;
    qint16 version;
    qint32 numberOfBlocks;

    QString toString() const {
        return QString("fingerprint %1, version %2, number of blocks %3")
                .arg(fingerprint).arg(version).arg(numberOfBlocks);
    }
};

struct infoBlock{
    qint16 fingerprint;
    qint16 version;
    qint32 numberOfRecords;
    qint32 recordSize;

    qint32 size() const {
        return numberOfRecords * recordSize;
    }

    QString toString() const {
        return QString("block fingerprint %1, block version %2, number of records %3, record size %4")
                .arg(fingerprint).arg(version).arg(numberOfRecords).arg(recordSize);
    }
};

struct generalRlvBlock {
    char _filename[522];
    float frameRate;
    float originalRunWeight;
    qint32 frameOffset;

    QString filename() const {
        return fromUtf16(_filename, sizeof(_filename));
    }

    QString toString() const {
        return QString("video file name: %1, frame rate %2").arg(filename()).arg(frameRate);
    }
};

struct rlvCourseInformation_t {
    float start;
    float end;
    char _courseName[66];
    quint8 _textFilename[522];

    QString courseName() const {
        return fromUtf16(_courseName, sizeof(_courseName));
    }

    QString toString() const {
        return QString("start: %1, end %2, name: %3")
                .arg(start).arg(end).arg(courseName());
    }
};

struct frameDistanceMapping_t {
    quint32 frameNumber;
    float metersPerFrame;

    QString toString() const {
        return QString("\tframeNumber %1, meters per frame %2").arg(frameNumber).arg(metersPerFrame);
    }
};

struct informationBox {
    qint32 frameNumber;
    qint32 commandNr;
};

struct InformationBoxCommand {
    QString text;
    QFileInfo imageFileInfo;
};

struct generalProfileBlock {
    quint32 checksum;
    char _courseName[34];
    qint32 powerSlopeOrHr = 1;
    qint32 _timeOrDistance;
    double _totalTimeOrDistance;
    double energyCons;
    float startAltitude;
    qint32 breakCategory;

    QString courseName() const {
        return fromUtf16(_courseName, sizeof(_courseName));
    }

    QString type() const {
        if (powerSlopeOrHr == 0)
            return QString("Power");
        if (powerSlopeOrHr == 1)
            return QString("Slope");
        else
            return QString("HR");
    }

    QString timeOrDistance() const {
        if (_timeOrDistance == 0)
            return "Time";
        else
            return "Distance";
    }

    QString totalTimeOrDistance() const {
        if (_timeOrDistance == 0)
            return QString("%1 seconds").arg(_totalTimeOrDistance);
        else
            return QString("%1 km").arg(_totalTimeOrDistance/1000.0);
    }

    QString toString() const {
        return QString("General pgmf: %1, type: %2, %3-based, start alt: %4")
                .arg(courseName())
                .arg(type())
                .arg(timeOrDistance())
                .arg(startAltitude);
    }
};

struct programBlock{
    float durationDistance;
    float powerSlopeHeartRate;
    float rollingFriction;

    QString toString() const {
        return QString("duration or distance: %1, value: %2, friction: %3")
                .arg(durationDistance).arg(powerSlopeHeartRate)
                .arg(rollingFriction);
    }
};


QString fromUtf16(const char* string, size_t size) {
    QByteArray bytes;

    size_t i = 0;
    while(string[i] != '\0' && i < size) {
        bytes.append(string[i]);
        bytes.append(string[i+1]);
        i += 2;
    }
    return QTextCodec::codecForName("UTF-16")->toUnicode(bytes);
}

class DistanceMappingEntry {
public:
    explicit DistanceMappingEntry(quint32 frameNumber, float metersPerFrame) {
        _frameNumber = frameNumber;
        _metersPerFrame = metersPerFrame;
    }

    quint32 frameNumber() const { return _frameNumber; }
    float metersPerFrame() const { return _metersPerFrame; }
private:
    quint32 _frameNumber;
    float _metersPerFrame;
};

/**
 * There can be two (? really ?) types of line in the CmdList.txt files:
 * TEXT("<img src="ImgG/Map.png" width="567" height="285">");HIDE("6");
 * TEXT("St-Michel-de-Maurienne<br>  Alt 731m");HIDE("7");
 *
 * We will first extract the part between TEXT(" and the "), resulting in this:
 * "<img src="ImgG/Map.png" width="567" height="285">"
 * "St-Michel-de-Maurienne<br>  Alt 731m"
 *
 * For images, we will extract only the filename, Map.png in this case. This is prepended
 * with "$infobox_root_dir/img/" to make the complete path to image files.
 * For text entries, we just return this text. The info box can handle displaying html.
 */
const InformationBoxCommand extractCommandFromLine(const QString &line, const QDir &infoBoxRootDir) {
    int startIndex = QString(INFO_BOX_TEXT).length();
    int endIndex = line.indexOf("\")");
    QString text = line.mid(startIndex, endIndex - startIndex);
    QFileInfo imageFileInfo;
    if (text.contains("<img src=\"")) {
        int imageSourceStart = text.indexOf(INFO_BOX_IMAGE_SOURCE) + INFO_BOX_IMAGE_SOURCE.length();
        QString imageSource = text.mid(imageSourceStart);
        int imageSourceEnd = imageSource.indexOf("\"");
        imageSource = imageSource.left(imageSourceEnd);
        int slashIndex = imageSource.lastIndexOf("/");
        imageSource = imageSource.mid(slashIndex + 1);
        QDir imageDir = infoBoxRootDir;
        bool imageDirOk = imageDir.cd("Img") || imageDir.cd("img");
        if (imageDirOk) {
            imageFileInfo = QFileInfo(imageDir, imageSource);
            text.clear();
        }
    }
    return { text, imageFileInfo };
}
}

RlvFileParser::RlvFileParser(const QList<QFileInfo> &pgmfFiles, const QList<QFileInfo>& videoFiles): TacxFileParser(),
    _pgmfFiles(toFileInfoMap(pgmfFiles)), _videoFiles(videoFiles)
{

}

QString RlvFileParser::findVideoFilename(const QList<QFileInfo>& videoFiles, const QString& rlvVideoFilename)
{
    return findVideoFileInfo(videoFiles, rlvVideoFilename).filePath();
}

QFileInfo RlvFileParser::findVideoFileInfo(const QList<QFileInfo> &videoFiles, const QString &rlvVideoFilename)
{
    QString normalizedVideoFilename;
    // if rlv video filename contains a path, get only the last part of it.
    normalizedVideoFilename = rlvVideoFilename.split("\\").last();

    foreach(const QFileInfo& fileInfo, videoFiles) {
        if (fileInfo.fileName().toLower() == normalizedVideoFilename.toLower())
            return fileInfo;
    }

    return QFileInfo();
}

QFileInfo RlvFileParser::findCommandListFileInfo(const QDir &rlvCommandListDir)
{
    QDir dir(rlvCommandListDir);
    if (dir.cd("EN") || dir.cd("en")) {
        QFileInfo commandListFileInfo(dir, "CmdList.txt");
        if (commandListFileInfo.exists()) {
            return commandListFileInfo;
        }
    }

    return QFileInfo();
}

std::vector<tacxfile::InformationBoxCommand> RlvFileParser::readInfoBoxCommands(const QFileInfo &commandListFileInfo,
                                                                                const QDir &infoBoxRootDir)
{
    std::vector<tacxfile::InformationBoxCommand> commands;
    if (commandListFileInfo.exists()) {
        QFile commandListFile(commandListFileInfo.absoluteFilePath());
        if (commandListFile.open(QFile::ReadOnly)) {
            QTextStream in(&commandListFile);
            while (!in.atEnd()) {
                QString line = in.readLine();
                if (line.startsWith(INFO_BOX_TEXT)) {
                    commands.push_back(tacxfile::extractCommandFromLine(line, infoBoxRootDir));
                }
            }
        }

    }
    return commands;
}

/**
 * Find the pgmf file for an rlv file. This checks if any of the pgmf files that were
 * found has the same name (minus the extension) as the rlv file. If found, returns
 * the path, else, returns an empty string.
 */
QFileInfo RlvFileParser::findPgmfFile(QFile &rlvFile)
{
    QFileInfo fileInfo(rlvFile);

    QString baseName = fileInfo.baseName();
    auto it = _pgmfFiles.find(fileInfo.baseName());
    if (it == _pgmfFiles.end()) {
        return QFileInfo();
    }
    return (*it).second;
}

RealLifeVideo RlvFileParser::parseRlvFile(QFile &rlvFile)
{
    qDebug() << rlvFile.fileName();
    if (!rlvFile.open(QIODevice::ReadOnly))
        return RealLifeVideo();

    QFile pgmfFile(findPgmfFile(rlvFile).filePath());
    if (!pgmfFile.open(QIODevice::ReadOnly))
        return RealLifeVideo();

    Profile profile = PgmfFileParser().readProfile(pgmfFile);

    std::vector<Course> courses;
    QString name = QFileInfo(rlvFile).baseName();
    VideoInformation videoInformation(QString("Unknown"), 0.0);
    std::vector<DistanceMappingEntry> distanceMapping;
    std::vector<tacxfile::informationBox> informationBoxes;

    tacxfile::headerBlock header = readHeaderBlock(rlvFile);
    for(qint32 blockNr = 0; blockNr < header.numberOfBlocks; ++blockNr) {
        tacxfile::infoBlock infoBlock = readInfoBlock(rlvFile);
        if (infoBlock.fingerprint < 0 || infoBlock.fingerprint > 10000)
            break;
        if (infoBlock.fingerprint == 2010) {
            tacxfile::generalRlvBlock generalRlv = readGeneralRlvBlock(rlvFile);
            QString videoFilename = findVideoFilename(_videoFiles, generalRlv.filename());
            videoInformation = VideoInformation(videoFilename, generalRlv.frameRate);
        }
        else if (infoBlock.fingerprint == 2020) {
            distanceMapping = readFrameDistanceMapping(rlvFile, infoBlock.numberOfRecords);
        } else if (infoBlock.fingerprint == 2030) {
            informationBoxes = readTacxInformationBoxes(rlvFile, infoBlock.numberOfRecords);

        }
        else if (infoBlock.fingerprint == 2040) {
            courses = readCourseInformation(rlvFile, infoBlock.numberOfRecords);
        }
        else {
            rlvFile.read(infoBlock.numberOfRecords * infoBlock.recordSize);
        }
    }

    QDir infoBoxRootDir = QFileInfo(videoInformation.videoFilename()).dir();
    std::vector<InformationBox> rlvInformationBoxes =
            readInformationBoxesContent(informationBoxes, infoBoxRootDir, name);

    return RealLifeVideo(name, RealLifeVideoFileType::TACX,
                         videoInformation, std::move(courses), std::move(distanceMapping), profile, std::move(rlvInformationBoxes));
}

tacxfile::headerBlock TacxFileParser::readHeaderBlock(QFile &rlvFile)
{
    tacxfile::headerBlock headerBlock;
    rlvFile.read((char*) &headerBlock, sizeof(headerBlock));

    return headerBlock;
}

tacxfile::infoBlock TacxFileParser::readInfoBlock(QFile &rlvFile)
{
    tacxfile::infoBlock infoBlock;
    rlvFile.read((char*) &infoBlock, sizeof(infoBlock));
    return infoBlock;
}

tacxfile::generalRlvBlock RlvFileParser::readGeneralRlvBlock(QFile &rlvFile)
{
    tacxfile::generalRlvBlock generalBlock;
    rlvFile.read((char*) &generalBlock._filename, 522);
    rlvFile.read((char*) &generalBlock.frameRate, sizeof(float));
    rlvFile.read((char*) &generalBlock.originalRunWeight, sizeof(float));
    rlvFile.read((char*) &generalBlock.frameOffset, sizeof(qint32));

    return generalBlock;
}

std::vector<Course> RlvFileParser::readCourseInformation(QFile &rlvFile, qint32 count)
{
    std::vector<Course> courses;
    for (qint32 i = 0; i < count; i++) {
        tacxfile::rlvCourseInformation_t courseInfoBlock;
        memset(&courseInfoBlock, 0, sizeof(courseInfoBlock));
        rlvFile.read((char*) &courseInfoBlock.start, sizeof(float));
        rlvFile.read((char*) &courseInfoBlock.end, sizeof(float));
        rlvFile.read((char*) &courseInfoBlock._courseName, 66);
        rlvFile.read((char*) &courseInfoBlock._textFilename, 522);

        courses.push_back(Course(courseInfoBlock.courseName(), courseInfoBlock.start, courseInfoBlock.end));
    }
    return courses;
}

std::vector<DistanceMappingEntry> RlvFileParser::readFrameDistanceMapping(QFile &rlvFile, qint32 count)
{
    std::vector<tacxfile::DistanceMappingEntry> mappings;
    mappings.reserve(count);

    for (qint32 i = 0; i < count; i++) {
        tacxfile::frameDistanceMapping_t frameDistanceMappingBlock;
        rlvFile.read((char*) &frameDistanceMappingBlock, sizeof(frameDistanceMappingBlock));
        mappings.push_back(tacxfile::DistanceMappingEntry(frameDistanceMappingBlock.frameNumber, frameDistanceMappingBlock.metersPerFrame));
    }
    return calculateRlvDistanceMappings(mappings);
}

std::vector<DistanceMappingEntry> RlvFileParser::calculateRlvDistanceMappings(const std::vector<tacxfile::DistanceMappingEntry> &tacxDistanceMappings) const
{
    float currentDistance = 0;
    float lastMetersPerFrame = 0;

    std::vector<DistanceMappingEntry> distanceMappings;
    if (!tacxDistanceMappings.empty()) {
        quint32 lastFrameNumber = tacxDistanceMappings[0].frameNumber();
        for (const auto &entry: tacxDistanceMappings) {
            quint32 nrFrames = entry.frameNumber() - lastFrameNumber;
            currentDistance += nrFrames * lastMetersPerFrame;
            distanceMappings.push_back(DistanceMappingEntry(currentDistance, entry.frameNumber(), entry.metersPerFrame()));

            lastMetersPerFrame = entry.metersPerFrame();
            lastFrameNumber = entry.frameNumber();
        }
    }
    return distanceMappings;
}

std::vector<tacxfile::informationBox> RlvFileParser::readTacxInformationBoxes(QFile &rlvFile, qint32 count)
{
    std::vector<tacxfile::informationBox> informationBoxes;
    for (auto i = 0; i < count; ++i) {
        tacxfile::informationBox informationBox = readBlock<tacxfile::informationBox>(rlvFile);
        informationBoxes.push_back(informationBox);
    }
    return informationBoxes;
}

std::vector<InformationBox> RlvFileParser::readInformationBoxesContent(const std::vector<tacxfile::informationBox> &informationBoxes, const QDir &infoBoxRootDir, const QString &rlvName)
{
    std::vector<InformationBox> rlvInformationBoxes;
    QDir rootDir = infoBoxRootDir;
    if (rootDir.cd(rlvName)) {
        QFileInfo commandListFileInfo = findCommandListFileInfo(rootDir);
        std::vector<tacxfile::InformationBoxCommand> tacxInfoBoxCommands =
                readInfoBoxCommands(commandListFileInfo, rootDir);

        for (unsigned i = 0; i < tacxInfoBoxCommands.size(); ++i) {
            if (i < informationBoxes.size()) {
                rlvInformationBoxes.push_back(InformationBox(informationBoxes[i].frameNumber,
                                                             0, // no distance known at this point
                                                             tacxInfoBoxCommands[i].text,
                                                             tacxInfoBoxCommands[i].imageFileInfo));
            }
        }
    }
    return rlvInformationBoxes;
}


Profile PgmfFileParser::readProfile(QFile &pgmfFile)
{
    tacxfile::headerBlock header = readHeaderBlock(pgmfFile);
    tacxfile::generalProfileBlock generalBlock;
    std::vector<tacxfile::programBlock> profileBlocks;

    for(qint32 blockNr = 0; blockNr < header.numberOfBlocks; ++blockNr) {
        tacxfile::infoBlock infoBlock = readInfoBlock(pgmfFile);
        if (infoBlock.fingerprint < 0 || infoBlock.fingerprint > 10000)
            break;
        else if (infoBlock.fingerprint == 1010)
            generalBlock = readGeneralPgmfInfo(pgmfFile);
        else if (infoBlock.fingerprint == 1020)
            profileBlocks = readProgram(pgmfFile, infoBlock.numberOfRecords);
        else
            pgmfFile.read(infoBlock.numberOfRecords * infoBlock.recordSize);
    }

    std::vector<ProfileEntry> profile;
    if (generalBlock.powerSlopeOrHr == 1) {
        float currentDistance = 0;
        float currentAltitude = 0;
        for (const auto &entry: profileBlocks) {
            profile.push_back(ProfileEntry(currentDistance, entry.powerSlopeHeartRate, currentAltitude));
            currentDistance += entry.durationDistance;
            currentAltitude += entry.powerSlopeHeartRate * .01f * entry.durationDistance;
        }
    }
    ProfileType type = static_cast<ProfileType>(generalBlock.powerSlopeOrHr);

    return Profile(type, generalBlock.startAltitude, std::move(profile));
}

tacxfile::generalProfileBlock PgmfFileParser::readGeneralPgmfInfo(QFile &pgmfFile)
{
    tacxfile::generalProfileBlock generalBlock;
    pgmfFile.read((char*) &generalBlock.checksum, sizeof(qint32));
    pgmfFile.read((char*) &generalBlock._courseName, 34);
    pgmfFile.read((char*) &generalBlock.powerSlopeOrHr, sizeof(generalBlock) - offsetof(tacxfile::generalProfileBlock, powerSlopeOrHr));

    return generalBlock;
}

std::vector<tacxfile::programBlock> PgmfFileParser::readProgram(QFile &pgmfFile, quint32 count)
{
    std::vector<tacxfile::programBlock> programBlocks;
    programBlocks.reserve(count);

    for (quint32 i = 0; i < count; ++i) {
        tacxfile::programBlock programBlock;
        pgmfFile.read((char*) &programBlock, sizeof(programBlock));
        programBlocks.push_back(programBlock);
    }
    return programBlocks;
}
