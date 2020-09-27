#include "virtualpowertest.h"

#include "common.h"
#include "model/virtualpower.h"

#include <array>
#include <memory>
namespace {
float mphToMps(float speedMph) {
    return (speedMph / 3600) * 1609.344;
}
float kmphToMps(float kmph) {
    return kmph / 3.6;
}
}
using namespace indoorcycling;
void VirtualPowerTest::testKurtKineticRoadMachine()
{
    VirtualPowerFunctionType kkFunction = virtualPowerFunctionForTrainer(
                VirtualPowerTrainer::KURT_KINETIC_ROAD_MACHINE);

    compareWithPowerTable(kkFunction, {{16.1, 164}}, SpeedUnit::MILES_PER_HOUR);
    compareWithPowerTable(kkFunction, {{7.96, 202}}, SpeedUnit::METERS_PER_SECOND);
}

void VirtualPowerTest::testCycleopsJetFluidPro()
{
    auto jetFluidProFunction = virtualPowerFunctionForTrainer(VirtualPowerTrainer::CYCLEOPS_JET_FLUID_PRO);
    compareWithPowerTable(jetFluidProFunction, {{10.0, 64}, {24.0, 444}, {30.0, 768}}, SpeedUnit::MILES_PER_HOUR);
}

void VirtualPowerTest::testCycleopsFluid2()
{
    auto fluid2Function = virtualPowerFunctionForTrainer(VirtualPowerTrainer::CYCLEOPS_FLUID_2);
    compareWithPowerTable(fluid2Function, {{10, 103}}, SpeedUnit::MILES_PER_HOUR);
    compareWithPowerTable(fluid2Function, {{40.0, 408}}, SpeedUnit::KILOMETERS_PER_HOUR);
}

void VirtualPowerTest::testEliteQuboFluid()
{
    auto eliteQuboFluidFunction = virtualPowerFunctionForTrainer(VirtualPowerTrainer::ELITE_QUBO_POWER_FLUID);
    compareWithPowerTable(eliteQuboFluidFunction, {{ 25.0, 239 }, { 41.5, 808 }}, SpeedUnit::KILOMETERS_PER_HOUR);
}

void VirtualPowerTest::testEliteTurboMuin13()
{
    auto eliteTurboMuin2013Function = virtualPowerFunctionForTrainer(VirtualPowerTrainer::ELITE_TURBO_MUIN_2013);
    std::vector<PowerTableLine> powerTable({{ 20.0, 110 }, { 40.0, 700 }, { 60.0, 2150 }});
    compareWithPowerTable(eliteTurboMuin2013Function, powerTable, SpeedUnit::KILOMETERS_PER_HOUR);

}

void VirtualPowerTest::compareWithPowerTable(std::function<float(float)> virtualPowerFunction, const std::vector<PowerTableLine>& powerTable, SpeedUnit speedUnit)
{
    for (auto line: powerTable) {
        float speed = line.speed;
        if (speedUnit == SpeedUnit::KILOMETERS_PER_HOUR) {
            speed = kmphToMps(line.speed);
        } else if (speedUnit == SpeedUnit::MILES_PER_HOUR){
            speed = mphToMps(line.speed);
        }
        int power = qRound(virtualPowerFunction(speed));
        QCOMPARE(power, line.power);
    }
}



