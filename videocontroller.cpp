#include "videocontroller.h"

#include <QtDebug>
#include <QDateTime>
#include <QTimer>

namespace {

const float videoUpdateInterval = 100; // ms
const quint32 NR_FRAMES_PER_REQUEST = 25;
const int NR_FRAMES_BUFFER_LOW = 40;
}

VideoController::VideoController(Cyclist &cyclist, VideoWidget* videoWidget, QObject *parent) :
	QObject(parent),
	_cyclist(cyclist),
	_videoWidget(videoWidget),
	_requestBusy(false),
	_lastFrameRateSample(QDateTime::currentDateTime()),
	_currentFrameRate(0u)
{
	_decoderThread.start();
	_videoDecoder.moveToThread(&_decoderThread);

	// set up timers
	_playTimer.setInterval(40);
	connect(&_playTimer, SIGNAL(timeout()), SLOT(playNextFrame()));

	// set up video decoder
	connect(&_videoDecoder, SIGNAL(videoLoaded()), SLOT(videoLoaded()));
	connect(&_videoDecoder, SIGNAL(framesReady(FrameList)), SLOT(framesReady(FrameList)));
	connect(&_videoDecoder, SIGNAL(seekFinished(Frame)), SLOT(seekFinished(Frame)));

	connect(this, SIGNAL(currentFrameRate(quint32)), _videoWidget, SLOT(setFrameRate(quint32)));
}

VideoController::~VideoController()
{
	_decoderThread.quit();
	_decoderThread.wait(1000); // wait for a maximum of 1 second.
}

bool VideoController::isBufferFull()
{
	return !_imageQueue.empty();
}

void VideoController::realLiveVideoSelected(RealLiveVideo rlv)
{
	reset();
	_currentRlv = rlv;
	if (!_currentRlv.videoInformation().videoFilename().isEmpty())
		loadVideo(_currentRlv.videoInformation().videoFilename());
}

void VideoController::courseSelected(int courseNr)
{
	reset();

	if (courseNr == -1) {
		return;
	}

	if (!_currentRlv.isValid())
		return;

	quint32 frame = _currentRlv.frameForDistance(_cyclist.distance());

	setPosition(frame);
}

void VideoController::play(bool doPlay)
{
	if (doPlay) {
		_playTimer.start();
	} else {
		_playTimer.stop();
	}
	emit playing(_playTimer.isActive());
}

void VideoController::videoLoaded()
{
	qDebug() << "video loaded";
}

void VideoController::playNextFrame()
{
	// keep track of frame rate
	QDateTime now = QDateTime::currentDateTime();
	if (_lastFrameRateSample.addSecs(1) < now) {
		_currentFrameRate = _framesThisSecond;
		_framesThisSecond = 0;
		_lastFrameRateSample = now;
		emit(currentFrameRate(_currentFrameRate));
	}

	quint32 frameToShow = _currentRlv.frameForDistance(_cyclist.distance());
	displayFrame(frameToShow);
}

void VideoController::displayFrame(quint32 frameToShow)
{
	if (frameToShow == _currentFrameNumber)
		return; // no need to display again.
	if (frameToShow < _currentFrameNumber && _currentFrameNumber != UNKNOWN_FRAME_NR) {
		qDebug() << "frame to show" << frameToShow << "current" << _currentFrameNumber;
		return; // wait until playing catches up.
	}

	if (_imageQueue.empty()) {
		qDebug() << "image queue empty, doing nothing";
		return;
	}

	Frame frame;

	if (_currentFrameNumber == UNKNOWN_FRAME_NR) {
		qDebug() << "current frame is null, taking first one";
		frame = takeFrame();
		_currentFrameNumber = frame.first;
		_videoWidget->displayFrame(frame.first, frame.second);
		return;
	}

	bool displayed = false;
	while(!displayed && _currentFrameNumber != UNKNOWN_FRAME_NR && !_imageQueue.empty()) {
		frame = takeFrame();
		_framesThisSecond += frame.first - _currentFrameNumber;
		_currentFrameNumber = frame.first;
		if (_currentFrameNumber == frameToShow) {
			_videoWidget->displayFrame(frame.first, frame.second);
			displayed = true;
		}
	}
}

void VideoController::framesReady(FrameList frames)
{
	_requestBusy = false;
	if (_currentFrameNumber == UNKNOWN_FRAME_NR) {
		_currentFrameNumber = 1;
	}

	if (frames.first().first >= _currentFrameNumber) {
		_imageQueue += frames;

		if (_imageQueue.size() >= NR_FRAMES_BUFFER_LOW)
			emit bufferFull(true);
	}

	// check if we need some new frames ASAP
	if (_imageQueue.size() <= NR_FRAMES_BUFFER_LOW)
		requestNewFrames(NR_FRAMES_PER_REQUEST);
}

void VideoController::seekFinished(Frame frame)
{
	_imageQueue.clear();
	_imageQueue.append(frame);
	displayFrame(frame.first);
}

void VideoController::loadVideo(const QString &filename)
{
	QMetaObject::invokeMethod(&_videoDecoder, "openFile",
							  Q_ARG(QString, filename));
}

void VideoController::setPosition(quint32 frameNr)
{
	QMetaObject::invokeMethod(&_videoDecoder, "seekFrame",
							  Q_ARG(quint32, frameNr));
}

void VideoController::reset()
{
	play(false);
	emit bufferFull(false);
	_playTimer.stop();
	_currentFrameNumber = UNKNOWN_FRAME_NR;
	_imageQueue.clear();
	_requestBusy = false;
}

Frame VideoController::takeFrame()
{
	if (!_requestBusy && _imageQueue.size() <= NR_FRAMES_BUFFER_LOW) {
		requestNewFrames(NR_FRAMES_PER_REQUEST);
	}

	if (_imageQueue.empty())
		return qMakePair(UNKNOWN_FRAME_NR, QImage());

	return _imageQueue.takeFirst();
}

void VideoController::requestNewFrames(quint32 numberOfFrames)
{
	_requestBusy = true;

	// when framerate is > 30, skip every second frame.
	int skip = (_currentFrameRate > 30) ? 2: 1;
	QMetaObject::invokeMethod(&_videoDecoder, "loadFrames", Q_ARG(quint32, numberOfFrames), Q_ARG(quint32, skip));
}
