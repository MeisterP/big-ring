#include "rollingaveragesensoritem.h"

#include <QtCore/QtDebug>

#include "model/rollingaveragecalculator.h"

RollingAverageSensorItem::RollingAverageSensorItem(const QuantityPrinter::Quantity quantity, const int averageTimeMilliseconds, const int displayUpdateTimeMilliseconds, QObject *parent):
    SensorItem(quantity, parent), _displayUpdateTimeMilliseconds(displayUpdateTimeMilliseconds), _lastDisplayedTime(QDateTime::fromMSecsSinceEpoch(0))
{
    _rollingAverageCalculator.reset(new RollingAverageCalculator(averageTimeMilliseconds));
}

void RollingAverageSensorItem::setValue(const QVariant &powerValue)
{
    QDateTime now(QDateTime::currentDateTime());
    _rollingAverageCalculator->addValue(now, powerValue.toFloat());

    // Only display anything if the last time that the value was updated in
    // the display was more than _displayTimeMilliseconds ago.
    if (now > _lastDisplayedTime.addMSecs(_displayUpdateTimeMilliseconds)) {
        SensorItem::setValue(
                    QVariant::fromValue(_rollingAverageCalculator->currentAverage()));
        _lastDisplayedTime = now;
    }
}
