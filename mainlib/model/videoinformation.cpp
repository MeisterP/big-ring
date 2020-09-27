#include "videoinformation.h"

VideoInformation::VideoInformation(const QString &videoFilename, float frameRate):
    _videoFilename(videoFilename), _frameRate(frameRate)
{
    // empty
}

VideoInformation::VideoInformation():
    VideoInformation(QString(), 0)
{
    // empty
}

