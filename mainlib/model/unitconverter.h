#ifndef UNITCONVERTER_H
#define UNITCONVERTER_H

#include <QtCore/QObject>
#include <QtCore/QSettings>

class UnitConverter : public QObject
{
    Q_OBJECT
public:
    /** The Unit Systems that can be converted to */
    enum class System {
        Metric, Imperial
    };

    enum class DistanceUnit {
        Meter,
        Kilometer,
        Yard,
        Mile,
        Foot
    };
    enum class SpeedUnit {
        MetersPerSecond,
        KilometersPerHour,
        MilesPerHour
    };
    enum class WeightUnit {
        Kilograms,
        Lbs
    };

    explicit UnitConverter(QObject *parent);

    System system();
    DistanceUnit distanceUnitForSystem();
    DistanceUnit altitudeUnitForSystem();
    SpeedUnit speedUnitForSystem();
    WeightUnit weightUnitForSystem();

    /** convert a distance to the distance unit provided */
    double convertDistanceToSystemUnit(double distance);
    static double convertDistanceTo(double distance, DistanceUnit);
    /** convert a distance from the distance unit provided, returning the distance in SI */
    static double convertDistanceFrom(double distance, DistanceUnit unit);
    double convertDistanceFromSystemUnit(double distance);

    static double convertSpeedTo(double speedMetersPerSecond, SpeedUnit unit);
    double convertSpeedToSystemUnit(double speedMetersPerSecond);
    static double convertAltitudeTo(double meters, DistanceUnit unit);
    double convertAltitudeToSystemUnit(double meters);
    double convertAltitudeFromSystemUnit(double altitudeInSystemUnit);

    static double convertWeightTo(double weightInKilograms, WeightUnit unit);
    double convertWeightToSystemUnit(double weightInKilograms);
    double convertWeightFromSystemUnit(double weightInSystemUnit);
private:
    QSettings _settings;
};

#endif // UNITCONVERTER_H
