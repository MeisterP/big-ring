#ifndef VIDEOREADER_H
#define VIDEOREADER_H

#include "genericvideoreader.h"
#include <memory>
#include <QtCore/QEvent>
#include <QtCore/QObject>

class RealLifeVideo;

struct SwsContext;

class ThumbnailCreatingVideoReader : public GenericVideoReader
{
    Q_OBJECT
public:
    explicit ThumbnailCreatingVideoReader(QObject *parent = 0);
    virtual ~ThumbnailCreatingVideoReader();

    void createImageForFrame(const RealLifeVideo& rlv, const qreal distance);
signals:
    void newFrameReady(const RealLifeVideo& rlv, qreal distance, const QImage& frame);

protected:
    virtual bool event(QEvent *) override;
private:
    virtual void openVideoFileInternal(const QString& videoFilename) override;

    void createImageForFrameNumber(RealLifeVideo &rlv, const qreal distance);
    QImage createImage();

    bool _initialized;
    // libav specific data
    std::unique_ptr<AVFrameWrapper> _frameRgb;
    SwsContext* _swsContext;
    QByteArray _imageBuffer;
};

#endif // VIDEOREADER_H
