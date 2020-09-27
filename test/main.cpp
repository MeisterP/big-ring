#include "antmessage2test.h"
#include "distanceentrycollectiontest.h"
#include "profiletest.h"
#include "reallifevideocachetest.h"
#include "ridefilewritertest.h"
#include "rollingaveragecalculatortest.h"
#include "virtualtrainingfileparsertest.h"
#include "virtualpowertest.h"

#include <QTest>

template <typename T>
void execTest()
{
    T test;
    if (QTest::qExec(&test) > 0) {
        std::exit(1);
    }
}

int main(int, char**) {
    execTest<AntMessage2Test>();
    execTest<VirtualPowerTest>();
    execTest<ProfileTest>();
    execTest<RollingAverageCalculatorTest>();
    execTest<RideFileWriterTest>();
    execTest<RealLifeVideoCacheTest>();
    execTest<DistanceEntryCollectionTest>();
    execTest<VirtualTrainingFileParserTest>();
}
