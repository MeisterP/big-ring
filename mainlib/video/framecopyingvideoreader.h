#ifndef VIDEOREADER_H
#define VIDEOREADER_H

#include <memory>

#include <QtCore/QEvent>
#include <QtCore/QObject>
#include "genericvideoreader.h"
#include "framebuffer.h"

class RealLifeVideo;
struct AVCodec;
struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;

class FrameCopyingVideoReader : public GenericVideoReader
{
    Q_OBJECT
public:
    explicit FrameCopyingVideoReader(QObject *parent = 0);
    virtual ~FrameCopyingVideoReader();

    void openVideoFile(const QString &videoFilename);
    void copyNextFrame(const std::weak_ptr<FrameBuffer> &buffer, int skipFrames = 0);
    void seekToFrame(qint64 frameNumber);

signals:
    void error(const QString& errorMessage);
    void videoOpened(const QString& videoFilename, const QSize& videoSize,
                     const QSize& totalFrameSize, const qint64 numberOfFrames);
    void frameCopied(int index, qint64 frameNumber, const QSize& frameSize);

protected:
    virtual bool event(QEvent *);
private:
    virtual void openVideoFileInternal(const QString &videoFilename);
    void copyNextFrameInternal(std::weak_ptr<FrameBuffer> &buffer, int skipFrames);
    void seekToFrameInternal(const qint64 frameNumber);
    QSize totalFrameSize();

    qint64 _currentFrameNumber;
};

#endif // VIDEOREADER_H
