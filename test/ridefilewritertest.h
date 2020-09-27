#ifndef RIDEFILEWRITERTEST_H
#define RIDEFILEWRITERTEST_H

#include <QtCore/QObject>

class RideFileWriterTest : public QObject
{
    Q_OBJECT
public:
    explicit RideFileWriterTest(QObject *parent = 0);

private slots:
    void testWriteSimpleRideFile();
};

#endif // RIDEFILEWRITERTEST_H
