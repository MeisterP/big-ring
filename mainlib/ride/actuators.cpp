
#include "actuators.h"
#include "config/bigringsettings.h"
#include "model/cyclist.h"

#include "ant/antcentraldispatch.h"
namespace indoorcycling {

Actuators::Actuators(const Cyclist *cyclist, indoorcycling::AntCentralDispatch *antCentralDispatch, QObject *parent):
    QObject(parent), _cyclist(cyclist), _antCentralDispatch(antCentralDispatch),
    _maximumSlope(BigRingSettings().maximumUphillForSmartTrainer()),
    _minimumSlope(BigRingSettings().maximumDownhillForSmartTrainer()),
    _difficultySetting(BigRingSettings().difficultySetting() * 0.01)
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
    if (!qFuzzyCompare(slopeInPercent, _currentSlope)) {
        _currentSlope = slopeInPercent;

        const qreal slopeWithDifficultySetting = slopeInPercent * _difficultySetting;
        const qreal cappedSlope = qBound(_minimumSlope, slopeWithDifficultySetting, _maximumSlope);

        _antCentralDispatch->setSlope(cappedSlope);
    }
}

void Actuators::configureWeight()
{
    _antCentralDispatch->setWeight(_cyclist->userWeight(), _cyclist->bikeWeight());
}
}
