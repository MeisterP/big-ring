/*
 * Copyright (c) 2012-2015 Ilja Booij (ibooij@gmail.com)
 *
 * This file is part of Big Ring Indoor Video Cycling
 *
 * Big Ring Indoor Video Cycling is free software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Big Ring Indoor Video Cycling  is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Big Ring Indoor Video Cycling.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <memory>
#include <QObject>
#include <QtCore/QTimer>
#include <QtOpenGL/QGLContext>

class FrameBuffer;
class OpenGLPainter2;
class FrameCopyingVideoReader;

/*!
 * \brief Video player for cycling videos. This is a frame based player, so clients can seek to
 * a particular frame and choose to step to a particular frame number.
 *
 * This video player will paint the frames from a video file onto a QGLWidget. A frame is displayed
 * using the displayCurrentFrame method.
 */
class VideoPlayer : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief construct a VideoPlayer using the \param paintWidget as the surface to paint to. Optionally,
     * set the \param parent to help disposing this element when no longer needed.
     */
    explicit VideoPlayer(QGLWidget *paintWidget, QObject *parent = 0);

    /*!
     * \brief destroy the videoplayer by cleaning up all internal resources.
     */
    ~VideoPlayer();

    /*!
     * \brief check if the player is ready to play or step.
     * \return true if ready to play, false otherwise.
     */
    bool isReadyToPlay();


signals:
    /*!
     * \brief signal emitted when a video is loaded. \param totalNumberOfFrames will hold the length
     * of the video in frames.
     */
    void videoLoaded(qint64 totalNumberOfFrames);
    /*!
     * \brief seekDone is emitted when a seek action is ready.
     */
    void seekDone();
    /*!
     * \brief updateVideo is emitted when a new frame is ready to be shown. Clients should call displayCurrentFrame after
     * this.
     */
    void updateVideo();

    /**
     * emitted when the frame rate changes
     * @param frameRate the frame rate in frames per second.
     */
    void frameRateChanged(int frameRate);

public slots:
    /*! stop the video */
    void stop();
    /*! step to a \param frameNumber. */
    void stepToFrame(quint32 frameNumber);

    /*! set the \param uri of the video file and load it. */
    void loadVideo(QString uri);

    /*! seek to a certain frame number */
    bool seekToFrame(quint32 frameNumber);

    void displayCurrentFrame(QPainter* painter, QRectF rect, Qt::AspectRatioMode aspectRatioMode);

private slots:
    void setVideoOpened(const QString& videoFilename, const QSize &videoSize, const QSize& frameSize, const qint64 numberOfFrames);

    void setSeekReady(qint64 frameNumber);

    void setFrameLoaded(int index, qint64 frameNumber, const QSize& frameSize);

    void setFrameNeeded(const std::weak_ptr<FrameBuffer> &frameBuffer);

    void determineFrameRate();
private:
    enum class LoadState
    {
        NONE, VIDEO_LOADING, VIDEO_LOADED, SEEKING, DONE, PLAYING
    };

    void updateCurrentFrameNumber(const quint32 frameNumber);
    void updateLoadState(const LoadState loadState);

    OpenGLPainter2* _painter;
    FrameCopyingVideoReader * const _videoReader;
    QThread *_videoReaderThread;

    LoadState _loadState = LoadState::NONE;
    quint32 _currentFrameNumber = 0u;
    quint32 _lastFrameNumber = 0u;
    qint64 _lastFrameLoaded = 0;
    quint32 _stepSize = 1;
    QTimer *_frameRateTimer;
};

#endif // VIDEOPLAYER_H
