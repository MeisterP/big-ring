#include <gtest/gtest.h>

#include "reallifevideo.h"

class RealLifeVideoTest: public ::testing::Test
 {
public:

	void testProfileEntryDefault();
	void testProfileEntry();

	void testProfileDefault();
	void testProfile();

	void testVideoInformationDefault();
	void testVideoInformation();
};

TEST_F(RealLifeVideoTest, testProfileEntryDefault)
{
	ProfileEntry entry;
	EXPECT_FLOAT_EQ(0, entry.distance());
	EXPECT_FLOAT_EQ(0, entry.slope());
}

TEST_F(RealLifeVideoTest, testProfileEntry)
{
	ProfileEntry entry(1.23f, 0, 8.23f, .25f);
	EXPECT_FLOAT_EQ(1.23f, entry.distance());
	EXPECT_FLOAT_EQ(8.23f, entry.slope());
	EXPECT_FLOAT_EQ(0, entry.totalDistance());
	EXPECT_FLOAT_EQ(.25f, entry.altitude());
}

TEST_F(RealLifeVideoTest, testProfileDefault)
{
	Profile profile;

	EXPECT_FLOAT_EQ(0, profile.startAltitude());
	EXPECT_FLOAT_EQ(0, profile.slopeForDistance(0.0));
	EXPECT_FLOAT_EQ(0, profile.slopeForDistance(10.0));
}

TEST_F(RealLifeVideoTest, testProfile)
{
	QList<ProfileEntry> entries;
	entries << ProfileEntry(1.0, 0.0, 1.0, 0.0);
	entries << ProfileEntry(1.0, 1.0, 2.0, 0.01);
	Profile profile(SLOPE, 12, entries);

	EXPECT_FLOAT_EQ(12.0f, profile.startAltitude());

	EXPECT_FLOAT_EQ(2.0f, profile.slopeForDistance(1.1));
	EXPECT_FLOAT_EQ( 1.0f,profile.slopeForDistance(0.5));

	EXPECT_FLOAT_EQ(12.0f, profile.altitudeForDistance(0.0f));
	EXPECT_FLOAT_EQ(12.01f, profile.altitudeForDistance(1.0f));
	EXPECT_FLOAT_EQ(12 + 0.01f + 0.02f, profile.altitudeForDistance(2.0f));
	EXPECT_FLOAT_EQ(12 + 0.01f + 0.01f, profile.altitudeForDistance(1.5f));
}

TEST_F(RealLifeVideoTest, testVideoInformationDefault)
{
	VideoInformation info;
	EXPECT_FLOAT_EQ(0.0f, info.frameRate());
	EXPECT_EQ(QString(""), info.videoFilename());
}

TEST_F(RealLifeVideoTest, testVideoInformation)
{
	VideoInformation info("VideoFile", 29.97f);
	EXPECT_FLOAT_EQ(29.97f, info.frameRate());
	EXPECT_EQ(QString("VideoFile"), info.videoFilename());
}

