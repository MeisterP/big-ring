#include "antmessage2test.h"
#include "common.h"

#include <QtCore/QtDebug>
#include <QtCore/QtEndian>

void AntMessage2Test::systemReset()
{
    AntMessage2 resetMessage = AntMessage2::systemReset();

    QCOMPARE_BYTE(resetMessage.id(), AntMessage2::AntMessageId::SYSTEM_RESET);
    QByteArray bytes = resetMessage.toBytes();

    qDebug() << bytes.size();
    QCOMPARE(bytes.size(), 5);
    QCOMPARE_BYTE(bytes[0], AntMessage2::SYNC_BYTE);
    QCOMPARE_BYTE(bytes[1], 1);

    qDebug() << bytes.toHex();
}

void AntMessage2Test::setNetworkKey()
{
    std::array<quint8,8> theKey = {{ 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8 }};
    AntMessage2 msg = AntMessage2::setNetworkKey(4, theKey);

    QCOMPARE_BYTE(msg.id(), AntMessage2::AntMessageId::SET_NETWORK_KEY);
    QByteArray bytes = msg.toBytes();

    QCOMPARE(bytes.size(), 13);
    QCOMPARE_BYTE(bytes[0], AntMessage2::SYNC_BYTE);
    QCOMPARE_BYTE(bytes[1], 9);
    QByteArray key = bytes.mid(4, 8);
    QByteArray comparison = QByteArray::fromRawData(reinterpret_cast<char*>(&theKey), 8);
    QCOMPARE(key, comparison);
}

void AntMessage2Test::unassignChannel()
{
    AntMessage2 msg = AntMessage2::unassignChannel(3u);
    QCOMPARE_BYTE(msg.id(), AntMessage2::AntMessageId::UNASSIGN_CHANNEL);
    QCOMPARE(msg.toBytes().size(), 5);
    int channel = msg.toBytes()[3];
    QCOMPARE(channel, 3);
}

void AntMessage2Test::assignChannel()
{
    AntMessage2 msg = AntMessage2::assignChannel(1, 0x10, 3);
    QCOMPARE_BYTE(msg.id(), AntMessage2::AntMessageId::ASSIGN_CHANNEL);
    QCOMPARE(msg.toBytes().size(), 7);
    QByteArray content = msg.toBytes().mid(3, 3);
    int channel = content[0];
    QCOMPARE(channel, 1);
    int channelType = content[1];
    QCOMPARE(channelType, 0x10);
    int networkNumber = content[2];
    QCOMPARE(networkNumber, 3);
}

void AntMessage2Test::openChannel()
{
    AntMessage2 msg = AntMessage2::openChannel(1);
    QCOMPARE_BYTE(msg.id(), AntMessage2::AntMessageId::OPEN_CHANNEL);
    QCOMPARE(msg.toBytes().size(), 5);
    QByteArray content = msg.toBytes().mid(3, 1);
    int channel = content[0];
    QCOMPARE(channel, 1);

}

void AntMessage2Test::requestMessage()
{
    AntMessage2 msg = AntMessage2::requestMessage(1, AntMessage2::AntMessageId::SET_CHANNEL_ID);

    QCOMPARE_BYTE(msg.id(), AntMessage2::AntMessageId::REQUEST_MESSAGE);
    QCOMPARE(msg.toBytes().size(), 6);
    QByteArray content = msg.toBytes().mid(3, 2);
    int channel = content[0];
    QCOMPARE(channel, 1);
    int messageId = content[1];
    QCOMPARE(messageId, static_cast<int>(AntMessage2::AntMessageId::SET_CHANNEL_ID));
}

void AntMessage2Test::setChannelId()
{
    quint16 theDeviceId = 10000;
    indoorcycling::AntSensorType theDeviceType = indoorcycling::AntSensorType::HEART_RATE;
    AntMessage2 msg = AntMessage2::setChannelId(1, theDeviceId, theDeviceType);

    QCOMPARE_BYTE(msg.id(), AntMessage2::AntMessageId::SET_CHANNEL_ID);
    QCOMPARE(msg.toBytes().size(), 9);
    QByteArray content = msg.toBytes().mid(3, 5);
    int channel = content[0];
    QCOMPARE(channel, 1);
    const uchar* deviceIdChars = reinterpret_cast<uchar*>(&content.data()[1]);
    quint16 deviceId = qFromLittleEndian<quint16>(deviceIdChars);
    QCOMPARE(deviceId, deviceId);
    int deviceType = content[3];
    QCOMPARE(deviceType, static_cast<int>(theDeviceType));
    int transmissionType = content[4];
    QCOMPARE(transmissionType, 0);
}

void AntMessage2Test::setSearchTimeout()
{
    AntMessage2 msg = AntMessage2::setSearchTimeout(2, 30);

    QCOMPARE_BYTE(msg.id(), AntMessage2::AntMessageId::SET_SEARCH_TIMEOUT);
    QByteArray bytes = msg.toBytes();
    QCOMPARE(bytes.size(), 6);
    QByteArray content = bytes.mid(3, 2);
    int channel = content[0];
    QCOMPARE(channel, 2);
    // timeout in seconds = timeout * 2.5
    QCOMPARE(static_cast<int>(content[1]), 12);
}

void AntMessage2Test::setChannelFrequency()
{
    AntMessage2 msg = AntMessage2::setChannelFrequency(2, AntMessage2::ANT_PLUS_CHANNEL_FREQUENCY);

    QCOMPARE_BYTE(msg.id(), AntMessage2::AntMessageId::SET_CHANNEL_FREQUENCY);
    QByteArray bytes = msg.toBytes();
    QCOMPARE(bytes.size(), 6);
    QByteArray content = bytes.mid(3, 2);
    int channel = content[0];
    QCOMPARE(channel, 2);
    quint16 frequency = content[1] + AntMessage2::ANT_CHANNEL_FREQUENCY_BASE;
    quint16 expected = AntMessage2::ANT_PLUS_CHANNEL_FREQUENCY;
    QCOMPARE(frequency, expected);
}

void AntMessage2Test::setChannelPeriod()
{
    AntMessage2 msg = AntMessage2::setChannelPeriod(2, AntSportPeriod::HR);

    QCOMPARE_BYTE(msg.id(), AntMessage2::AntMessageId::SET_CHANNEL_PERIOD);
    QByteArray bytes = msg.toBytes();
    QCOMPARE(bytes.size(), 7);
    QByteArray content = bytes.mid(3, 3);
    int channel = content[0];
    QCOMPARE(channel, 2);

    quint8 lowByte = content[1];
    quint8 highByte = content[2];

    quint16 period = ((highByte << 8) & 0xFF00) + lowByte;
    QCOMPARE(period, static_cast<quint16>(AntSportPeriod::HR));
}

void AntMessage2Test::channelEventNoError()
{
    QByteArray bytes = QByteArray::fromHex(QByteArray::fromRawData("a40340014615a0", 14));
    AntChannelEventMessage msg(bytes);

    QCOMPARE(msg.id(), AntMessage2::AntMessageId::CHANNEL_EVENT);
    quint8 channelNumber = 1;
    QCOMPARE(msg.channelNumber(), channelNumber);
    int messageId = static_cast<int>(msg.messageId());
    QCOMPARE(messageId, 0x46);
    int actualMessageCode = static_cast<int>(msg.messageCode());
    int expectedMessageCode = static_cast<int>(AntChannelEventMessage::MessageCode::CHANNEL_IN_WRONG_STATE);
    QCOMPARE(actualMessageCode, expectedMessageCode);
}

void AntMessage2Test::channelEventNoErrorUsingFactory()
{
    QByteArray bytes = QByteArray::fromHex(QByteArray::fromRawData("a40340014615a0", 14));
    std::unique_ptr<AntMessage2> msg = AntMessage2::createMessageFromBytes(bytes);
    QVERIFY2(msg, "messages should not be null");
    QCOMPARE(msg->id(), AntMessage2::AntMessageId::CHANNEL_EVENT);

    const AntChannelEventMessage* antChannelEventMessage = msg->asChannelEventMessage();
    quint8 channelNumber = 1;
    QCOMPARE(antChannelEventMessage->channelNumber(), channelNumber);
    int messageId = static_cast<int>(antChannelEventMessage->messageId());
    QCOMPARE(messageId, 0x46);
    int actualMessageCode = static_cast<int>(antChannelEventMessage->messageCode());
    int expectedMessageCode = static_cast<int>(AntChannelEventMessage::MessageCode::CHANNEL_IN_WRONG_STATE);
    QCOMPARE(actualMessageCode, expectedMessageCode);

}
