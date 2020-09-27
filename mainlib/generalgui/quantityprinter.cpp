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

#include "quantityprinter.h"
#include "model/unitconverter.h"

#include <QtCore/QtDebug>

QuantityPrinter::QuantityPrinter(QObject *parent) :
    QObject(parent), _unitConverter(new UnitConverter(this))
{
    // empty
}

QString QuantityPrinter::unitString(QuantityPrinter::Quantity quantity, QuantityPrinter::Precision precision, QVariant value) const
{
    switch(quantity) {
    case Quantity::Distance:
        return unitForDistance(precision, value);
    case Quantity::Speed:
        return (_unitConverter->system() == UnitConverter::System::Imperial) ? "MPH" : "KM/H";
    case Quantity::Altitude:
        return (_unitConverter->system() == UnitConverter::System::Imperial) ? "Ft" : "M";
    case Quantity::Cadence:
        return tr("RPM");
    case Quantity::Power:
        return tr("W");
    case Quantity::HeartRate:
        return tr("BPM");
    case Quantity::Grade:
        return "%";
    case Quantity::FramesPerSecond:
        return "FPS";
    default:
        return "#";
    }
}

QString QuantityPrinter::unitForDistance(QuantityPrinter::Precision precision, QVariant value) const
{
    if (_unitConverter->system() == UnitConverter::System::Imperial) {
        if (precision == Precision::AlwaysPrecise || (precision == Precision::Precise && _unitConverter->convertDistanceFrom(1, UnitConverter::DistanceUnit::Mile) > value.toReal())) {
            return "Y";
        }
        return "Mi";
    }
    if (precision == Precision::AlwaysPrecise || (precision == Precision::Precise && value.toReal() < 1000)) {
        return "M";
    }
    return "KM";
}

QString QuantityPrinter::unitForAltitude() const
{
    return unitString(Quantity::Altitude);
}

QString QuantityPrinter::unitForWeight() const
{
    if (_unitConverter->system() == UnitConverter::System::Imperial) {
        return "lbs";
    }
    return "kg";
}

QString QuantityPrinter::print(QVariant value, QuantityPrinter::Quantity quantity, Precision precision, int width) const
{
    switch(quantity) {
    case Quantity::Altitude:
        return printAltitude(value.toReal());
    case Quantity::Distance:
        return printDistance(value.toReal(), precision, width);
    case Quantity::Speed:
        return printSpeed(value.toReal(), width);
    case Quantity::Cadence:
        return QString("%1").arg(value.toInt(), width);
    case Quantity::Power:
        return QString("%1").arg(value.toInt(), width);
    case Quantity::HeartRate:
        return QString("%1").arg(value.toInt(), width);
    case Quantity::Grade:
        return QString("%1").arg(value.toReal(), width, 'f', 1);
    case Quantity::Weight:
        return printWeight(value.toReal());
    case Quantity::FramesPerSecond:
        return QString("%1").arg(value.toInt(), width);
    }
    Q_ASSERT_X(false, "QuantityPrinter::print", "This should not be reached");
    return "";
}

QString QuantityPrinter::printDistance(qreal meters, Precision precision, int width) const
{
    if (_unitConverter->system() == UnitConverter::System::Imperial) {
        if (precision == Precision::AlwaysPrecise || (precision == Precision::Precise && _unitConverter->convertDistanceFrom(1, UnitConverter::DistanceUnit::Mile) > meters)) {
            return QString("%1").arg(_unitConverter->convertDistanceTo(meters, UnitConverter::DistanceUnit::Yard), width, 'f', 0);
        }
        return QString("%1").arg(_unitConverter->convertDistanceTo(meters, UnitConverter::DistanceUnit::Mile), width, 'f', 2);
    }
    if (precision == Precision::AlwaysPrecise || (precision == Precision::Precise && meters < 1000)) {
        return QString("%1").arg(meters, width, 'f', 0);
    }
    return QString("%1").arg(_unitConverter->convertDistanceTo(meters, UnitConverter::DistanceUnit::Kilometer), width, 'f', 2);
}

QString QuantityPrinter::printSpeed(qreal metersPerSecond, int width) const
{
    return QString("%1").arg(_unitConverter->convertSpeedToSystemUnit(metersPerSecond),  width, 'f', 1);
}

QString QuantityPrinter::printAltitude(qreal meters) const
{
    return QString("%1").arg(static_cast<int>(qRound(_unitConverter->convertAltitudeToSystemUnit(meters))));
}

QString QuantityPrinter::printWeight(qreal weightInKilograms) const
{
    return QString("%1").arg(_unitConverter->convertWeightToSystemUnit(weightInKilograms), 4, 'f', 1);
}
