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

#include "reallifevideo.h"

#include "distanceentrycollection.h"
#include "distancemappingentry.h"
#include "videoinformation.h"

#include <QtDebug>
#include <QMapIterator>

namespace
{
std::map<RealLifeVideoFileType,const QString> REAL_LIFE_VIDEO_TYPE_NAMES =
{{RealLifeVideoFileType::GPX, "GPX"},
 {RealLifeVideoFileType::TACX, "Tacx"},
 {RealLifeVideoFileType::VIRTUAL_TRAINING, "Cycleops Virtual Training"}};
const QString REAL_LIFE_VIDEO_TYPE_NAMES_UNKNOWN_TYPE;
}

const QString &realLifeVideoFileTypeName(RealLifeVideoFileType type) {
    auto it = REAL_LIFE_VIDEO_TYPE_NAMES.find(type);
    if (it == REAL_LIFE_VIDEO_TYPE_NAMES.end()) {
        return REAL_LIFE_VIDEO_TYPE_NAMES_UNKNOWN_TYPE;
    }
    return (*it).second;
}

class RealLifeVideoData
{
public:
    RealLifeVideoData()  {}

    ~RealLifeVideoData() {}

    QString _name;
    RealLifeVideoFileType _fileType;
    Profile _profile;
    DistanceEntryCollection<DistanceMappingEntry> _distanceMappings;
    DistanceEntryCollection<GeoPosition> _geoPositions;
    VideoInformation _videoInformation;
    std::vector<Course> _courses;
    std::vector<InformationBox> _informationBoxes;
    float _videoCorrectionFactor;
};

Course::Course(const QString &name, const Type type, float start, float end):
    _name(name), _type(type), _start(start), _end(end)
{}

Course::Course(const QString &name, float start, float end):
    Course(name, Type::Normal, start, end)
{
    // empty
}

Course::Course(): Course("", Type::Invalid, 0, 0)
{
    // empty
}

RealLifeVideo::RealLifeVideo(const QString &name, RealLifeVideoFileType fileType, const VideoInformation &videoInformation,
                             const std::vector<Course> &&courses,
                             const std::vector<DistanceMappingEntry> &&distanceMappings,
                             const Profile &profile, const std::vector<InformationBox> &&informationBoxes, const std::vector<GeoPosition> &&geoPositions):
    _d(new RealLifeVideoData)
{
    _d->_name = name;
    _d->_fileType = fileType;
    _d->_profile = profile;
    _d->_videoInformation = videoInformation;
    _d->_courses = courses;
    _d->_videoCorrectionFactor = 1.0;
    _d->_distanceMappings = DistanceEntryCollection<DistanceMappingEntry>(distanceMappings,
        [](const DistanceMappingEntry &entry) {
        return entry.distance();
    });
    _d->_informationBoxes = informationBoxes;
    _d->_geoPositions =
            DistanceEntryCollection<GeoPosition>(geoPositions,
                                                 [](const GeoPosition &position) {
        return position.distance();
    });
}

RealLifeVideo::RealLifeVideo(const RealLifeVideo &other):
    _d(other._d)
{

}

RealLifeVideo::RealLifeVideo():
    _d(new RealLifeVideoData)
{

}

bool RealLifeVideo::isValid() const
{
    return (!_d->_name.isEmpty() && !_d->_videoInformation.videoFilename().isEmpty());
}

RealLifeVideoFileType RealLifeVideo::fileType() const
{
    return _d->_fileType;
}

ProfileType RealLifeVideo::type() const
{
    return _d->_profile.type();
}

const Profile &RealLifeVideo::profile() const
{
    return _d->_profile;
}

const QString RealLifeVideo::name() const
{
    return _d->_name;
}

const QString &RealLifeVideo::videoFilename() const
{
    return _d->_videoInformation.videoFilename();
}

float RealLifeVideo::videoFrameRate() const
{
    return _d->_videoInformation.frameRate();
}

const std::vector<Course> &RealLifeVideo::courses() const
{
    return _d->_courses;
}

const std::vector<DistanceMappingEntry> &RealLifeVideo::distanceMappings() const
{
    return _d->_distanceMappings.entries();
}

const std::vector<InformationBox> &RealLifeVideo::informationBoxes() const
{
    return _d->_informationBoxes;
}

const std::vector<GeoPosition> &RealLifeVideo::positions() const
{
    return _d->_geoPositions.entries();
}

const QGeoRectangle RealLifeVideo::geoRectangle() const
{
    QList<QGeoCoordinate> coordinates;
    coordinates.reserve(_d->_geoPositions.entries().size());
    for (const GeoPosition &position: _d->_geoPositions.entries()) {
        coordinates.append(position.coordinate());
    }
    return QGeoRectangle(coordinates);
}

void RealLifeVideo::addStartPoint(float distance, const QString &name)
{
    addCustomCourse(distance, totalDistance(), name);
}

void RealLifeVideo::addCustomCourse(float startDistance, float endDistance, const QString &name)
{
    Course customCourse(name, Course::Type::Custom, startDistance, endDistance);
    _d->_courses.push_back(customCourse);
}

float RealLifeVideo::metersPerFrame(const float distance) const
{
    const DistanceMappingEntry& entry = findDistanceMappingEntryFor(distance);
    return entry.metersPerFrame();
}

quint32 RealLifeVideo::frameForDistance(const float distance) const
{
    float correctedDistance = distance * _d->_videoCorrectionFactor;
    const DistanceMappingEntry& entry = findDistanceMappingEntryFor(correctedDistance);
    return entry.frameNumber() + (correctedDistance - entry.distance()) / entry.metersPerFrame();
}

float RealLifeVideo::slopeForDistance(const float distance) const
{
    return _d->_profile.slopeForDistance(distance);
}

float RealLifeVideo::altitudeForDistance(const float distance) const
{
    return _d->_profile.altitudeForDistance(distance);
}

/**
 * @brief RealLifeVideo::positionForDistance get the gps position for a distance.
 *
 * If the distance is greater than the last position entry for the rlv, the last position will be returned.
 *
 * @param distance distance to get position for.
 * @return the position for the distance, or GeoPosition::NULL_POSITION if no position could be found.
 */
const GeoPosition RealLifeVideo::positionForDistance(const float distance) const
{
    const auto entry = _d->_geoPositions.iteratorForDistance(distance);
    if (_d->_geoPositions.isEndEntryIterator(entry)) {
        // no position, just return NULL_POSITION.
        return GeoPosition::NULL_POSITION;
    }
    const auto nextEntry = entry + 1;
    if (_d->_geoPositions.isEndEntryIterator(nextEntry)) {
        // last entry, just return it.
        return *entry;
    }
    return GeoPosition::interpolateBetween(*entry, *nextEntry, distance);
}

const InformationBox RealLifeVideo::informationBoxForDistance(const float distance) const
{
    if (fileType() == RealLifeVideoFileType::TACX) {
        return informationBoxForDistanceTacx(distance);
    }

    // default implementation.
    InformationBox informationBoxForDistance;
    for (const InformationBox &informationBox: _d->_informationBoxes) {
        if (informationBox.distance() > distance) {
            break;
        }
        informationBoxForDistance = informationBox;
    }
    return informationBoxForDistance;
}

float RealLifeVideo::totalDistance() const
{
    return _d->_profile.totalDistance();
}


void RealLifeVideo::setNumberOfFrames(quint64 numberOfFrames)
{
    if (_d->_fileType == RealLifeVideoFileType::TACX) {
        calculateVideoCorrectionFactor(numberOfFrames);
        qDebug() << "correction factor" << _d->_videoCorrectionFactor;
    } else {
        qDebug() << "no correction factor needed.";
    }
}

bool RealLifeVideo::operator==(const RealLifeVideo &other) const
{
    return isValid() && other.isValid() && _d->_name == other._d->_name
            && _d->_fileType == other._d->_fileType;
}

bool RealLifeVideo::compareByName(const RealLifeVideo &rlv1, const RealLifeVideo &rlv2)
{
    return rlv1.name().toLower() < rlv2.name().toLower();
}

/*!
 * \brief Determine the ratio between the length of the video and the
 * length of the profile. Store this ratio in the field _videoCorrectionFactor.
 * This ratio is used whenever we need to determine the exact frame for a difference.
 *
 * There are still some videos where this does not work correctly, like IT_GiroMortirolo,
 * but most videos seem to work ok.
 *
 * \param totalNrOfFrames the total number of frames in the video.
 */
void RealLifeVideo::calculateVideoCorrectionFactor(quint64 totalNrOfFrames)
{
    // some rlvs, mostly the old ones like the old MajorcaTour have only two
    // entries in the distancemappings list. For these rlvs, it seems to work
    // better to just use a _videoCorrectionFactor of 1.0.
    if (_d->_distanceMappings.entries().size() < 3) {
        _d->_videoCorrectionFactor = 1;
    } else {
        const auto &lastDistanceMapping = _d->_distanceMappings.entries().back();
        quint64 framesInLastEntry;
        if (totalNrOfFrames > lastDistanceMapping.frameNumber()) {
            framesInLastEntry = totalNrOfFrames - lastDistanceMapping.frameNumber();
        } else {
            framesInLastEntry = 0u;
        }
        float videoDistance = lastDistanceMapping.distance() + framesInLastEntry * lastDistanceMapping.metersPerFrame();
        _d->_videoCorrectionFactor = videoDistance / _d->_profile.totalDistance();
    }
}

const DistanceMappingEntry &RealLifeVideo::findDistanceMappingEntryFor(const float distance) const
{
    return *(_d->_distanceMappings.iteratorForDistance(distance));
}

const InformationBox RealLifeVideo::informationBoxForDistanceTacx(const float distance) const
{
    quint32 frame = frameForDistance(distance);
    InformationBox informationBoxForDistance;
    for (const InformationBox &informationBox: _d->_informationBoxes) {
        if (informationBox.frameNumber() > frame) {
            break;
        }
        informationBoxForDistance = informationBox;
    }
    return informationBoxForDistance;
}

bool operator==(const InformationBox &lhs, const InformationBox &rhs)
{
    return lhs.frameNumber() == rhs.frameNumber() && qFuzzyCompare(lhs.distance(), rhs.distance());
}
