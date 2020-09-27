#ifndef RIDEFILE_H
#define RIDEFILE_H

#include <QtCore/QDateTime>
#include <QtCore/QTime>

#include "geoposition.h"

/**
 * A record of a complete ride.
 */
class RideFile
{
public:
    /**
     * A Sample from the Ride
     */
    struct Sample {
        QTime time;

        float altitude;
        int cadence;
        float distance;
        int heartRate;
        int power;
        float speed;
        GeoPosition position;
    };

    explicit RideFile();
    explicit RideFile(const QString &rlvName, const QString &courseName);

    /**
     * Start time of the ride. Timezone of the start time is UTC!
     */
    const QDateTime &startTime() const;
    const QString &rlvName() const;
    const QString &courseName() const;
    const std::vector<Sample> &samples() const;

    int durationInMilliSeconds() const;
    float totalDistance() const;
    float maximumSpeedInMps() const;
    int maximumHeartRate() const;

    void addSample(const Sample &sample);

private:
    const QDateTime _startTime;
    const QString _rlvName;
    const QString _courseName;
    std::vector<Sample> _samples;
};

#endif // RIDEFILE_H
