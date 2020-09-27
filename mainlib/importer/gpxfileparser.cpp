#include "gpxfileparser.h"

#include "model/distancemappingentry.h"
#include "model/videoinformation.h"
#include "video/videoinforeader.h"

#include <deque>
#include <functional>

#include <QtCore/QDateTime>
#include <QtCore/QtDebug>
#include <QtCore/QTextStream>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoPositionInfo>

namespace {

const int NUMBER_OF_ITEMS_FOR_MOVING_AVERAGE = 5;
const float MINIMUM_SLOPE = -15.0;
const float MAXIMUM_SLOPE = 20.0;

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

/**
 * @brief calculate the frame number for a trackpoint.
 * @param trackPoint the track point
 * @param startTime start time of the track
 * @param frameRate the frame rate.
 * @return a frame number.
 */
quint32 frameNumberForTrackPoint(const QGeoPositionInfo &trackPoint, const QDateTime &startTime, const float frameRate)
{
    const qint64 differenceFromStart = startTime.msecsTo(trackPoint.timestamp());
    return static_cast<quint32>((differenceFromStart * frameRate) / 1000);
}

/**
 * Given a begin, current and end iterator to a collection, and value function for calculating the value for a entry
 * in the collection, calculate the moving average for the "current" value.
 *
 * @param begin start iterator
 * @param current iterator to entry around which running average should be calculated.
 * @param end end iterator
 * @param valueFunction function to calculate value from entry.
 */
template <typename T>
double runningAverageValue(const typename std::vector<T>::const_iterator &begin,
                           const typename std::vector<T>::const_iterator &current,
                           const typename std::vector<T>::const_iterator &end,
                           const std::function<double(const T&)> &valueFunction) {

    // determine beginning and end iterator for the moving average.
    const auto averageEntriesBegin = std::max(begin, current - NUMBER_OF_ITEMS_FOR_MOVING_AVERAGE / 2);
    const auto averageEntriesEnd = std::min(end, current + NUMBER_OF_ITEMS_FOR_MOVING_AVERAGE / 2 + 1);

    // calculate the average.
    return std::accumulate(averageEntriesBegin, averageEntriesEnd, 0.0, [&valueFunction](const float sum, const T& entry) {
        return sum + valueFunction(entry);
    }) / (averageEntriesEnd - averageEntriesBegin);
}
}

namespace indoorcycling {

GpxFileParser::GpxFileParser(const QList<QFileInfo> &videoFiles, QObject *parent) :
    QObject(parent), _videoFiles(videoFiles)
{
}

RealLifeVideo GpxFileParser::parseGpxFile(QFile &inputFile) const
{
    const QFileInfo* videoFile = videoFileForGpsFile(inputFile);
    if (videoFile == nullptr) {
        return RealLifeVideo();
    }
    inputFile.open(QIODevice::ReadOnly);
    const QString xml = QTextStream(&inputFile).readAll();
    QXmlStreamReader reader(xml);
    return parseXml(inputFile, *videoFile, reader);
}

const QFileInfo* GpxFileParser::videoFileForGpsFile(const QFile &inputFile) const
{
    const QString baseName = QFileInfo(inputFile).baseName();
    for (const QFileInfo &videoFileInfo: _videoFiles) {
        const QString videoBaseName = videoFileInfo.baseName();
        if (baseName == videoBaseName) {
            return &videoFileInfo;
        }
    }
    return nullptr;
}

RealLifeVideo GpxFileParser::parseXml(const QFile &inputFile,
                                      const QFileInfo &videoFileInfo,
                                      QXmlStreamReader &reader) const
{
    QString name = QFileInfo(inputFile).baseName();
    float frameRate = VideoInfoReader().videoInfoForVideo(videoFileInfo).frameRate;

    std::vector<QGeoPositionInfo> trackPoints;
    QXmlStreamReader::TokenType currentTokenType;
    while (!reader.atEnd()) {
        currentTokenType = reader.readNext();
        if (isElement(currentTokenType, reader, "trkpt")) {
            trackPoints.push_back(readTrackPoint(reader));
        }
    }

    const std::vector<GeoPosition> geoPositions = convertTrackPoints(trackPoints);
    const std::vector<GeoPosition> smoothedAltitudeGeoPositions = smoothAltitudes(geoPositions);

    const VideoInformation videoInformation(videoFileInfo.filePath(), frameRate);
    const Profile profile(ProfileType::SLOPE, 0.0f, smoothSlopes(convertProfileEntries(smoothedAltitudeGeoPositions)));
    std::vector<Course> courses = { Course("Complete Distance", 0, profile.totalDistance()) };
    std::vector<DistanceMappingEntry> distanceMappings = convertDistanceMappings(frameRate, trackPoints);

    return RealLifeVideo(name, RealLifeVideoFileType::GPX, videoInformation, std::move(courses),
                      std::move(distanceMappings), profile, std::move(std::vector<InformationBox>()),
                      std::move(smoothedAltitudeGeoPositions));
}

QGeoPositionInfo GpxFileParser::readTrackPoint(QXmlStreamReader &reader) const
{
    double latitude = readSingleAttribute(reader.attributes(), "lat").toDouble();
    double longitude = readSingleAttribute(reader.attributes(), "lon").toDouble();
    double altitude = 0.0;
    double speed = 0.0;
    QDateTime dateTime;

    while(!reader.atEnd()) {
        QXmlStreamReader::TokenType currentTokenType = reader.readNext();
        if (isElement(currentTokenType, reader, "ele")) {
            altitude = reader.readElementText().toDouble();
        } else if (isElement(currentTokenType, reader, "time")) {
            dateTime = QDateTime::fromString(reader.readElementText(), Qt::ISODate);
        } else if (isElement(currentTokenType, reader, "speed")) {
            speed = reader.readElementText().toDouble();
        } else if (currentTokenType == QXmlStreamReader::EndElement && reader.name() == "trkpt") {
            break;
        }
    }

    QGeoPositionInfo geoPositionInfo(QGeoCoordinate(latitude, longitude, altitude), dateTime);
    if (speed > 0.0) {
        geoPositionInfo.setAttribute(QGeoPositionInfo::GroundSpeed, speed);
    }
    return geoPositionInfo;
}

GpxFileParser::GeoPositionVector GpxFileParser::convertTrackPoints(const std::vector<QGeoPositionInfo> &positions) const
{
    qreal currentDistance = 0;
    GeoPositionVector geoPositions;
    const QGeoPositionInfo *lastEntry = nullptr;
    geoPositions.reserve(positions.size());
    for (const QGeoPositionInfo& position: positions) {
        if (lastEntry == nullptr) {
            geoPositions.push_back(GeoPosition(currentDistance, position.coordinate()));
            lastEntry = &position;
        } else {
            const qreal segmentDistance = distanceBetweenPoints(*lastEntry, position);
            currentDistance += segmentDistance;
            if (segmentDistance < 0.1) {
                qDebug() << "segment distance very small, pruning position.";
            } else {
                geoPositions.push_back(GeoPosition(currentDistance, position.coordinate()));
                lastEntry = &position;
            }
        }

    }
    return geoPositions;
}

std::vector<ProfileEntry> GpxFileParser::convertProfileEntries(const GeoPositionVector &trackPoints) const
{
    std::vector<ProfileEntry> profileEntries;

    const GeoPosition *lastEntry = nullptr;
    qreal currentDistance = 0;
    qreal currentElevation = 0;
    for (const GeoPosition &trackPoint: trackPoints) {
        qreal segmentDistance = 0;
        if (lastEntry) {
            segmentDistance = trackPoint.distance() - lastEntry->distance();
            qreal segmentAltitudeDifference = trackPoint.altitude() - currentElevation;

            if (segmentDistance > 0) {
                float slope = segmentAltitudeDifference / segmentDistance;
                float slopeInPercent = slope * 100.0f;

                profileEntries.push_back(ProfileEntry(currentDistance, slopeInPercent, currentElevation));
            }
        }
        currentDistance += segmentDistance;
        currentElevation = trackPoint.altitude();
        lastEntry = &trackPoint;
    }

    // if there was an entry, and there's at least one entry in profile entries, add a last entry
    // to profile entries with the same slope as the previous one
    if (lastEntry && !profileEntries.empty()) {
        profileEntries.push_back(ProfileEntry(currentDistance, profileEntries.back().slope(), lastEntry->altitude()));
    }

    return profileEntries;
}

GpxFileParser::GeoPositionVector GpxFileParser::smoothAltitudes(const GpxFileParser::GeoPositionVector &positions) const
{
    GeoPositionVector smoothedAltitudes;
    smoothedAltitudes.reserve(positions.size());

    // this function will be passed to the running average function to get the altitude for each position.
    std::function<double(const GeoPosition&)> altitudeValueFunction([](const GeoPosition &position) {
        return position.altitude();
    });

    for (auto it = positions.cbegin(); it != positions.end(); ++it) {
        const double averageAltitude = runningAverageValue(positions.cbegin(), it, positions.cend(), altitudeValueFunction);

        smoothedAltitudes.push_back(it->withAltitude(averageAltitude));
    }

    return smoothedAltitudes;
}


/**
 * Smooth the profile by taking a running average for the slope of each entry.
 */
std::vector<ProfileEntry> GpxFileParser::smoothSlopes(const std::vector<ProfileEntry> &profile) const
{
    std::vector<ProfileEntry> smoothedProfile;
    smoothedProfile.reserve(profile.size());

    std::function<double(const ProfileEntry&)> slopeValueFunction([](const ProfileEntry &entry) {
        return qBound(MINIMUM_SLOPE, entry.slope(), MAXIMUM_SLOPE);
    });

    for (auto it = profile.cbegin(); it != profile.end(); ++it) {
        const double averageSlope = runningAverageValue(profile.cbegin(), it, profile.cend(), slopeValueFunction);

        const ProfileEntry& lastEntry = (smoothedProfile.empty()) ? *it : smoothedProfile.back();

        // determine altitude from distance and new slope. For the first entry, distanceDifference will be zero.
        const float distanceDifference = it->distance() - lastEntry.distance();
        const float newAltitude = lastEntry.altitude() + lastEntry.slope() * 0.01 * distanceDifference;

        smoothedProfile.push_back(ProfileEntry(it->distance(), averageSlope, newAltitude));
    }
    return smoothedProfile;
}

qreal GpxFileParser::distanceBetweenPoints(const QGeoPositionInfo &start, const QGeoPositionInfo &end) const
{
    if (start.hasAttribute(QGeoPositionInfo::GroundSpeed)) {
        const int durationMsecs = start.timestamp().msecsTo(end.timestamp());
        const qreal speedMps = start.attribute(QGeoPositionInfo::GroundSpeed);
        return durationMsecs * 0.001 * speedMps;
    } else {
        return start.coordinate().distanceTo(end.coordinate());
    }
}

/**
 * @brief Convert positions to DistanceMappingEntries.
 * @param frameRate the frame rate of the video.
 * @param trackPoints all track points as positions.
 * @return a vector of DistanceMappingEntry objects which contain the mapping from distance to frames in the video.
 */
std::vector<DistanceMappingEntry> GpxFileParser::convertDistanceMappings(
        float frameRate,
        const std::vector<QGeoPositionInfo> &trackPoints) const
{
    Q_ASSERT_X(!trackPoints.empty(), "convertDistanceMappings", "trackpoints should not be empty");

    std::vector<DistanceMappingEntry> mappings;

    const QDateTime startTime = trackPoints[0].timestamp();

    const QGeoPositionInfo *lastTrackPoint = nullptr;
    float currentDistance = 0;
    quint32 currentFrame = 0;
    for (const QGeoPositionInfo &trackPoint: trackPoints) {
        qreal segmentDistance = 0;
        const quint32 frameNumberForPoint = frameNumberForTrackPoint(trackPoint, startTime, frameRate);

        if (lastTrackPoint) {
            segmentDistance = distanceBetweenPoints(*lastTrackPoint, trackPoint);
            quint32 frameDifference = frameNumberForPoint - currentFrame;
            if (frameDifference > 0) {
                float metersPerFrame = segmentDistance / frameDifference;
                mappings.push_back(DistanceMappingEntry(currentDistance, currentFrame, metersPerFrame));
            }
        }
        currentDistance += segmentDistance;
        currentFrame = frameNumberForPoint;
        lastTrackPoint = &trackPoint;
    }

    if (lastTrackPoint && !mappings.empty()) {
        quint32 frameNumber = frameNumberForTrackPoint(*lastTrackPoint, startTime, frameRate);
        mappings.push_back(DistanceMappingEntry(currentDistance, frameNumber, mappings.back().metersPerFrame()));
    }

    return mappings;
}

quint32 frameNumberForTrackPoint(const QGeoPositionInfo &trackPoint, const QDateTime &startTime, const float frameRate)
{
    const qint64 differenceFromStart = startTime.msecsTo(trackPoint.timestamp());
    return static_cast<quint32>((differenceFromStart * frameRate) / 1000);
}
}
