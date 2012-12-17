#ifndef VIDEODECODER_H
#define VIDEODECODER_H

#include <QObject>
#include <QImage>
#include <QMutex>

struct AVCodec;
struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;
struct SwsContext;

/**
  Pure virtual class which can be implemented by classes which want to handle
  images from the video.
  */
class VideoImageHandler
{
public:
	virtual void handleImage(const QImage& image) = 0;
};

class VideoDecoder : public QObject
{
	Q_OBJECT
public:
	explicit VideoDecoder(QObject *parent = 0);
	~VideoDecoder();

	void doWithImage(VideoImageHandler& handler);
signals:
	void frameReady(quint32 frameNr);
	void error();
	void videoLoaded();
public slots:
	void seekFrame(quint32 frameNr);
	void openFile(QString filename);
	void nextImage();

private slots:
	void seekDelayFinished();
private:
	void close();
	void closeFramesAndBuffers();
	void initialize();
	void initializeFrames();
	void decodeNextFrame();

	int findVideoStream();
	void printError(int errorNr, const QString& message);

	AVFormatContext* _formatContext;
	AVCodecContext* _codecContext;
	AVCodec* _codec;
	AVFrame* _frame;
	AVFrame* _frameRgb;
	int _bufferSize;
	quint8 *_frameBuffer;
	SwsContext* _swsContext;
	QImage _currentImage;

	// seeking
	QTimer* _seekTimer;
	quint32 _seekTargetFrame;


	int _videoStream;
	QMutex _mutex;

	qint32 _currentFrame;
	int _widgetWidth;
	int _widgetHeight;
};

#endif // VIDEODECODER_H
