#ifndef POWERSENSORITEM_H
#define POWERSENSORITEM_H

#include "sensoritem.h"

#include <memory>
#include <QtCore/QDateTime>

#include "model/rollingaveragecalculator.h"

/**
 * A Sensor item that displays it's value using a rolling average.
 */
class RollingAverageSensorItem : public SensorItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    /**
     * Create a new RollingAvergaeSensorItem.
     *
     * @param quantity the quantity to display.
     * @param averageTimeMilliseconds the time (in milliseconds) over which the
 * rolling average is calculated.
     * @param displayUpdateTimeMilliseconds the time taken between display updates.
     * @param parent parent item.
     */
    explicit RollingAverageSensorItem(const QuantityPrinter::Quantity quantity, const int averageTimeMilliseconds, const int displayUpdateTimeMilliseconds, QObject *parent = 0);
public slots:
    virtual void setValue(const QVariant &value) override;
private:
    const int _displayUpdateTimeMilliseconds;
    QDateTime _lastDisplayedTime;
    std::unique_ptr<RollingAverageCalculator> _rollingAverageCalculator;
};

#endif // POWERSENSORITEM_H
