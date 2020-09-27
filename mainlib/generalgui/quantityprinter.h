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

#ifndef QUANTITYPRINTER_H
#define QUANTITYPRINTER_H

#include <QtCore/QObject>
#include <QtCore/QSettings>

class UnitConverter;
class QuantityPrinter : public QObject
{
    Q_OBJECT
public:
    enum class Precision {
        NonPrecise,
        Precise,
        AlwaysPrecise
    };

    enum class Quantity {
        Altitude,
        Distance,
        Speed,
        Power,
        HeartRate,
        Cadence,
        Grade,
        Weight,
        FramesPerSecond
    };

    explicit QuantityPrinter(QObject *parent = 0);
    QString unitString(Quantity quantity, QuantityPrinter::Precision precision = Precision::NonPrecise, QVariant value = QVariant::fromValue(0.0)) const;
    QString unitForDistance(QuantityPrinter::Precision precision, QVariant value) const;
    QString unitForAltitude() const;
    QString unitForWeight() const;
    QString print(QVariant value, Quantity quantity, Precision = Precision::NonPrecise, int width = 5) const;
    QString printDistance(qreal meters, Precision = Precision::NonPrecise, int width = 5) const;
    QString printSpeed(qreal metersPerSecond, int width) const;
    QString printAltitude(qreal meters) const;
    QString printWeight(qreal weightInKilograms) const;
private:
    UnitConverter *_unitConverter;
    QSettings _settings;
};

#endif // QUANTITYPRINTER_H
