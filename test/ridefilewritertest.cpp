#include "ridefilewritertest.h"

#include <QtCore/QDateTime>

#include "model/ridefile.h"
#include "ride/ridefilewriter.h"

RideFileWriterTest::RideFileWriterTest(QObject *parent) :
    QObject(parent)
{
    // empty
}

void RideFileWriterTest::testWriteSimpleRideFile()
{
    RideFile rideFile("rlv", "course");
    RideFileWriter writer;

    RideFile::Sample sample;
    sample.altitude = 1.0;
    sample.cadence = 90;
    sample.distance = 0.0;
    sample.heartRate = 150;
    sample.power = 300;
    sample.speed = 8.6;
    sample.time = QTime::fromMSecsSinceStartOfDay(250);

    rideFile.addSample(sample);

    sample.altitude = 2.0;
    sample.cadence = 95;
    sample.distance = 10.0;
    sample.heartRate = 160;
    sample.power = 310;
    sample.speed = 9.0;
    sample.time = QTime::fromMSecsSinceStartOfDay(2250);

    rideFile.addSample(sample);
    writer.writeRideFile(rideFile);
}
