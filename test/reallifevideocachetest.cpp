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

#include "reallifevideocachetest.h"

#include "model/distancemappingentry.h"
#include "importer/rlvfileparser.h"

namespace {
const QFileInfo BAVELLA_VIDEO_FILE = QFileInfo("/media/video/RLV/FR_Bavella.avi");
const QFileInfo BAVELLA_PGMF_FILE = QFileInfo(":///resources/FR_Bavella.pgmf");
QList<QFileInfo> videoFiles = { BAVELLA_VIDEO_FILE };
}

#include <QtTest/QTest>
RealLifeVideoCacheTest::RealLifeVideoCacheTest(QObject *parent) :
    QObject(parent)
{
}

void RealLifeVideoCacheTest::testSaveAndLoad()
{
    QFile fTacx(":///resources/FR_Bavella.rlv");
    RlvFileParser rlvFileParser({BAVELLA_PGMF_FILE}, videoFiles);
    RealLifeVideo tacxRlv = rlvFileParser.parseRlvFile(fTacx);

    _cache.save(fTacx, tacxRlv);

    std::unique_ptr<RealLifeVideo> rlvPtr = _cache.load(fTacx);
    QVERIFY(rlvPtr.get() != nullptr);
    RealLifeVideo &rlv = *rlvPtr;

    QCOMPARE(rlvPtr->name(), tacxRlv.name());
    QCOMPARE(rlv.fileType(), RealLifeVideoFileType::TACX);
    QCOMPARE(rlv.videoFilename(), QString("/media/video/RLV/FR_Bavella.avi"));
    QCOMPARE(rlv.videoFrameRate(), 25.0);

    QCOMPARE(rlv.courses().size(), tacxRlv.courses().size());
    for (auto i = 0u; i < tacxRlv.courses().size(); ++i) {
        QCOMPARE(rlv.courses()[i].start(), tacxRlv.courses()[i].start());
        QCOMPARE(rlv.courses()[i].end(), tacxRlv.courses()[i].end());
    }

    QCOMPARE(rlv.distanceMappings().size(), tacxRlv.distanceMappings().size());
    for (auto i = 0u; i < tacxRlv.distanceMappings().size(); ++i) {
        const DistanceMappingEntry &original = tacxRlv.distanceMappings()[i];
        const DistanceMappingEntry &deserialized = rlv.distanceMappings()[i];

        QCOMPARE(deserialized.distance(), original.distance());
        QCOMPARE(deserialized.frameNumber(), original.frameNumber());
        QCOMPARE(deserialized.metersPerFrame(), original.metersPerFrame());
    }
    Profile originalProfile = tacxRlv.profile();
    Profile deserializedProfile = rlv.profile();

    QCOMPARE(deserializedProfile.startAltitude(), originalProfile.startAltitude());
    QCOMPARE(deserializedProfile.type(), originalProfile.type());
    QCOMPARE(deserializedProfile.entries().size(), originalProfile.entries().size());
    for (auto i = 0u; i < originalProfile.entries().size(); ++i) {
        const ProfileEntry &original = originalProfile.entries()[i];
        const ProfileEntry &deserialized = deserializedProfile.entries()[i];

        QCOMPARE(deserialized.altitude(), original.altitude());
        QCOMPARE(deserialized.distance(), original.distance());
        QCOMPARE(deserialized.slope(), original.slope());
    }
}
