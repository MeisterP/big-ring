#include "ridefilewriter.h"

#include <QtCore/QDir>
#include <QtCore/QJsonDocument>
#include <QtCore/QStandardPaths>

namespace {
const QString DATE_TIME_FORMAT_WITH_MILLIS = "yyyy-MM-ddTHH:mm:ss.zzz";
const QString DATE_TIME_FORMAT_WITHOUT_MILLIS = "yyyy-MM-ddTHH:mm:ss";
}
RideFileWriter::RideFileWriter(QObject *parent): QObject(parent)
{
    // empty
}

const QString RideFileWriter::writeRideFile(const RideFile &rideFile)
{
    const QString filePath = determineFilePath(rideFile);

    QFile outputFile(filePath);
    outputFile.open(QFile::WriteOnly);

    writeXml(rideFile, outputFile);

    return filePath;
}

QString RideFileWriter::dateTimeString(const QDateTime dateTime, bool withMillis) const
{
    const QString &format = (withMillis) ? DATE_TIME_FORMAT_WITH_MILLIS : DATE_TIME_FORMAT_WITHOUT_MILLIS;

    return dateTime.toString(format) + "Z";
}

QString RideFileWriter::determineFilePath(const RideFile &rideFile) const
{
    QDir appDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    const QString directoryName = QString("Big-Ring/Rides");
    const bool ok = appDir.mkpath(directoryName);
    if (!ok) {
        qFatal("unable to create dir");
    }

    appDir.cd(directoryName);
    const QString filename = QString("%1.tcx").arg(rideFile.startTime().toString("yyyy_MM_dd_HH_mm_ss"));

    return appDir.filePath(filename);
}

void RideFileWriter::writeXml(const RideFile &rideFile, QFile &outputFile)
{
    QXmlStreamWriter writer(&outputFile);

    writer.setAutoFormatting(true);
    writer.writeStartDocument();

    writer.writeStartElement("TrainingCenterDatabase");
    writer.writeStartElement("Activities");
    writer.writeStartElement("Activity");
    writer.writeAttribute("Sport", "Biking");

    writer.writeTextElement("Id", dateTimeString(rideFile.startTime()));
    writer.writeStartElement("Lap");

    writeLapSummary(writer, rideFile);

    writer.writeStartElement("Track");
    for (const RideFile::Sample &sample: rideFile.samples()) {
        writeTrackPoint(writer, rideFile.startTime(), sample);
    }
    writer.writeEndElement(); // Track
    writer.writeEndElement(); // Lap
    writer.writeEndElement(); // Activity
    writer.writeEndElement(); // Activities
    writer.writeEndElement(); // TrainingCenterDatabase

    writer.writeEndDocument();
}

/**
 * Write the summary for the lap. As we put everything in one lap, this will just use the values from the ride file.
 */
void RideFileWriter::writeLapSummary(QXmlStreamWriter &writer, const RideFile &rideFile)
{
    writer.writeAttribute("StartTime", dateTimeString(rideFile.startTime()));
    const int msecs = rideFile.durationInMilliSeconds();

    writer.writeTextElement("TotalTimeSeconds", QString("%1.%2").arg(msecs / 1000).arg(msecs / 10));
    writer.writeTextElement("DistanceMeters", QString::number(rideFile.totalDistance()));

    writer.writeTextElement("MaximumSpeed", QString::number(rideFile.maximumSpeedInMps()));

    writer.writeStartElement("MaximumHeartRateBpm");
    writer.writeTextElement("Value", QString::number(rideFile.maximumHeartRate()));
    writer.writeEndElement();
}

/**
 * Write a single track point.
 */
void RideFileWriter::writeTrackPoint(QXmlStreamWriter &writer, const QDateTime &startTime, const RideFile::Sample &sample)
{
    writer.writeStartElement("Trackpoint");

    // simple elements
    writer.writeTextElement("Time", dateTimeString(startTime.addMSecs(sample.time.msecsSinceStartOfDay()), true));
    writer.writeTextElement("AltitudeMeters", QString::number(sample.altitude));
    writer.writeTextElement("DistanceMeters", QString::number(sample.distance));
    writer.writeTextElement("Cadence", QString::number(sample.cadence));

    // heart rate is put in a Value element inside a HeartRateBpm element.
    writer.writeStartElement("HeartRateBpm");
    writer.writeAttribute("xsi:type", "HeartRateInBeatsPerMinute_t");
    writer.writeTextElement("Value", QString::number(sample.heartRate));
    writer.writeEndElement(); // HeartRateBpm

    // power and speed are put inside a TPX element, which is inside an Extensions element. All to make it more
    // verbose I guess.
    writer.writeStartElement("Extensions");
    writer.writeStartElement("TPX");
    writer.writeAttribute("xmlns", "http://www.garmin.com/xmlschemas/ActivityExtension/v2");
    writer.writeTextElement("Speed", QString::number(sample.speed));
    writer.writeTextElement("Watts", QString::number(sample.power));
    writer.writeEndElement(); // TPX
    writer.writeEndElement(); // Extensions

    writer.writeEndElement(); // Track point
}
