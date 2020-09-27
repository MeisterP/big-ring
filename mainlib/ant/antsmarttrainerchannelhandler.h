#ifndef ANTSMARTTRAINERCHANNELHANDLER_H
#define ANTSMARTTRAINERCHANNELHANDLER_H
#include <QtCore/QTimer>
#include "antchannelhandler.h"
namespace indoorcycling {
class CapabilitiesMessage;
class CommandStatusReplyMessage;
class GeneralFitnessEquipmentMessage;
class SpecificTrainerDataMessage;
class TrackResistanceMessage;
class UserConfigurationMessage;
class WindResistanceMessage;

class AntSmartTrainerChannelHandler : public AntChannelHandler
{
    Q_OBJECT
public:
    enum class DataPage {
        GENERAL_FITNESS_EQUIPMENT = 0x10,
        SPECIFIC_TRAINER_DATA = 0x19,
        WIND_RESISTANCE = 0x32,
        TRACK_RESISTANCE = 0x33,
        FE_CAPABILITIES = 0x36,
        USER_CONFIGURATION = 0x37,
        DATA_PAGE_REQUEST = 0x46,
        COMMAND_STATUS = 0x47,
    };

    enum class State {
        INITIAL,
        CAPABILITIES_REQUESTED,
        INITIALIZED
    };

    explicit AntSmartTrainerChannelHandler(int channelNumber, QObject *parent = nullptr);

    void setWeight(const qreal userWeightInKilograms, const qreal bikeWeightInKilograms);
    void setSlope(const qreal slopeInPercent);
public slots:
    virtual void close() override;
protected:
    virtual void channelOpened() override final;
    virtual void handleBroadCastMessage(const BroadCastMessage &message) override;
private slots:
    void checkConfiguration();
private:
    void setState(State state);
    void handleCapabilitiesMessage(const CapabilitiesMessage &message);
    void handleGeneralFitnessEquipmentMessage(const GeneralFitnessEquipmentMessage& message);
    void handleSpecificTrainerDataMessage(const SpecificTrainerDataMessage &message);
    void handleCommandStatusReplyMessage(const CommandStatusReplyMessage &message);
    void handleTrackResistanceMessage(const TrackResistanceMessage &message);
    void handleWindResistanceMessage(const WindResistanceMessage &message);
    void handleUserConfigurationMessage(const UserConfigurationMessage &message);

    void requestCapabilities();
    void sendWindResistanceMessage();
    void sendTrackResistanceMessage();
    void sendUserConfigurationMessage();

    AntMessage2 createWindResistenceMessage();
    AntMessage2 createTrackResistanceMessage();
    AntMessage2 createUserConfigurationMessage();
    AntMessage2 createRequestMessage(const DataPage dataPage);

    qreal _slope = 0.0;
    qreal _bikeWeight = 10.0;
    qreal _userWeight = 75.0;

    QTimer *_configurationCheckerTimer;
    State _state = State::INITIAL;
};
}
#endif // ANTSMARTTRAINERCHANNELHANDLER_H
