#include "rollingaveragecalculatortest.h"

#include <QtTest/QTest>

namespace
{
const int DURATION = 3000; // ms
}
RollingAverageCalculatorTest::RollingAverageCalculatorTest(QObject *parent) :
    QObject(parent)
{
    // empty
}

void RollingAverageCalculatorTest::init()
{
    _calculator.reset(new RollingAverageCalculator(DURATION));
}

void RollingAverageCalculatorTest::testWithoutMeasurements()
{
    QCOMPARE(_calculator->currentAverage(), 0.0f);
}

void RollingAverageCalculatorTest::testWithOneMeasurement()
{
    _calculator->addValue(QDateTime::currentDateTime(), 10.0);

    QCOMPARE(_calculator->currentAverage(), 10.0);
}

void RollingAverageCalculatorTest::testWithTwoMeasurementsWithinDuration()
{
    _calculator->addValue(QDateTime::currentDateTime(), 10.0);
    _calculator->addValue(QDateTime::currentDateTime(), 20.0);

    QCOMPARE(_calculator->currentAverage(), 15.0);
}

void RollingAverageCalculatorTest::testWithTwoMeasurementsJustInsideDuration()
{
    const QDateTime firstTime = QDateTime::currentDateTime();
    const QDateTime secondTime = firstTime.addSecs(3);

    _calculator->addValue(firstTime, 10.0);
    _calculator->addValue(secondTime, 20.0);

    QCOMPARE(_calculator->currentAverage(), 15.0);
}

void RollingAverageCalculatorTest::testWithTwoMeasurementsJustOutsideDuration()
{
    const QDateTime firstTime = QDateTime::currentDateTime();
    const QDateTime secondTime = firstTime.addMSecs(3001);

    _calculator->addValue(firstTime, 10.0);
    _calculator->addValue(secondTime, 20.0);

    QCOMPARE(_calculator->currentAverage(), 20.0);
}
