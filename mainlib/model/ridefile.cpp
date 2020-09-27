#include "ridefile.h"

#include <QtCore/QtDebug>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>

namespace {
const QString ISO_WITH_MILLISECONDS_FORMAT("hh:mm:ss.zzz");
}

RideFile::RideFile():
    RideFile("", "")
{
    // empty
}


RideFile::RideFile(const QString &rlvName, const QString &courseName):
    _startTime(QDateTime::currentDateTimeUtc()), _rlvName(rlvName), _courseName(courseName)
{
    // empty
}

const QDateTime &RideFile::startTime() const
{
    return _startTime;
}

const QString &RideFile::rlvName() const
{
    return _rlvName;
}

const QString &RideFile::courseName() const
{
    return _courseName;
}

int RideFile::durationInMilliSeconds() const
{
    if (_samples.empty()) {
        return 0;
    }
    return _samples.rbegin()->time.msecsSinceStartOfDay();
}

float RideFile::totalDistance() const
{
    if (_samples.empty()) {
        return 0;
    }
    return _samples.rbegin()->distance;
}

float RideFile::maximumSpeedInMps() const
{
    float max = 0;
    for (const Sample &sample: _samples) {
        max = std::max(max, sample.speed);
    }
    return max;
}

int RideFile::maximumHeartRate() const
{
    int max = 0;
    for (const Sample &sample: _samples) {
        max = std::max(max, sample.heartRate);
    }
    return max;
}

const std::vector<RideFile::Sample> &RideFile::samples() const
{
    return _samples;
}

void RideFile::addSample(const RideFile::Sample &sample)
{
    _samples.push_back(sample);
}
