#ifndef ANTSPEEDANDCADENCECHANNELHANDLER_H
#define ANTSPEEDANDCADENCECHANNELHANDLER_H

#include "antchannelhandler.h"

namespace indoorcycling
{

/**
 * Speed And Cadence message (Device Type 0x79)
 * Byte 0 contains the channel
 *
 * Byte 1-2 cadence event time
 * Byte 3-4 cumulative cadence revolution count
 * Byte 5-6 bike speed event time
 * Byte 7-8 cumulative wheel revolution count
*/
class SpeedAndCadenceMessage: public BroadCastMessage
{
public:
    SpeedAndCadenceMessage(const AntMessage2& antMessage);

    quint16 cadenceEventTime() const;
    quint16 pedalRevolutions() const;
    quint16 speedEventTime() const;
    quint16 wheelRevolutions() const;

};

/**
 * Bike Speed Message (Device Type 0x7B).
 * Bytes:
 * 0: channel
 * 1: data page
 * 2-4: depends on data page.
 * 5-6: last bike speed event time in 1/1024s
 * 7-8: cumulative wheel revolution count
 */
class SpeedMessage: public BroadCastMessage
{
public:
    SpeedMessage(const AntMessage2& antMessage);

    /** The time of the last valid speed event, in 1/1024s. */
    quint16 speedEventTime() const;
    /** total number of wheel revolutions */
    quint16 wheelRevolutions() const;
};

/**
 * Bike Cadence Message (Device Type 0x7A).
 * Bytes:
 * 0: channel
 * 1: data page
 * 2-4: depends on data page.
 * 5-6: bike cadence event time
 * 7-8: cumulative pedal revolution count
 */
class CadenceMessage: public BroadCastMessage
{
public:
    CadenceMessage(const AntMessage2& antMessage);

    quint16 cadenceEventTime() const;
    quint16 pedalRevolutions() const;
};

class AntSpeedAndCadenceChannelHandler : public AntChannelHandler
{
public:
    virtual ~AntSpeedAndCadenceChannelHandler() {}
    /**
     * Create a new combined ANT+ speed and cadence handling channel.
     * @return
     */
    static AntChannelHandler* createCombinedSpeedAndCadenceChannelHandler (int channelNumber, QObject* parent);
    static AntChannelHandler* createCadenceChannelHandler (int channelNumber, QObject* parent);
    static AntChannelHandler* createSpeedChannelHandler (int channelNumber, QObject* parent);
protected:
    virtual void handleBroadCastMessage(const BroadCastMessage &message) override;
private:
    AntSpeedAndCadenceChannelHandler(int channelNumber, AntSensorType sensorType, AntSportPeriod period,
                                     QObject* parent);

    void handleSpeedAndCadenceMessage(const SpeedAndCadenceMessage& message);
    void handleCadenceMessage(const CadenceMessage& message);
    void handleSpeedMessage(const SpeedMessage& message);
    void calculateSpeed(const quint16 previousTime, const quint16 previousWheelRevolutions,
                        const quint16 currentTime, const quint16 currentWheelRevolutions);
    void calculateCadence(const quint16 previousTime, const quint16 previousPedalRevolutions,
                        const quint16 currentTime, const quint16 currentPedalRevolutions);
    BroadCastMessage _previousMessage;

};
}
#endif // ANTSPEEDANDCADENCECHANNELHANDLER_H
