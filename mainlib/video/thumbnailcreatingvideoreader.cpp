#include "thumbnailcreatingvideoreader.h"

#include <cstring>

#include <QtCore/QCoreApplication>
#include <QtCore/QSize>
#include <QtCore/QtDebug>
#include <QtGui/QImage>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include "libswscale/swscale.h"
}

#include "model/reallifevideo.h"

namespace {
QEvent::Type CreateImageForFrameEventType = static_cast<QEvent::Type>(QEvent::User + 102);

class CreateImageForFrameEvent: public QEvent
{
public:
    CreateImageForFrameEvent(const RealLifeVideo& rlv, qreal distance):
        QEvent(CreateImageForFrameEventType), _rlv(rlv), _distance(distance)
    {
        // empty
    }

    RealLifeVideo _rlv;
    qreal _distance;
};
}

ThumbnailCreatingVideoReader::ThumbnailCreatingVideoReader(QObject *parent) :
    GenericVideoReader(parent)
{
    // empty
}

ThumbnailCreatingVideoReader::~ThumbnailCreatingVideoReader()
{
    qDebug() << "closing ThumbnailCreatingVideoReader";
}

void ThumbnailCreatingVideoReader::createImageForFrame(const RealLifeVideo& rlv, const qreal distance)
{
    QCoreApplication::postEvent(this, new CreateImageForFrameEvent(rlv, distance));
}

void ThumbnailCreatingVideoReader::openVideoFileInternal(const QString &videoFilename)
{
    GenericVideoReader::openVideoFileInternal(videoFilename);

    _frameRgb.reset(new AVFrameWrapper);
    int numBytes= avpicture_get_size(PIX_FMT_RGB24,
          codecContext()->width, codecContext()->height);
    _imageBuffer.resize(numBytes);
    avpicture_fill(_frameRgb->asPicture(), reinterpret_cast<uint8_t*>(_imageBuffer.data()), PIX_FMT_RGB24,
                   codecContext()->width, codecContext()->height);

    _swsContext = sws_getContext(codecContext()->width, codecContext()->height, AV_PIX_FMT_YUV420P,
                                 codecContext()->width, codecContext()->height, AV_PIX_FMT_RGB24, SWS_FAST_BILINEAR,
                                 nullptr, nullptr, nullptr);
}

void ThumbnailCreatingVideoReader::createImageForFrameNumber(RealLifeVideo& rlv, const qreal distance)
{
    // the first few frames are sometimes black, so when requested to take a "screenshot" of the first frames, just
    // skip to a few frames after the start.
    qint64 frameNumber = qMax(20, static_cast<int>(rlv.frameForDistance(distance)));
    qDebug() << "creating image for" << rlv.name() << "distance" << distance << "frame nr" << frameNumber;
    performSeek(frameNumber);
    loadFramesUntilTargetFrame(frameNumber);
    emit newFrameReady(rlv, distance, createImage());
}

bool ThumbnailCreatingVideoReader::event(QEvent *event)
{
    if (event->type() == CreateImageForFrameEventType) {
        CreateImageForFrameEvent* createImageForFrameEvent = dynamic_cast<CreateImageForFrameEvent*>(event);

        RealLifeVideo& rlv = createImageForFrameEvent->_rlv;
        const qreal distance = createImageForFrameEvent->_distance;
        qDebug() << "creating thumbnail for rlv" << rlv.name();
        openVideoFileInternal(rlv.videoFilename());

        rlv.setNumberOfFrames(totalNumberOfFrames());
        createImageForFrameNumber(rlv, distance);
        return true;
    }
    return GenericVideoReader::event(event);
}

QImage ThumbnailCreatingVideoReader::createImage()
{
    if (frameYuv().isEmpty()) {
        return QImage();
    }
    sws_scale(_swsContext, frameYuv().frame->data, frameYuv().frame->linesize, 0, codecContext()->height, _frameRgb->frame->data,
              _frameRgb->frame->linesize);
    QImage image(frameYuv().frame->width, frameYuv().frame->height, QImage::Format_RGB888);
    std::memcpy(image.bits(), _frameRgb->frame->data[0], _imageBuffer.size());
    return image;
}
