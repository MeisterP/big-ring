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
#ifndef VIRTUALPOWER_H
#define VIRTUALPOWER_H

#include <functional>
#include <QtCore/QMap>
#include <QtCore/QString>
namespace indoorcycling {

enum class VirtualPowerTrainer {
    KURT_KINETIC_ROAD_MACHINE = 1,
    KURT_KINETIC_CYCLONE = 2,
    CYCLEOPS_FLUID_2 = 100,
    CYCLEOPS_JET_FLUID_PRO = 101,
    ELITE_QUBO_POWER_FLUID = 200,
    ELITE_TURBO_MUIN_2013
};

const QMap<VirtualPowerTrainer,QString> VIRTUAL_POWER_TRAINERS =
        QMap<VirtualPowerTrainer,QString>(
{{VirtualPowerTrainer::KURT_KINETIC_ROAD_MACHINE, "Kurt Kinetic Road Machine & Rock 'n' Roll"},
 {VirtualPowerTrainer::KURT_KINETIC_CYCLONE, "Kurt Kinetic Cyclone"},
 {VirtualPowerTrainer::CYCLEOPS_FLUID_2, "Saris Cycleops Fluid2"},
 {VirtualPowerTrainer::CYCLEOPS_JET_FLUID_PRO, "Saris Cycleops Jet Fluid Pro"},
 {VirtualPowerTrainer::ELITE_QUBO_POWER_FLUID, "Elite Qubo Power Fluid"},
 {VirtualPowerTrainer::ELITE_TURBO_MUIN_2013, "Elite Turbo Muin 2013 Model"}});

typedef std::function<float(float)> VirtualPowerFunctionType;

/**
 * Get the power function for a trainer.
 * @param trainer the trainer to get the function for.
 * @return a function from speed (in mps) to power (W).
 */
VirtualPowerFunctionType virtualPowerFunctionForTrainer(VirtualPowerTrainer trainer);
}

#endif // VIRTUALPOWER_H
