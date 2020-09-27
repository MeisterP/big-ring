#ifndef ANTPOWERCHANNELHANDLER_H
#define ANTPOWERCHANNELHANDLER_H

#include "antchannelhandler.h"
#include <QtCore/QTimer>
namespace indoorcycling
{
/** Power message
 * Byte 0 contains the channel
 * Byte 1 contains the data page.
 *
 * For the Power-only page (0x10):
 * Byte 2 is the event count
 * Byte 3 pedal power (power balance)
 * Byte 4 instantaneous cadence
 * Byte 5-6 Accumulated Power
 * Byte 7-8 Instantaneous Power
*/
class PowerMessage: public BroadCastMessage
{
public:
    enum DataPages {
        POWER_ONLY_PAGE = 0x10
    };

    /** Create a PowerMessage.
     * @param antMessage the ANT+ broadcast message.
     * @return a PowerMessage
     */
    PowerMessage(const AntMessage2& antMessage = AntMessage2());

    static AntMessage2 createPowerMessage(quint8 channel, quint8 eventCount, quint8 cadence,
                                           quint16 accumulatedPower, quint16 instantaneousPower);

    /**
     * The values from the power messages are all taken from the power only data page (0x10). For other pages,
     * the values below are not valid!
     */
    bool isPowerOnlyPage() const;

    /** event count increments every time the power meter has a value to transmit. Note that multiple messages
     *  with the same event count can be sent. */
    quint8 eventCount() const;
    /** Not all power messages have cadence information */
    bool hasCadence() const;
    /** the cadence */
    quint8 instantaneousCadence() const;
    /** accumulated power. Use this to calculate average power over multiple event counts. */
    quint16 accumulatedPower() const;
    /** the power */
    quint16 instantaneousPower() const;

    QByteArray toContentBytes() const;
};

/**
 * Implements a ANT+ Power Channel Handler. This channel is used for receiving messages from an ANT+ power
 * meter.
 *
 * When power messages are received, the sensorValue(SensorValueType, AntSensorType, QVariant) signal will be emitted
 * with cadence and power values.
 */
class AntPowerSlaveChannelHandler : public AntChannelHandler
{
    Q_OBJECT
public:
    explicit AntPowerSlaveChannelHandler(int channelNumber, QObject* parent = nullptr);
    virtual ~AntPowerSlaveChannelHandler() {}
protected:
    virtual void handleBroadCastMessage(const BroadCastMessage& message) override;
private:
    PowerMessage _lastPowerMessage;
};

/**
 * Implements a ANT+ Power Master Channel Handler. This channel is used for transmitting messages so they can be picked
 * up by an ANT+ slave, like a head unit, which can display the values.
 */
class AntPowerMasterChannelHandler : public AntMasterChannelHandler
{
    Q_OBJECT
public:
    explicit AntPowerMasterChannelHandler(int channelNumber, QObject* parent = nullptr);
    virtual ~AntPowerMasterChannelHandler() {}
public slots:
    /** Set the power that will be transmitted */
    void setPower(quint16 power);
    virtual void sendSensorValue(const SensorValueType valueType, const QVariant &value) override;
protected:
    virtual quint8 transmissionType() const override;
    virtual void channelOpened() override;
private slots:
    void sendPowerUpdate();
private:
    QTimer* const _updateTimer;

    quint8 _eventCount;
    quint16 _accumulatedPower;
    quint16 _instantaneousPower;
    quint8 _cadence;
};
}
#endif // ANTPOWERCHANNELHANDLER_H
