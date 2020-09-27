#include <gtest/gtest.h>
#include "antmessagegatherer.h"
#include "ANT.h"
#include <QtTest/QTestEventLoop>
#include <QtTest/QSignalSpy>

class AntMessageGathererTest: public ::testing::Test
{
public:
	AntMessageGathererTest():
		spy(&gatherer, SIGNAL(antMessageReceived(QByteArray))) {}

	AntMessageGatherer gatherer;
	QSignalSpy spy;
};

QByteArray oneByteMessage(int payload = 0)
{
	QByteArray bytes;
	bytes.append((char) ANT_SYNC_BYTE);
	bytes.append((char) 1);
	bytes.append((char) ANT_OPEN_CHANNEL);
	bytes.append((char) payload);

	quint8 checksum = ANT_SYNC_BYTE ^ 1 ^ ANT_OPEN_CHANNEL;

	bytes.append(checksum ^ payload);

	return bytes;
}

TEST_F(AntMessageGathererTest, noData)
{
	gatherer.submitBytes(QByteArray());

	EXPECT_EQ(0, spy.count());
}

TEST_F(AntMessageGathererTest, oneByteMessage)
{
	QByteArray bytes = oneByteMessage();
	gatherer.submitBytes(bytes);

	ASSERT_EQ(1, spy.count());

	QByteArray messageBytes = spy.takeFirst()[0].toByteArray();
	EXPECT_EQ(bytes, messageBytes);
}

TEST_F(AntMessageGathererTest, syncByteAsSecondByte)
{
	QByteArray bytes;
	bytes.append((char) ANT_OPEN_CHANNEL);
	bytes.append(oneByteMessage());

	gatherer.submitBytes(bytes);

	ASSERT_EQ(1, spy.count());
	QByteArray messageBytes = spy.takeFirst()[0].toByteArray();
	EXPECT_EQ(oneByteMessage(), messageBytes);
}

TEST_F(AntMessageGathererTest, twoMessages)
{
	QByteArray bytes;
	bytes.append(oneByteMessage());
	bytes.append(oneByteMessage(1));

	gatherer.submitBytes(bytes);

	ASSERT_EQ(2, spy.count());
	QByteArray messageBytes = spy.takeFirst()[0].toByteArray();
	EXPECT_EQ(oneByteMessage(), messageBytes);
	messageBytes = spy.takeFirst()[0].toByteArray();
	EXPECT_EQ(oneByteMessage(1), messageBytes);
}

/**
 * Test what happens when a message is said to be N bytes, but is actually shorter.
 */
TEST_F(AntMessageGathererTest, messageEndLost)
{
	QByteArray bytes = oneByteMessage();
	bytes.remove(bytes.length() - 2, 1);

	gatherer.submitBytes(bytes);

	ASSERT_EQ(0, spy.count());
}

/**
 * Test what happens when a message is not submitted all at once, but in two parts.
 */
TEST_F(AntMessageGathererTest, messageSubmittedInTwoParts)
{
	QByteArray part1 = oneByteMessage().left(2);
	QByteArray part2 = oneByteMessage().mid(2);

	gatherer.submitBytes(part1);
	gatherer.submitBytes(part2);

	ASSERT_EQ(1, spy.count());
	EXPECT_EQ(oneByteMessage(), spy.takeFirst()[0].toByteArray());
}

TEST_F(AntMessageGathererTest, messageWithWrongChecksumShouldNotBeEmitted)
{
	QByteArray bytes = oneByteMessage();
	bytes[bytes.length() - 1] = 0; // wrong checksum in last byte.

	gatherer.submitBytes(bytes);

	ASSERT_EQ(0, spy.count());
}

TEST_F(AntMessageGathererTest, messageAfterMessageWithWrongChecksumIsOk)
{
	QByteArray bytes = oneByteMessage();
	bytes[bytes.length() - 1] = 0; // wrong checksum in last byte.
	QByteArray bytes2 = oneByteMessage();
	gatherer.submitBytes(bytes + bytes2);

	ASSERT_EQ(1, spy.count());
	EXPECT_EQ(oneByteMessage(), spy.takeFirst()[0].toByteArray());
}
