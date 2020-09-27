#ifndef GENERICVIDEOREADER_H
#define GENERICVIDEOREADER_H

#include <memory>
#include <QtCore/QObject>

struct AVCodec;
struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;
struct AVPicture;
struct AVStream;

/** Simple wrapper around an AVFrame struct. */
class AVFrameWrapper {
public:
    AVFrameWrapper();
    ~AVFrameWrapper();

    /** return the AVFrame as an AVPicture. */
    AVPicture* asPicture();

    bool isEmpty();

    AVFrame* frame;
};

class GenericVideoReader : public QObject
{
    Q_OBJECT
public:
    explicit GenericVideoReader(QObject *parent = 0);
    virtual ~GenericVideoReader();

signals:
    void error(const QString& errorMessage);
    void seekReady(qint64 frameNumber);

public slots:

protected:
    virtual void openVideoFileInternal(const QString &videoFilename);
    void performSeek(qint64 targetFrameNumber);
    void loadFramesUntilTargetFrame(qint64 targetFrameNumber);
    qint64 loadNextFrame();
    qint64 totalNumberOfFrames();

    AVCodecContext *codecContext() const;
    AVFormatContext *formatContext() const;
    AVFrameWrapper &frameYuv() const;
    const AVStream *videoStream() const;
private:
    void initialize();
    void close();
    void printError(int errorNumber, const QString& message);
    void printError(const QString &message);
    int findVideoStream(AVFormatContext* formatContext) const;
    qint64 frameNumberToTimestamp(const qint64 frameNumber) const;
    qint64 timestampToFrameNumber(const qint64 timestamp) const;

    bool _initialized = false;
    // libav specific data
    AVCodec* _codec = nullptr;
    AVCodecContext* _codecContext = nullptr;
    AVFormatContext* _formatContext = nullptr;
    std::unique_ptr<AVFrameWrapper> _frameYuv;
    int _currentVideoStream;
    AVStream* _videoStream = nullptr;
};

#endif // GENERICVIDEOREADER_H
