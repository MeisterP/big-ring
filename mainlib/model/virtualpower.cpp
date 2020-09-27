/*
 * Copyright (c) 2015 Ilja Booij (ibooij@gmail.com)
 *
 * This file is part of Big Ring Indoor Video Cycling
 *
 * Big Ring Indoor Video Cycling is free software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Big Ring Indoor Video Cycling  is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Big Ring Indoor Video Cycling.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include "virtualpower.h"

#include "unitconverter.h"

#include <QtCore/QtDebug>
#include <QtCore/QtMath>
namespace
{
using indoorcycling::VirtualPowerTrainer;
using indoorcycling::VirtualPowerFunctionType;

typedef std::function<float(float)> SpeedConversionFunctionType;

struct PowerCurve {
    UnitConverter::SpeedUnit speedUnit;
    std::vector<float> coefficients;
};

const PowerCurve IDENTITY_POWER_CURVE = { UnitConverter::SpeedUnit::MetersPerSecond, {1} };

const std::map<VirtualPowerTrainer, PowerCurve> powerCurves = {
    // Kurt Kinetic Road Machine: P = 5.244820 * MPH + 0.019168 * MPH^3
    { VirtualPowerTrainer::KURT_KINETIC_ROAD_MACHINE, { UnitConverter::SpeedUnit::MilesPerHour, {0, 5.244820, 0, 0.019168} }},
    // Kurt Kinetic Cyclone: P = 6.481090 * mph + 0.020106 * (mph*mph*mph)
    { VirtualPowerTrainer::KURT_KINETIC_CYCLONE, { UnitConverter::SpeedUnit::MilesPerHour, {0, 6.481090, 0, 0.020106} }},
    // Cycleops Fluid 2: P = 8.9788 * MPH + 0.0137 * MPH^2 + 0.0115 * MPH^3
    { VirtualPowerTrainer::CYCLEOPS_FLUID_2, { UnitConverter::SpeedUnit::MilesPerHour, {0, 8.9788, 0.0137, 0.0115} }},
    // Cyclops Jet Fluid Pro: P = -46.511 + 15.468 * MPH - 1.0988 * MPH^2 + 0.0736 * MPH^3 - 0.0008 * MPH^4
    { VirtualPowerTrainer::CYCLEOPS_JET_FLUID_PRO, { UnitConverter::SpeedUnit::MilesPerHour, {-46.511, 15.468, -1.0988, 0.0736,-0.0008} }},
    // ELITE QUBO POWER FLUID f(x) = 4.31746 * kmph -2.59259e-002 * kmph^2 +  9.41799e-003 * kmph^3
    { VirtualPowerTrainer::ELITE_QUBO_POWER_FLUID, { UnitConverter::SpeedUnit::KilometersPerHour, {0, 4.31746, -2.59259e-002, 9.41799e-003} }},
    // Elite Turbo Muin 2013 model P =  0.00791667 v^3 + 0.125 v^2 - 0.16669 v
    { VirtualPowerTrainer::ELITE_TURBO_MUIN_2013, { UnitConverter::SpeedUnit::KilometersPerHour, {0, - 0.16669, 0.125, 0.00791667} }}
};

const PowerCurve &findPowerCurve(VirtualPowerTrainer trainer)
{
    auto it = powerCurves.find(trainer);
    if (it == powerCurves.end()) {
        return IDENTITY_POWER_CURVE;
    }
    return (*it).second;
}

const SpeedConversionFunctionType determineSpeedConversionFunction(VirtualPowerTrainer trainer) {
    switch (findPowerCurve(trainer).speedUnit) {
    case UnitConverter::SpeedUnit::KilometersPerHour:
        return [](double metersPerSecond) {
            return UnitConverter::convertSpeedTo(metersPerSecond, UnitConverter::SpeedUnit::KilometersPerHour);
        };
    case UnitConverter::SpeedUnit::MilesPerHour:
        return [](double metersPerSecond) {
            return UnitConverter::convertSpeedTo(metersPerSecond, UnitConverter::SpeedUnit::MilesPerHour);
        };
    default:
        return [](float mps) { return mps; };
    }
}
}

namespace indoorcycling
{

VirtualPowerFunctionType virtualPowerFunctionForTrainer(VirtualPowerTrainer trainer)
{
    const PowerCurve &powerCurve = findPowerCurve(trainer);
    SpeedConversionFunctionType speedConversionFunction = determineSpeedConversionFunction(trainer);

    return [powerCurve, speedConversionFunction](float speedMetersPerSecond) {
        const float convertedSpeed = speedConversionFunction(speedMetersPerSecond);

        float power = 0;
        int i = 0;
        for(auto &factor: powerCurve.coefficients) {
            power += factor * qPow(convertedSpeed, i);
            i += 1;
        }
        return power;
    };
}
}
