#include "profiletest.h"

#include "importer/rlvfileparser.h"
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtTest/QTest>

namespace {
const QFileInfo BAVELLA_VIDEO_FILE = QFileInfo("/media/video/RLV/FR_Bavella.avi");
const QFileInfo BAVELLA_PGMF_FILE = QFileInfo(":///resources/FR_Bavella.pgmf");
QList<QFileInfo> VIDEO_FILES = { BAVELLA_VIDEO_FILE };
}
ProfileTest::ProfileTest(QObject *parent) :
    QObject(parent)
{
}

void ProfileTest::testMaximumAltitude()
{
    QFile fTacx(":///resources/FR_Bavella.rlv");
    RlvFileParser rlvFileParser({BAVELLA_PGMF_FILE}, VIDEO_FILES);
    RealLifeVideo rlv = rlvFileParser.parseRlvFile(fTacx);

    Profile profile = rlv.profile();
    QCOMPARE(profile.minimumAltitude(), 0.0f);
    QCOMPARE(profile.minimumAltitudeForPart(0.0f, rlv.totalDistance()), 0.0f);

    QCOMPARE(profile.maximumAltitude(), 1158.02f);
    QCOMPARE(profile.maximumAltitudeForPart(0.0f, rlv.totalDistance()), 1158.02f);
    QCOMPARE(profile.maximumAltitudeForPart(0.0f, 100.0f), 3.48141f);
    QCOMPARE(profile.maximumAltitudeForPart(rlv.totalDistance() - 10, rlv.totalDistance()), 739.192f);
}
