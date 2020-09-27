#include "framecopyingvideoreader.h"

#include <cstring>

#include <QtCore/QCoreApplication>
#include <QtCore/QSize>
#include <QtCore/QtDebug>
#include <QtCore/QTime>
#include <QtGui/QImage>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "model/reallifevideo.h"

namespace {
QEvent::Type OpenVideoFileEventType = static_cast<QEvent::Type>(QEvent::User + 103);
QEvent::Type CopyNextFrameEventType = static_cast<QEvent::Type>(QEvent::User + 104);
QEvent::Type SeekEventType = static_cast<QEvent::Type>(QEvent::User + 105);

class OpenVideoFileEvent: public QEvent
{
public:
    OpenVideoFileEvent(const QString& videoFilename):
        QEvent(OpenVideoFileEventType), _videoFilename(videoFilename)
    {
        // empty
    }

    QString _videoFilename;
};

class CopyNextFrameEvent: public QEvent
{
public:
    CopyNextFrameEvent(const std::weak_ptr<FrameBuffer> &frameBuffer, int skipFrames):
        QEvent(CopyNextFrameEventType), _frameBuffer(frameBuffer), _skipFrames(skipFrames)
    {
        // empty
    }

    std::weak_ptr<FrameBuffer> _frameBuffer;
    int _skipFrames;
};

class SeekEvent: public QEvent
{
public:
    SeekEvent(const qint64 frameNumber):
        QEvent(SeekEventType), _frameNumber(frameNumber)
    {
        // empty
    }

    qint64 _frameNumber;
};
}

FrameCopyingVideoReader::FrameCopyingVideoReader(QObject *parent) :
    GenericVideoReader(parent), _currentFrameNumber(0)
{
    // empty
}

FrameCopyingVideoReader::~FrameCopyingVideoReader()
{
    qDebug() << "closing VideoReader2";
}

void FrameCopyingVideoReader::copyNextFrame(const std::weak_ptr<FrameBuffer> &buffer, int skipFrames)
{
    QCoreApplication::postEvent(this, new CopyNextFrameEvent(buffer, skipFrames));
}

void FrameCopyingVideoReader::seekToFrame(qint64 frameNumber)
{
    QCoreApplication::postEvent(this, new SeekEvent(frameNumber));
}

void FrameCopyingVideoReader::openVideoFile(const QString &videoFilename)
{
    QCoreApplication::postEvent(this, new OpenVideoFileEvent(videoFilename));
}

void FrameCopyingVideoReader::openVideoFileInternal(const QString &videoFilename)
{
    GenericVideoReader::openVideoFileInternal(videoFilename);

    _currentFrameNumber = 0;
    QSize frameSize = totalFrameSize();
    emit videoOpened(videoFilename, QSize(codecContext()->width, codecContext()->height),
                     frameSize, totalNumberOfFrames());
}

void FrameCopyingVideoReader::copyNextFrameInternal(std::weak_ptr<FrameBuffer> &buffer, int skipFrames)
{
    AVFrame* frame = frameYuv().frame;
    int frameBufferIndex = -1;
    QSize requestFrameSize;
    if (auto locked = buffer.lock()) {
        locked->withMutex([this, frame, &frameBufferIndex, &requestFrameSize] (void *ptr, const QSize& frameSize, int index) {
            Q_ASSERT_X(ptr, "copyNextFrameInternal", "ptr should always be non-null here");
            if (ptr) {
                frameBufferIndex = index;
                requestFrameSize = frameSize;
                quint8* bufferPointer = reinterpret_cast<quint8*>(ptr);
                quint32 offset = 0;
                quint32 ysize = frame->linesize[0] * frame->height;
                std::memcpy(bufferPointer, frame->data[0], ysize);
                offset += ysize;
                quint32 usize = frame->linesize[1] * frame->height / 2;
                std::memcpy(bufferPointer + offset, frame->data[1], usize);
                offset += usize;
                quint32 vsize = usize;
                std::memcpy(bufferPointer + offset, frame->data[2], vsize);
            }
        });
    } else {
        qDebug() << "unable to lock weak_ptr, skipFrames =" << skipFrames;
    }
    if (frameBufferIndex >= 0) {
        emit frameCopied(frameBufferIndex, _currentFrameNumber, requestFrameSize);
    }
    // skip frames. We still have to decode these frames, but they won't be saved in _frame for
    // copying to video memory. This is used when the frame rate requested is higher than
    // the normal frame rate of the video.
    for (int i = 0; i < skipFrames; ++i) {
        _currentFrameNumber = loadNextFrame();
    }
    _currentFrameNumber = loadNextFrame();
}

void FrameCopyingVideoReader::seekToFrameInternal(const qint64 frameNumber)
{
    performSeek(frameNumber);
    loadFramesUntilTargetFrame(frameNumber);
}

QSize FrameCopyingVideoReader::totalFrameSize()
{
    _currentFrameNumber = loadNextFrame();
    AVFrame *frame = frameYuv().frame;
    Q_ASSERT_X(frame->linesize[1] = frame->linesize[0] / 2, __FUNCTION__,
            "linesize of U plane should be half of Y plane.");
    return QSize(frame->linesize[0], frame->height);
}

bool FrameCopyingVideoReader::event(QEvent *event)
{
    if (event->type() == OpenVideoFileEventType) {
        OpenVideoFileEvent* openVideoFileEvent = dynamic_cast<OpenVideoFileEvent*>(event);
        openVideoFileInternal(openVideoFileEvent->_videoFilename);
        _currentFrameNumber = loadNextFrame();
        return true;
    } else if (event->type() == CopyNextFrameEventType) {
        CopyNextFrameEvent* copyNextFrameEvent = dynamic_cast<CopyNextFrameEvent*>(event);
        copyNextFrameInternal(copyNextFrameEvent->_frameBuffer, copyNextFrameEvent->_skipFrames);
        return true;
    } else if (event->type() == SeekEventType) {
        seekToFrameInternal(dynamic_cast<SeekEvent*>(event)->_frameNumber);
        return true;
    }
    return GenericVideoReader::event(event);
}
