#include "videoinforeader.h"

extern "C" {
#include "libavformat/avformat.h"
}
VideoInfoReader::VideoInfoReader(QObject *parent) :
    GenericVideoReader(parent)
{
    // empty
}

VideoInfoReader::~VideoInfoReader()
{
    // noop
}

VideoInfo VideoInfoReader::videoInfoForVideo(const QFileInfo &videoFileInfo)
{
    openVideoFileInternal(videoFileInfo.filePath());
    const AVStream* stream = videoStream();
    VideoInfo videoInfo = { static_cast<float>(av_q2d(stream->avg_frame_rate)) };
    return videoInfo;
}
