/*
 * Copyright (c) 2015 Ilja Booij (ibooij@gmail.com)
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
#ifndef REALLIFEVIDEOCACHE_H
#define REALLIFEVIDEOCACHE_H

#include <QtCore/QObject>
#include <memory>
#include "model/reallifevideo.h"

/**
 * A File Cache for RealLifeVideos. Objects of this class are used to save and load RealLifeVideos to and from a format
 * that can be loaded very quickly. We use this because loading a RealLifeVideo from a .gpx or .xml (VirtualTraining) can
 * be pretty slow. It can take several seconds, especially on slower hardware with spinning disks. This cache can be used
 * to trim that time to several milliseconds (on SSD) or tens of milliseconds on spinning disks, greatly improving
 * startup time of the application.
 *
 * This cache uses QDataStream to save and load the cache files.
 */
class RealLifeVideoCache
{
public:
    /** Load an rlv from the cache. \param rlvFile is used to determine the cache file. If there is a cache file
     * and it's younger that \param rlvFile, and it is of the current application version, the RealLifeVideo is read
     * and returned. Otherwise, an empty unique_ptr is returned.
     */
    std::unique_ptr<RealLifeVideo> load(const QFile &rlvFile);
    /**
     * Saves the rlv to a cache file.
     * @param rlvFile used to determine the name of the cache file.
     * @param rlv the rlv to save.
     */
    void save(const QFile &rlvFile, const RealLifeVideo &rlv);

private:
    QString absoluteFilenameForRlv(const QString &name) const;
    void saveCourses(QDataStream &out, const std::vector<Course> &courses) const;
    std::vector<Course> readCourses(QDataStream &in) const;

    void saveDistanceMappings(QDataStream &out, const std::vector<DistanceMappingEntry> &distanceMappings) const;
    std::vector<DistanceMappingEntry> readDistanceMappings(QDataStream &in) const;

    void saveProfile(QDataStream &out, const Profile &profile) const;
    Profile readProfile(QDataStream &in);

    void saveProfileEntries(QDataStream &out, const std::vector<ProfileEntry> &entries) const;
    std::vector<ProfileEntry> readProfileEntries(QDataStream &in) const;

    void saveInformationBoxes(QDataStream &out, const std::vector<InformationBox> &entries) const;
    std::vector<InformationBox> readInformationBoxes(QDataStream &in) const;

    void savePositions(QDataStream &out, const std::vector<GeoPosition> &positions) const;
    std::vector<GeoPosition> readPositions(QDataStream &in) const;
};

#endif // REALLIFEVIDEOCACHE_H
