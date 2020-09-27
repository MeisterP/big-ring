#include "rollingaveragecalculator.h"

RollingAverageCalculator::RollingAverageCalculator(const int durationInMilliseconds):
    _durationInMilliseconds(durationInMilliseconds)
{
}

void RollingAverageCalculator::addValue(const QDateTime &measurementTime, const float value)
{
    _sum -= removeOldValues(measurementTime);
    _measurements.push_back(std::make_pair(measurementTime, value));
    _sum += value;
}

float RollingAverageCalculator::currentAverage() const
{
    if (_measurements.empty()) {
        return 0.0f;
    }

    return _sum / _measurements.size();
}

float RollingAverageCalculator::removeOldValues(const QDateTime &lastAddedTimestamp)
{
    const QDateTime oldestValidTimestamp = lastAddedTimestamp.addMSecs(-_durationInMilliseconds);
    // search for the first timestamp that is not too old.
    auto firstValidValueIt = std::find_if(_measurements.begin(), _measurements.end(),
                                          [oldestValidTimestamp](const std::pair<QDateTime,float> &measurement) {
        return measurement.first >= oldestValidTimestamp;
    });
    // get the sum of all values that will be removed.
    const float sum = std::accumulate(_measurements.begin(), firstValidValueIt, 0.0f, [](float accumulator, const std::pair<QDateTime,float> &measurement) {
        return accumulator + measurement.second;
    });
    _measurements.erase(_measurements.begin(), firstValidValueIt);
    return sum;
}
