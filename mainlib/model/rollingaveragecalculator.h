#ifndef ROLLINGAVERAGECALCULATOR_H
#define ROLLINGAVERAGECALCULATOR_H

#include <deque>
#include <QtCore/QDateTime>

/**
 * Class which can calculate rolling averages over time periods for measurement values.
 */
class RollingAverageCalculator
{
public:
    /** Create a new calculator, which calculates the rolling average over \param durationInMilliseconds
     */
    RollingAverageCalculator(const int durationInMilliseconds);

    void addValue(const QDateTime &measurementTime, const float value);
    float currentAverage() const;
private:
    /** remove old values and return the sum of the removed values. */
    float removeOldValues(const QDateTime &lastAddedTimestamp);
    const int _durationInMilliseconds;
    float _sum = 0.0f;
    std::deque<std::pair<QDateTime,float>> _measurements;
};

#endif // ROLLINGAVERAGECALCULATOR_H
