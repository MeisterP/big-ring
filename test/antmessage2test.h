#ifndef ANTMESSAGE2TEST_H
#define ANTMESSAGE2TEST_H

#include <QtCore/QObject>

#include "ant/antmessage2.h"
class AntMessage2Test : public QObject
{
    Q_OBJECT
private slots:
    void systemReset();
    void setNetworkKey();
    void unassignChannel();
    void assignChannel();
    void openChannel();
    void requestMessage();
    void setChannelId();
    void setSearchTimeout();
    void setChannelFrequency();
    void setChannelPeriod();
    void channelEventNoError();

    // factory
    void channelEventNoErrorUsingFactory();
};

#endif // ANTMESSAGE2TEST_H
