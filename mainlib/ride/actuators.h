#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <QtCore/QObject>
#include "ant/antsensortype.h"

class Cyclist;
namespace indoorcycling {

class AntCentralDispatch;
class Actuators : public QObject
{
    Q_OBJECT
public:
    explicit Actuators(const Cyclist* cyclist, AntCentralDispatch *antCentralDispatch, QObject *parent = nullptr);
public slots:
    void initialize();
    void setSensorFound(AntSensorType channelType, int deviceNumber);
    void setSlope(qreal slopeInPercent);

private:
    void configureWeight();

    const Cyclist *const _cyclist;
    AntCentralDispatch* const _antCentralDispatch;

    qreal _currentSlope = 0.0;
    const qreal _maximumSlope;
    const qreal _minimumSlope;
    const qreal _difficultySetting;
};
}
#endif // ACTUATORS_H
