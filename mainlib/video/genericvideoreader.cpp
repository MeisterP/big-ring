#include "genericvideoreader.h"

#include <array>

#include <QtCore/QSize>
#include <QtCore/QtDebug>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

namespace {
const int ERROR_STR_BUF_SIZE = 128;
}
GenericVideoReader::GenericVideoReader(QObject *parent) :
    QObject(parent)
{
}

GenericVideoReader::~GenericVideoReader()
{
    qDebug() << "closing GenericVideoreader";
    close();
}

void GenericVideoReader::initialize()
{
    if (!_initialized) {
        avcodec_register_all();
        av_register_all();
        _initialized = true;
    }
}

void GenericVideoReader::close()
{
    _videoStream = nullptr;
    if (_codecContext) {
        avcodec_close(_codecContext);
    }
    if (_formatContext) {
        avformat_close_input(&_formatContext);
    }
}

/**
 * This will load all frame until we encounter the frame with the target frame number.
 * @param targetFrameNumber the number of the target frame
 */
void GenericVideoReader::loadFramesUntilTargetFrame(qint64 targetFrameNumber)
{
    bool seekAgain;
    bool extraSeekDone = false;
    qint64 currentFrameNumber = -1;
    do {
        seekAgain = false;
        currentFrameNumber = loadNextFrame();

        // special case: Some videos, probably those that do not start with a keyframe,
        // will exhibit faulty seek behaviour. After a seek, they end up *after* the
        // targetted frame. In that case, we do another seek to a frame number
        // some frames before the targetted frame to hopefully get a keyframe there.
        // We only do this once to prevent us from causing an endless loop here.
        if (!extraSeekDone && currentFrameNumber > 100 && currentFrameNumber > targetFrameNumber) {
            performSeek(targetFrameNumber - 500);
            seekAgain = true;
            extraSeekDone = true;
        }

    } while (seekAgain || (currentFrameNumber >= 0 && currentFrameNumber < targetFrameNumber));
    emit seekReady(currentFrameNumber);
}

void GenericVideoReader::printError(int errorNumber, const QString &message)
{
    std::array<char, ERROR_STR_BUF_SIZE> errorString;
    av_strerror(errorNumber, errorString.data(), ERROR_STR_BUF_SIZE);

    QString completeMessage = QString("%1 : %2").arg(message).arg(errorString.data());
    printError(completeMessage);
}

void GenericVideoReader::printError(const QString &message)
{
    qWarning("%s", qPrintable(message));
    emit error(message);
}

void GenericVideoReader::performSeek(qint64 targetFrameNumber)
{
    qDebug() << "seeking to" << targetFrameNumber;
    AVStream* videoStream = formatContext()->streams[_currentVideoStream];
    double timeBase = av_q2d(videoStream->time_base);
    double framerate = av_q2d(videoStream->avg_frame_rate);

    qint64  ts = targetFrameNumber / (timeBase * framerate);
    av_seek_frame(formatContext(), _currentVideoStream, ts,
                  AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);
        avcodec_flush_buffers(codecContext());
}

qint64 GenericVideoReader::loadNextFrame()
{
    AVPacket packet;
    int frameFinished = 0;
    while (!frameFinished) {
        if (av_read_frame(formatContext(), &packet) < 0) {
            qDebug() << "end of file reached";
            return -1;
        }
        if (packet.stream_index == _currentVideoStream) {
            avcodec_decode_video2(codecContext(), _frameYuv->frame,
                                  &frameFinished, &packet);
        }
    }

    qint64 currentFrameNumber;
    qint64 pts = packet.pts;
    if (pts == static_cast<qint64>(AV_NOPTS_VALUE)) {
        currentFrameNumber = packet.dts;
    } else {
        currentFrameNumber = timestampToFrameNumber(pts);
    }

    av_free_packet(&packet);
    return currentFrameNumber;
}

qint64 GenericVideoReader::totalNumberOfFrames()
{
    AVStream* videoStream = formatContext()->streams[_currentVideoStream];
    return timestampToFrameNumber(videoStream->duration);
}

void GenericVideoReader::openVideoFileInternal(const QString &videoFilename)
{
    initialize();
    if (_formatContext && QString(_formatContext->filename) == videoFilename) {
        return;
    }
    close();
    int errorNr = avformat_open_input(&_formatContext, videoFilename.toStdString().c_str(),
                                                                              NULL, NULL);
    if (errorNr != 0) {
        printError(errorNr, QString("Unable to open %1").arg(videoFilename));
    }
    errorNr = avformat_find_stream_info(_formatContext, NULL);
    if (errorNr < 0) {
        printError(errorNr, QString("Unable to find video stream"));
    }
    av_dump_format(_formatContext, 0, videoFilename.toStdString().c_str(), 0);

    _currentVideoStream = findVideoStream(_formatContext);
    if (_currentVideoStream < 0) {
        printError("Unable to find video stream");
    }
    _videoStream = _formatContext->streams[_currentVideoStream];

    _codecContext = _videoStream->codec;
    if (!_codecContext) {
        printError("Unable to open codec context");
    }

    _codec = avcodec_find_decoder(_codecContext->codec_id);
    if (!_codec) {
        printError("Unable to find codec");
    }

    errorNr = avcodec_open2(_codecContext, _codec, NULL);
    if (errorNr < 0) {
        printError(errorNr, "Unable to open codec");
    }

    _frameYuv.reset(new AVFrameWrapper);
}

/**
 * @brief GenericVideoReader::findVideoStream find the video stream amongst a number of streams in a format context
 * @param formatContext the format context for a video file
 * @return the number of the video stream or -1 or no video stream was found.
 */
int GenericVideoReader::findVideoStream(AVFormatContext *formatContext) const
{
    for (quint32 i = 0; i < formatContext->nb_streams; ++i) {
        if (formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

qint64 GenericVideoReader::frameNumberToTimestamp(const qint64 frameNumber) const
{
    return frameNumber / av_q2d(av_mul_q(_videoStream->time_base, _videoStream->avg_frame_rate));
}

qint64 GenericVideoReader::timestampToFrameNumber(const qint64 timestamp) const
{
    return timestamp * av_q2d(av_mul_q(_videoStream->time_base, _videoStream->avg_frame_rate));
}

AVCodecContext *GenericVideoReader::codecContext() const
{
    return _codecContext;
}

AVFormatContext *GenericVideoReader::formatContext() const
{
    return _formatContext;
}

AVFrameWrapper &GenericVideoReader::frameYuv() const
{
    return *_frameYuv;
}

const AVStream *GenericVideoReader::videoStream() const
{
    Q_ASSERT(_videoStream != nullptr);
    return _videoStream;
}

AVFrameWrapper::AVFrameWrapper()
{
    frame = av_frame_alloc();
}

AVFrameWrapper::~AVFrameWrapper()
{
    av_frame_free(&frame);
}

AVPicture *AVFrameWrapper::asPicture()
{
    return reinterpret_cast<AVPicture*>(frame);
}

bool AVFrameWrapper::isEmpty()
{
    return frame->width == 0 || frame->height == 0;
}

