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

#ifndef OPENGLPAINTER_H
#define OPENGLPAINTER_H

#include <QObject>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLDebugMessage>
#include <QtGui/QOpenGLShaderProgram>
#include <QtOpenGL/QGLWidget>
#include <QtGui/QOpenGLFunctions_1_3>

#include <array>
#include <memory>
#include "framebuffer.h"

namespace
{
const int NUMBER_OF_BUFFERS = 150;
}
class OpenGLPainter2 : public QObject
{
    Q_OBJECT
public:
    explicit OpenGLPainter2(QGLWidget* widget, QObject *parent = 0);
    virtual ~OpenGLPainter2();

    /** paint the current from using OpenGL */
    void paint(QPainter* painter, const QRectF& rect, Qt::AspectRatioMode aspectRatioMode);
signals:
    /**
     * When this signal is emitted, we need to load another frame for the painter.
     * @param frameBuffer the structure in which to load the frame.
     */
    void frameNeeded(const std::weak_ptr<FrameBuffer>& frameBuffer);
public slots:
    /**
     * Set the video size.
     * @param videoSize the destination size, in which the video should be presented.
     * @param frameSize the source size, coming from the video file.
     */
    void setVideoSize(const QSize& videoSize, const QSize &frameSize);
    /**
     * Signal to the painter that a frame is loaded.
     * @param index internal index in the OpenGLPainter. Get this from the FrameBuffer that has been loaded.
     * @param frameNumber number of the frame, from the video file.
     * @param frameSize size of the frame.
     */
    void setFrameLoaded(int index, qint64 frameNumber, const QSize& frameSize);
    /**
     * Prepare a frame for painting.
     * @param frameNumber frame number of the frame that should be shown later.
     * @return true if a new frame was prepared. False if no new frame has to be shown
     */
    bool showFrame(qint64 frameNumber);
    /**
     * Make the painter fill it's buffers
     */
    void fillBuffers();
private slots:
    void handleLoggedMessage(const QOpenGLDebugMessage &debugMessage);
private:
    void initializeOpenGL();
    void initYuv420PTextureInfo();
    void loadTextures();
    void loadPlaneTextureFromPbo(int glTextureUnit, int textureUnit,
                                  int lineSize, int height, size_t offset);
    void adjustPaintAreas(const QRectF& targetRect, Qt::AspectRatioMode aspectRationMode);
    void initializeVertexCoordinatesBuffer(const QRectF &videoRect);
    void initializeTextureCoordinatesBuffer();
    quint32 combinedSizeOfTextures();
    /** Get a new FrameBuffer, to which we can copy frame information from libav */
    std::weak_ptr<FrameBuffer> getNextFrameBuffer();
    /**
     * Make the OpenGLPainter reqeuest new frames to fill it's buffers.
     */
    void requestNewFrames();

    QGLWidget* _widget;
    QOpenGLFunctions_1_3 *_glFunctions;
    bool _openGLInitialized;
    bool _firstFrameLoaded;
    bool _texturesInitialized;
    QSize _sourceTotalSize;
    QSize _sourcePictureSize;
    QRectF _targetRect;
    QRectF _blackBar1, _blackBar2;
    bool _sourceSizeDirty;
    Qt::AspectRatioMode _aspectRatioMode;

    GLuint _yTextureId;
    GLuint _uTextureId;
    GLuint _vTextureId;

    std::array<int, 3> _textureWidths;
    std::array<int, 3> _textureHeights;
    std::array<int, 3> _textureOffsets;

    QOpenGLBuffer _textureCoordinatesBuffer;
    QOpenGLBuffer _vertexBuffer;

    /**
     * Buffer containing
     * * a QOpenGLBuffer representing an OpenGL Pixel Buffer Object.
     * * a Mapped Pixel Buffer object, which represents the QOpenGLBuffer mapped to memory, so we can copy a frame into it.
     * * a frameNumber, representing the number of the frame that is contained.
     */
    struct PixelBuffer {
        QOpenGLBuffer openGlPixelBuffer;
        std::shared_ptr<FrameBuffer> mappedPixelBuffer;
        qint64 frameNumber;
    };
    /** This is our frame buffer, containing a number of frames that can be displayed */
    std::array<PixelBuffer, NUMBER_OF_BUFFERS> _pixelBuffers;

    /** The last position in _pixelBuffers in which a frame was written. */
    quint32 _currentPixelBufferWritePosition;
    /** The current position from which a frame will displayed. */
    quint32 _currentPixelBufferReadPosition;
    /** The last position in _pixelBuffers which has been mapped to memory. */
    quint32 _currentPixelBufferMappedPosition;

    QOpenGLShaderProgram _program;
};

#endif // OPENGLPAINTER_H
