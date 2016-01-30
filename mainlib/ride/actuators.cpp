
#include "actuators.h"
#include "config/bigringsettings.h"
#include "model/cyclist.h"

#include "ant/antcentraldispatch.h"
namespace indoorcycling {

Actuators::Actuators(const Cyclist *cyclist, indoorcycling::AntCentralDispatch *antCentralDispatch,
                                    const NamedSensorConfigurationGroup &sensorConfigurationGroup, QObject *parent):
    QObject(parent), _cyclist(cyclist), _antCentralDispatch(antCentralDispatch), _sensorConfigurationGroup(sensorConfigurationGroup),
    _maximumSlope(BigRingSettings().maximumUphillForSmartTrainer()),
    _minimumSlope(-BigRingSettings().maximumDownhillForSmartTrainer())
{
    connect(_antCentralDispatch, &AntCentralDispatch::sensorFound, this, &Actuators::setSensorFound);
}

void Actuators::initialize()
{
    // empty
}

void Actuators::setSensorFound(AntSensorType channelType, int)
{
    if (channelType == AntSensorType::SMART_TRAINER) {
        configureWeight();
    }
}

void Actuators::setSlope(const qreal slopeInPercent)
{
    const qreal cappedSlope = qBound(_minimumSlope, slopeInPercent, _maximumSlope);
    if (!qFuzzyCompare(cappedSlope, _currentSlope)) {
        _currentSlope = cappedSlope;
        _antCentralDispatch->setSlope(cappedSlope);
    }
}

void Actuators::configureWeight()
{
    _antCentralDispatch->setWeight(_cyclist->userWeight(), _cyclist->bikeWeight());
}
}
