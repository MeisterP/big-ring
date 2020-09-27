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

#ifndef REALLIVEVIDEO_H
#define REALLIVEVIDEO_H

#include <QtCore/QFileInfo>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QPair>
#include <QtCore/QSharedPointer>
#include <QtCore/QString>

#include <QtPositioning/QGeoRectangle>

#include "geoposition.h"
#include "profile.h"

/** The different types of RealLifeVideo that are supported */
enum class RealLifeVideoFileType {
    TACX, // Tacx Fortius Software (.rlv and .pgmf files)
    VIRTUAL_TRAINING, // Cycleops Virtual Training .xml files
    GPX // GPS tracks in GPX format.
};

/** Return the name of a RealLifeVideoFileType as a string */
const QString &realLifeVideoFileTypeName(RealLifeVideoFileType type);

/* forward declarations */
class DistanceMappingEntry;
class VideoInformation;

class Course
{
public:
    enum class Type { Invalid, Normal, Custom };
    explicit Course(const QString& name, const Type type, float start, float end);
    explicit Course(const QString& name, float start, float end);
    explicit Course(float start, float end);
    explicit Course();

    bool isValid() const { return !(_type == Type::Invalid);}
    const QString& name() const { return _name; }
    Type type() const { return _type; }
    /*! start distance of course in  meters */
    float start() const { return _start; }
    /*! end distance of course in meters */
    float end() const { return _end; }
    /*! distance of course in meters */
    float distance() const { return _end - _start; }

private:
    QString _name;
    Type _type;
    float _start;
    float _end;
};

class InformationBox
{
public:
    InformationBox(const quint32 frameNumber, const float distance, const QString &message, const QFileInfo &imageFileInfo):
        _frameNumber(frameNumber), _distance(distance), _message(message), _imageFileInfo(imageFileInfo) {}

    InformationBox() {}

    quint32 frameNumber() const { return _frameNumber; }
    float distance() const { return _distance; }
    const QString &message() const { return _message; }
    const QFileInfo &imageFileInfo() const { return _imageFileInfo; }
private:
    quint32 _frameNumber = 0u;
    float _distance = 0;
    QString _message = QString("");
    QFileInfo _imageFileInfo;
};

bool operator==(const InformationBox& lhs, const InformationBox& rhs);


class RealLifeVideoData;

class RealLifeVideo
{
public:
    explicit RealLifeVideo(const QString& name, RealLifeVideoFileType fileType, const VideoInformation& videoInformation, const std::vector<Course> &&courses,
                           const std::vector<DistanceMappingEntry>&& distanceMappings, const Profile &profile,
                           const std::vector<InformationBox> &&informationBoxes = std::vector<InformationBox>(),
                           const std::vector<GeoPosition> &&geoPositions =
            std::vector<GeoPosition>());
    RealLifeVideo(const RealLifeVideo& other);
    explicit RealLifeVideo();

    bool isValid() const;
    RealLifeVideoFileType fileType() const;
    ProfileType type() const;
    const Profile &profile() const;
    const QString name() const;
    const QString &videoFilename() const;
    float videoFrameRate() const;
    const std::vector<Course>& courses() const;
    const std::vector<DistanceMappingEntry> &distanceMappings() const;
    const std::vector<InformationBox> &informationBoxes() const;
    const std::vector<GeoPosition> &positions() const;

    const QGeoRectangle geoRectangle() const;

    /** Add a new custom start point */
    void addStartPoint(float distance, const QString& name);
    /** Add a new custom course */
    void addCustomCourse(float startDistance, float endDistance, const QString& name);

    /** Get the number or frames per meter for a certain distance */
    float metersPerFrame(const float distance) const;
    /** Get the exact frame for a distance. */
    quint32 frameForDistance(const float distance) const;
    /** Get the slope for a distance */
    float slopeForDistance(const float distance) const;
    //! Get the altitude for a distance */
    float altitudeForDistance(const float distance) const;
    /*! Get the geo coordinate for a distance. If no position could be found,
     * this returns GeoPosition::NULL_POSITION. */
    const GeoPosition positionForDistance(const float distance) const;
    //! Get the information box message for a distance */
    const InformationBox informationBoxForDistance(const float distance) const;
    /** Total distance */
    float totalDistance() const;
    /** Set duration of video, in number of frames */
    void setNumberOfFrames(quint64 numberOfFrames);

    bool operator==(const RealLifeVideo& other) const;
    static bool compareByName(const RealLifeVideo& rlv1, const RealLifeVideo& rlv2);
private:
    void calculateVideoCorrectionFactor(quint64 totalNrOfFrames);

    const DistanceMappingEntry &findDistanceMappingEntryFor(const float distance) const;
    const InformationBox informationBoxForDistanceTacx(const float distance) const;

    QSharedPointer<RealLifeVideoData> _d;
};
typedef QList<RealLifeVideo> RealLifeVideoList;

Q_DECLARE_METATYPE(RealLifeVideo)
#endif // REALLIVEVIDEO_H
