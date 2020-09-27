#include "virtualtrainingfileparser.h"

#include "model/distancemappingentry.h"
#include "model/videoinformation.h"

#include <QtCore/QtDebug>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamAttributes>

namespace {
QString readSingleAttribute(const QXmlStreamAttributes& attributes, const QString& attributeName) {
    for (auto attribute: attributes) {
        if (attribute.name().toString() == attributeName) {
            return attribute.value().toString();
        }
    }
    return QString();
}

bool isElement(const QXmlStreamReader::TokenType currentTokenType, const QXmlStreamReader& reader, const QString &elementName)
{
    return (currentTokenType == QXmlStreamReader::StartElement && reader.name() == elementName);
}
}
namespace indoorcycling {

namespace virtualtrainingfileparser {
struct ProfileEntry {
    float distance;
    float altitude;
};
struct DistanceMappingEntry {
    float frame;
    float distance;
};
}
VirtualTrainingFileParser::VirtualTrainingFileParser(const QList<QFileInfo> &videoFiles, QObject *parent) :
    QObject(parent), _videoFilesInfo(videoFiles)
{
}

RealLifeVideo VirtualTrainingFileParser::parseVirtualTrainingFile(QFile &inputFile) const
{
    inputFile.open(QIODevice::ReadOnly);
    QTextStream textStream(&inputFile);
    QString xml = textStream.readAll();
    QXmlStreamReader reader(xml);
    RealLifeVideo rlv = parseXml(reader);
    return rlv;
}

RealLifeVideo VirtualTrainingFileParser::parseXml(QXmlStreamReader &reader) const
{
    QString name;
    QString videoFilePath;
    float frameRate = 0;
    Profile profile;
    std::vector<Course> courses;
    std::vector<DistanceMappingEntry> distanceMappings;
    std::vector<InformationBox> informationBoxes;
    std::vector<GeoPosition> positions;

    QXmlStreamReader::TokenType currentTokenType;
    while (!reader.atEnd()) {
        currentTokenType = reader.readNext();
        if (isElement(currentTokenType, reader, "name")) {
            name = reader.readElementText();
        } else if (isElement(currentTokenType, reader, "video-file-path")) {
            QString videoFileName = reader.readElementText();
            videoFilePath = findVideoFile(videoFileName);
        } else if (isElement(currentTokenType, reader, "framerate")) {
            frameRate = reader.readElementText().toFloat();
        } else if (isElement(currentTokenType, reader, "altitudes")) {
            std::vector<ProfileEntry> profileEntries = convertProfileEntries(readProfileEntries(reader));
            profile = Profile(ProfileType::SLOPE, 0, std::move(profileEntries));
        } else if (isElement(currentTokenType, reader, "segments")) {
            courses = readCourses(reader);
        } else if (isElement(currentTokenType, reader, "mappings")) {
            distanceMappings = convertDistanceMappings(readDistanceMappings(reader));
        } else if (isElement(currentTokenType, reader, "informations")) {
            informationBoxes = readInformationBoxes(reader);
        } else if (isElement(currentTokenType, reader, "positions")) {
            positions = readPositions(reader);
        } else {
//            qDebug() << "unknown token" << reader.name();
        }

    }
    if (reader.hasError()) {
        qDebug() << "error!" << reader.errorString();
        // Error handling..?
        return RealLifeVideo();
    }
    VideoInformation videoInformation(videoFilePath, frameRate);

    return RealLifeVideo(name, RealLifeVideoFileType::VIRTUAL_TRAINING, videoInformation,
                         std::move(courses), std::move(distanceMappings), profile,
                         std::move(informationBoxes),
                         std::move(positions));
}

std::vector<virtualtrainingfileparser::ProfileEntry> VirtualTrainingFileParser::readProfileEntries(QXmlStreamReader &reader) const
{
    std::vector<virtualtrainingfileparser::ProfileEntry> profileEntries;
    while(!reader.atEnd()) {
        QXmlStreamReader::TokenType tokenType = reader.readNext();
        if (isElement(tokenType, reader, "altitude")) {
            virtualtrainingfileparser::ProfileEntry entry;
            entry.distance = readSingleAttribute(reader.attributes(), "distance").toFloat();
            entry.altitude = readSingleAttribute(reader.attributes(), "height").toFloat();
            profileEntries.push_back(entry);
        } else if (tokenType == QXmlStreamReader::EndElement && reader.name() == "altitudes") {
            break;
        }
    }
    return profileEntries;
}

std::vector<ProfileEntry> VirtualTrainingFileParser::convertProfileEntries(const std::vector<virtualtrainingfileparser::ProfileEntry> &virtualTrainingProfileEntries) const
{
    std::vector<ProfileEntry> profileEntries;
    profileEntries.reserve(virtualTrainingProfileEntries.size());

    const virtualtrainingfileparser::ProfileEntry *lastEntry = nullptr;
    float currentDistance;
    float currentAltitude;
    for (const virtualtrainingfileparser::ProfileEntry &profileEntry: virtualTrainingProfileEntries) {
        if (lastEntry) {
            float segmentDistance = profileEntry.distance - currentDistance;
            float segmentAltitudeDifference = profileEntry.altitude - currentAltitude;

            float slope = segmentAltitudeDifference / segmentDistance;
            float slopeInPercent = slope * 100.0f;

            profileEntries.push_back(ProfileEntry(currentDistance, slopeInPercent, currentAltitude));
        }
        currentDistance = profileEntry.distance;
        currentAltitude = profileEntry.altitude;
        lastEntry = &profileEntry;
    }

    // if there was an entry, and there's at least one entry in profile entries, add a last entry
    // to profile entries with the same slope as the previous one
    if (lastEntry && !profileEntries.empty()) {
        profileEntries.push_back(ProfileEntry(currentDistance, profileEntries.back().slope(),
                                           lastEntry->altitude));
    }

    return profileEntries;
}

QString VirtualTrainingFileParser::findVideoFile(QString filename) const
{
    for (const QFileInfo &fileInfo: _videoFilesInfo) {
        if (fileInfo.fileName().toLower() == filename.toLower()) {
            return fileInfo.absoluteFilePath();
        }
    }
    return "";
}

std::vector<Course> VirtualTrainingFileParser::readCourses(QXmlStreamReader &reader) const
{
    std::vector<Course> courses;
    while(!reader.atEnd()) {
        QXmlStreamReader::TokenType tokenType = reader.readNext();
        if (isElement(tokenType, reader, "segment")) {
            QString name = readSingleAttribute(reader.attributes(), "name");
            float start = readSingleAttribute(reader.attributes(), "start").toFloat();
            float end = readSingleAttribute(reader.attributes(), "end").toFloat();

            courses.push_back(Course(name, start, end));
        } else if (tokenType == QXmlStreamReader::EndElement && reader.name() == "segments") {
            break;
        }
    }
    return courses;
}

std::vector<InformationBox> VirtualTrainingFileParser::readInformationBoxes(QXmlStreamReader &reader) const
{
    std::vector<InformationBox> informationBoxes;
    while(!reader.atEnd()) {
        QXmlStreamReader::TokenType tokenType = reader.readNext();
        if (isElement(tokenType, reader, "information")) {
            float distance = readSingleAttribute(reader.attributes(), "distance").toFloat();
            QString message = readSingleAttribute(reader.attributes(), "en");

            // messages are just text, but want to have rich text (with html text). We'll
            // replace new lines with <br> to fake it.
            message.replace("\\n", "<br>");

            informationBoxes.push_back(InformationBox(0, distance, message, QFileInfo()));
        } else if (tokenType == QXmlStreamReader::EndElement && reader.name() == "informations") {
            break;
        }
    }
    return informationBoxes;
}

std::vector<GeoPosition> VirtualTrainingFileParser::readPositions(QXmlStreamReader &reader) const
{
    std::vector<GeoPosition> positions;
    while(!reader.atEnd()) {
        QXmlStreamReader::TokenType tokenType = reader.readNext();
        if (isElement(tokenType, reader, "position")) {
            const qreal distance = readSingleAttribute(reader.attributes(), "distance").toDouble();
            const qreal latitude = readSingleAttribute(reader.attributes(), "lat").toDouble();
            const qreal longitude = readSingleAttribute(reader.attributes(), "lon").toDouble();

            positions.push_back(GeoPosition(distance, QGeoCoordinate(latitude, longitude)));
        } else if (tokenType == QXmlStreamReader::EndElement && reader.name() == "positions") {
            break;
        }
    }
    return positions;
}

std::vector<virtualtrainingfileparser::DistanceMappingEntry> VirtualTrainingFileParser::readDistanceMappings(QXmlStreamReader &reader) const
{
    std::vector<virtualtrainingfileparser::DistanceMappingEntry> entries;
    while(!reader.atEnd()) {
        QXmlStreamReader::TokenType tokenType = reader.readNext();
        if (isElement(tokenType, reader, "mapping")) {
            virtualtrainingfileparser::DistanceMappingEntry entry;
            entry.distance = readSingleAttribute(reader.attributes(), "distance").toFloat();
            entry.frame = readSingleAttribute(reader.attributes(), "frame").toFloat();
            entries.push_back({entry});
        } else if (tokenType == QXmlStreamReader::EndElement && reader.name() == "mappings") {
            break;
        }
    }
    return entries;
}

std::vector<DistanceMappingEntry> VirtualTrainingFileParser::convertDistanceMappings(const std::vector<virtualtrainingfileparser::DistanceMappingEntry> &vcEntries) const
{
    std::vector<DistanceMappingEntry> mappings;
    mappings.reserve(vcEntries.size());

    const virtualtrainingfileparser::DistanceMappingEntry *lastEntry = nullptr;
    float currentDistance;
    quint32 currentFrame;
    for (const virtualtrainingfileparser::DistanceMappingEntry &distanceMappingEntry: vcEntries) {
        if (lastEntry) {
            float segmentDistance = distanceMappingEntry.distance - currentDistance;
            quint32 frameDifference = distanceMappingEntry.frame - currentFrame;

            float metersPerFrame = segmentDistance / frameDifference;

            mappings.push_back(DistanceMappingEntry(currentDistance, currentFrame, metersPerFrame));
        }
        currentDistance = distanceMappingEntry.distance;
        currentFrame = distanceMappingEntry.frame;
        lastEntry = &distanceMappingEntry;
    }

    if (lastEntry && !mappings.empty()) {
        mappings.push_back(DistanceMappingEntry(lastEntry->distance, lastEntry->frame, mappings.back().metersPerFrame()));
    }

    return mappings;
}
}
