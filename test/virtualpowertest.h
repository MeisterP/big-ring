#ifndef VIRTUALPOWERTEST_H
#define VIRTUALPOWERTEST_H

#include <QtCore/QObject>

#include <functional>
#include <vector>

enum class SpeedUnit {
    METERS_PER_SECOND,
    KILOMETERS_PER_HOUR,
    MILES_PER_HOUR
};

struct PowerTableLine
{
    float speed;
    int power;
};

class VirtualPowerTest : public QObject
{
    Q_OBJECT
private slots:
    void testKurtKineticRoadMachine();
    void testCycleopsJetFluidPro();
    void testCycleopsFluid2();
    void testEliteQuboFluid();
    void testEliteTurboMuin13();
private:
    void compareWithPowerTable(std::function<float(float)> virtualPowerFunction,const std::vector<PowerTableLine>& powerTable, SpeedUnit speedUnit);
};

#endif // VIRTUALPOWERTEST_H
