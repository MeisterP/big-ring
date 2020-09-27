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

#ifndef PROFILE_H
#define PROFILE_H

#include <vector>
#include <QtCore/QObject>
#include "distanceentrycollection.h"
class ProfileEntry
{
public:
    explicit ProfileEntry(float distance, float slope, float altitude);
    explicit ProfileEntry();

    float slope() const { return _slope; }
    float distance() const { return _distance; }
    float altitude() const { return _altitude; }

    bool operator ==(const ProfileEntry& other) const;
private:
    float _distance;
    float _altitude;
    float _slope;
};

enum class ProfileType {
    POWER, SLOPE, HEART_RATE
};

class Profile
{
public:
    explicit Profile(ProfileType type, float startAltitude, const std::vector<ProfileEntry> &&entries);
    Profile(const Profile &other);
    explicit Profile();

    Profile &operator=(const Profile &other);

    ProfileType type() const { return _type; }
    //! get the slope for a particular distance
    float slopeForDistance(float distance) const;
    //! total distance of the profile
    float totalDistance() const;

    float startAltitude() const;

    const std::vector<ProfileEntry> &entries() const;

    //! get the altitude for a particular distance. The profile always starts at altitude 0.0f
    float altitudeForDistance(float distance) const;

    float minimumAltitude() const;
    float minimumAltitudeForPart(float start, float end) const;
    float maximumAltitude() const;
    float maximumAltitudeForPart(float start, float end) const;
private:
    typedef std::vector<ProfileEntry> ProfileEntryVector;
    typedef std::vector<ProfileEntry>::const_iterator ProfileEntryVectorIt;

    ProfileType _type;
    float _startAltitude;
    mutable DistanceEntryCollection<ProfileEntry> _entries;

    /** Get an iterator the the ProfileEntry we need for a specific distance */
    const ProfileEntryVectorIt entryIteratorForDistance(const float distance) const;
    const std::pair<const ProfileEntryVectorIt,
        const ProfileEntryVectorIt> rangeForDistances(float start, float end) const;
};

#endif // PROFILE_H
