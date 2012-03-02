#include "reallivevideo.h"

#include <QtDebug>

RealLiveVideo::RealLiveVideo(const QString& name, const VideoInformation& videoInformation):
        _name(name), _videoInformation(videoInformation)
{
}

RealLiveVideo::RealLiveVideo() {}

VideoInformation::VideoInformation(const QString &videoFilename, float frameRate):
        _videoFilename(videoFilename), _frameRate(frameRate)
{
}

VideoInformation::VideoInformation():
    _frameRate(0.0) {}


