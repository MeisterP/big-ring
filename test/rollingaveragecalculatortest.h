#ifndef ROLLINGAVERAGECALCULATORTEST_H
#define ROLLINGAVERAGECALCULATORTEST_H

#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include "model/rollingaveragecalculator.h"

class RollingAverageCalculatorTest : public QObject
{
    Q_OBJECT
public:
    explicit RollingAverageCalculatorTest(QObject *parent = 0);

private slots:
    void init();
    void testWithoutMeasurements();
    void testWithOneMeasurement();
    void testWithTwoMeasurementsWithinDuration();
    void testWithTwoMeasurementsJustInsideDuration();
    void testWithTwoMeasurementsJustOutsideDuration();
private:
    QScopedPointer<RollingAverageCalculator> _calculator;

};

#endif // ROLLINGAVERAGECALCULATORTEST_H
