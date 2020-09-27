#include "virtualtrainingfileparsertest.h"

#include "importer/rlvfileparser.h"
#include "importer/virtualtrainingfileparser.h"
#include <QtCore/QFile>
#include <QtTest/QTest>

namespace {
const QFileInfo BAVELLA_VIDEO_FILE = QFileInfo("/media/video/RLV/FR_Bavella.avi");
const QFileInfo DOBEL_VIDEO_FILE = QFileInfo("/media/video/RLV/DE_Dobel.avi");
const QFileInfo BAVELLA_PGMF_FILE = QFileInfo(":///resources/FR_Bavella.pgmf");
QList<QFileInfo> videoFiles = { DOBEL_VIDEO_FILE, BAVELLA_VIDEO_FILE };
}
VirtualTrainingFileParserTest::VirtualTrainingFileParserTest(QObject *parent):
    QObject(parent)
{
    // empty
}

void VirtualTrainingFileParserTest::testWithBavellaFile()
{
    QFile f(":///resources/FR_Bavella-utf8.xml");
    QFile fTacx(":///resources/FR_Bavella.rlv");

    RlvFileParser rlvFileParser({BAVELLA_PGMF_FILE}, videoFiles);

    indoorcycling::VirtualTrainingFileParser parser(videoFiles);
    RealLifeVideo rlv = parser.parseVirtualTrainingFile(f);
    RealLifeVideo tacxRlv = rlvFileParser.parseRlvFile(fTacx);

    QVERIFY(rlv.isValid());
    QCOMPARE(rlv.name(), QString("FR_Bavella"));
    QCOMPARE(rlv.videoFilename(), QString("/media/video/RLV/FR_Bavella.avi"));
    QCOMPARE(rlv.videoFrameRate(), 25.0f);
    QCOMPARE(tacxRlv.videoFrameRate(), 25.0f);

    Profile profile = rlv.profile();
    float startAltitude = 12.5033845f;
    QCOMPARE(profile.altitudeForDistance(0), startAltitude);
    QCOMPARE(tacxRlv.profile().altitudeForDistance(0), 0.0f);

    QCOMPARE(profile.altitudeForDistance(38440), 750.67939f);
    QCOMPARE(qRound(profile.slopeForDistance(38432) * 100), -10);
    QCOMPARE(qRound(tacxRlv.profile().slopeForDistance(38432) * 100), -10);

    std::vector<Course> courses = rlv.courses();
    unsigned size = courses.size();
    QCOMPARE(size, 3u);
    QCOMPARE(courses[1].name(), QString("Erstes Teilstück"));
    QCOMPARE(courses[1].start(), 0.0f);
    QCOMPARE(courses[1].end(), 20280.0f);

    QCOMPARE(rlv.frameForDistance(0.0f), 0u);
    QCOMPARE(tacxRlv.frameForDistance(0.0f), 0u);
    QCOMPARE(rlv.frameForDistance(332.8), 1000u);
    QCOMPARE(tacxRlv.frameForDistance(332.8), 1000u);
}

void VirtualTrainingFileParserTest::testCoordinates()
{
    QFile f(":///resources/FR_Bavella-utf8.xml");
    indoorcycling::VirtualTrainingFileParser parser(videoFiles);
    RealLifeVideo rlv = parser.parseVirtualTrainingFile(f);

    GeoPosition position = rlv.positionForDistance(0);
    QVERIFY2(position.isValid(), "Position should be set");
    QVERIFY(position.coordinate().latitude() > 41.862);
    QVERIFY(position.coordinate().latitude() < 41.863);
    position = rlv.positionForDistance(5555);
    QVERIFY2(position.isValid(), "Position should be set");
    QVERIFY(position.coordinate().latitude() > 41.847);
    QVERIFY(position.coordinate().latitude() < 41.848);
    QVERIFY(position.coordinate().longitude() > 9.352);
    QVERIFY(position.coordinate().longitude() < 9.353);

    position = rlv.positionForDistance(5556);
    QVERIFY2(position.isValid(), "Position should be set");
    QVERIFY(position.coordinate().latitude() > 41.847);
    QVERIFY(position.coordinate().latitude() < 41.848);
    QVERIFY(position.coordinate().longitude() > 9.352);
    QVERIFY(position.coordinate().longitude() < 9.353);

    const QGeoRectangle geoRectangle = rlv.geoRectangle();
    qDebug() << "geo rectangle" << geoRectangle.topLeft() << geoRectangle.bottomRight();
}
