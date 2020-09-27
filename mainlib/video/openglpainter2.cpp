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

#include "openglpainter2.h"

#include <QtGui/QOpenGLContext>
#include <QtCore/QtMath>
#include <QtCore/QThread>
#include <QtCore/QTime>
#include <QtGui/QOpenGLDebugLogger>
#include <QtGui/QOpenGLFunctions>

OpenGLPainter2::OpenGLPainter2(QGLWidget* widget, QObject *parent) :
    QObject(parent), _widget(widget), _openGLInitialized(false), _firstFrameLoaded(false),
    _texturesInitialized(false), _aspectRatioMode(Qt::KeepAspectRatioByExpanding),
    _currentPixelBufferWritePosition(0),
    _currentPixelBufferReadPosition(0),
    _currentPixelBufferMappedPosition(0)
{
    Q_INIT_RESOURCE(shaders);
}

OpenGLPainter2::~OpenGLPainter2()
{
    // we'll make sure that the map pixel buffers cannot be used anymore. To do this, we'll reset all the FrameBuffer
    // objects in them.
    std::for_each(_pixelBuffers.begin(), _pixelBuffers.end(),
                  [](PixelBuffer &buffer) {
        buffer.mappedPixelBuffer->reset();
    });
}

/**
 * 3 textures will be loaded, one for each of the Y, U and V planes. These textures will be applied by the the
 * OpenGL fragment shader. The GPU is much more efficient than the CPU for doing conversion from YUV to RGB, and scaling the
 * video to the right size.
 */
void OpenGLPainter2::loadTextures()
{
    loadPlaneTextureFromPbo(GL_TEXTURE0, _yTextureId, _textureWidths[0], _textureHeights[0], (size_t) 0);
    loadPlaneTextureFromPbo(GL_TEXTURE1, _uTextureId, _textureWidths[1], _textureHeights[1], _textureOffsets[1]);
    loadPlaneTextureFromPbo(GL_TEXTURE2, _vTextureId, _textureWidths[2], _textureHeights[2], _textureOffsets[2]);

    // on the first pass, we need to load the textures with glTexImage2D
    // on every subsequent pass we can use glTexSubImage2D, which can be faster.
    // To facilitate this, set _texturesInitialized to true after the first pass.
    _texturesInitialized = true;
}

void OpenGLPainter2::loadPlaneTextureFromPbo(int glTextureUnit, int textureUnit,
                                           int lineSize, int height, size_t offset)
{

    _glFunctions->glActiveTexture(glTextureUnit);
    _glFunctions->glBindTexture(GL_TEXTURE_RECTANGLE, textureUnit);
    QOpenGLBuffer &pixelBuffer = _pixelBuffers[_currentPixelBufferReadPosition].openGlPixelBuffer;

    pixelBuffer.bind();

    // for the first texture upload of a texture unit, we need to use glTexImage2D. After that, we can use
    // glTexSubImage2D, which should be faster most of the times.
    if (_texturesInitialized) {
        _glFunctions->glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, lineSize, height,
                        GL_LUMINANCE, GL_UNSIGNED_BYTE, (void*) offset);
    } else {
        _glFunctions->glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_LUMINANCE, lineSize, height,
                     0,GL_LUMINANCE,GL_UNSIGNED_BYTE, (void*) offset);
    }

    pixelBuffer.release();
    _glFunctions->glTexParameteri(GL_TEXTURE_RECTANGLE,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    _glFunctions->glTexParameteri(GL_TEXTURE_RECTANGLE,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    _glFunctions->glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    _glFunctions->glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
}

void OpenGLPainter2::paint(QPainter *painter, const QRectF &rect, Qt::AspectRatioMode aspectRatioMode)
{
    QTime time;
    time.start();
    if (!_openGLInitialized) {
        initializeOpenGL();
    }
    if (!_firstFrameLoaded) {
        painter->fillRect(rect, Qt::black);
        return;
    }

    adjustPaintAreas(rect, aspectRatioMode);

    // if these are enabled, we need to reenable them after beginNativePainting()
    // has been called, as they may get disabled
    bool stencilTestEnabled = _glFunctions->glIsEnabled(GL_STENCIL_TEST);
    bool scissorTestEnabled = _glFunctions->glIsEnabled(GL_SCISSOR_TEST);

    painter->beginNativePainting();

    if (stencilTestEnabled)
        _glFunctions->glEnable(GL_STENCIL_TEST);
    if (scissorTestEnabled)
        _glFunctions->glEnable(GL_SCISSOR_TEST);

    _glFunctions->glEnableClientState(GL_VERTEX_ARRAY);
    _glFunctions->glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    _program.bind();

    loadTextures();

    // set the texture and vertex coordinates using VBOs.
    _textureCoordinatesBuffer.bind();
    _glFunctions->glTexCoordPointer(2, GL_FLOAT, 0, 0);
    _textureCoordinatesBuffer.release();

    _vertexBuffer.bind();
    _glFunctions->glVertexPointer(2, GL_FLOAT, 0, 0);
    _vertexBuffer.release();

    _glFunctions->glActiveTexture(GL_TEXTURE0);
    _glFunctions->glBindTexture(GL_TEXTURE_RECTANGLE, _yTextureId);
    _glFunctions->glActiveTexture(GL_TEXTURE1);
    _glFunctions->glBindTexture(GL_TEXTURE_RECTANGLE, _uTextureId);
    _glFunctions->glActiveTexture(GL_TEXTURE2);
    _glFunctions->glBindTexture(GL_TEXTURE_RECTANGLE, _vTextureId);
    _glFunctions->glActiveTexture(GL_TEXTURE0);

    _program.setUniformValue("yTex", 0);
    _program.setUniformValue("uTex", 1);
    _program.setUniformValue("vTex", 2);

    _glFunctions->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    _program.release();

    _glFunctions->glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    _glFunctions->glDisableClientState(GL_VERTEX_ARRAY);

    painter->endNativePainting();
    painter->fillRect(_blackBar1, Qt::black);
    painter->fillRect(_blackBar2, Qt::black);

//    qDebug() << "Painting took" << time.elapsed() << "ms";
}

std::weak_ptr<FrameBuffer> OpenGLPainter2::getNextFrameBuffer()
{
    if (!_openGLInitialized) {
        _widget->context()->makeCurrent();
        initializeOpenGL();
    }
    _widget->context()->makeCurrent();

    QOpenGLBuffer &openGlBuffer = _pixelBuffers[_currentPixelBufferMappedPosition].openGlPixelBuffer;

    openGlBuffer.bind();
    // if the frame buffer is currently mapped, we first need to reset the mapped pointer and then unmap the pixel buffer.
    std::shared_ptr<FrameBuffer> oldFrameBuffer = _pixelBuffers[_currentPixelBufferMappedPosition].mappedPixelBuffer;
    if (oldFrameBuffer) {
        // resets the mapped pointer in oldFrameBuffer. reset() uses a mutex internally
        oldFrameBuffer->reset();
        openGlBuffer.unmap();
    }

    /* map the OpenGL buffer to a pointer in memory, so a frame can be copied */
    void *mappedBufferPtr = openGlBuffer.map(QOpenGLBuffer::WriteOnly);

    if (mappedBufferPtr == nullptr) {
        qWarning("Unable map opengl pixel buffer nr %d", _currentPixelBufferMappedPosition);
    }

    std::shared_ptr<FrameBuffer> frameBuffer(new FrameBuffer(mappedBufferPtr, _sourceTotalSize, _currentPixelBufferMappedPosition));
    _pixelBuffers[_currentPixelBufferMappedPosition].mappedPixelBuffer = frameBuffer;

    // Set mapped pixel buffer position to the next position to map.
    _currentPixelBufferMappedPosition = (_currentPixelBufferMappedPosition + 1) % _pixelBuffers.size();

    openGlBuffer.release();

    return frameBuffer;
}

void OpenGLPainter2::setVideoSize(const QSize &videoSize, const QSize& frameSize)
{
    if (!_openGLInitialized) {
        _widget->context()->makeCurrent();
        initializeOpenGL();
        qDebug() << "initializing opengl";
    }
    if (frameSize != _sourceTotalSize) {
        _sourceSizeDirty = true;
        _sourceTotalSize = frameSize;
        _sourcePictureSize = videoSize;
        initYuv420PTextureInfo();
    }
}

void OpenGLPainter2::setFrameLoaded(int index, qint64 frameNumber, const QSize &)
{
    QTime time;
    time.start();
    _widget->context()->makeCurrent();
    _currentPixelBufferWritePosition = index;
    QOpenGLBuffer &buffer = _pixelBuffers[index].openGlPixelBuffer;
    buffer.bind();
    buffer.unmap();
    buffer.release();
    _pixelBuffers[index].frameNumber = frameNumber;
    _firstFrameLoaded = true;
    _glFunctions->glFlush();
}

/**
 * Request new frames to make sure the buffer is a full as possible.
 */
void OpenGLPainter2::requestNewFrames()
{
    while (_currentPixelBufferMappedPosition != _currentPixelBufferReadPosition) {
        emit frameNeeded(getNextFrameBuffer());
    }
}

bool OpenGLPainter2::showFrame(qint64 frameNumber)
{
    // if we are requested to show the current frame, do nothing.
    if (_pixelBuffers[_currentPixelBufferReadPosition].frameNumber >= frameNumber) {
        qDebug() << "now new frame shown, because" << _pixelBuffers[_currentPixelBufferReadPosition].frameNumber << ">=" << frameNumber;
        return false;
    }

    quint32 newIndex = (_currentPixelBufferReadPosition + 1) % _pixelBuffers.size();
    while (_pixelBuffers[newIndex].frameNumber < frameNumber && newIndex != _currentPixelBufferReadPosition) {
        newIndex = (newIndex + 1) % _pixelBuffers.size();
    }
    if (newIndex == _currentPixelBufferWritePosition) {
        qDebug() << "frame not present, have to wait for new frames to arrive to catch up.";
    }

    _currentPixelBufferReadPosition = newIndex;
    requestNewFrames();
    return true;
}

void OpenGLPainter2::fillBuffers()
{
    for (quint32 i = 0; i < _pixelBuffers.size(); ++i) {
        emit frameNeeded(getNextFrameBuffer());
    }
}

void OpenGLPainter2::handleLoggedMessage(const QOpenGLDebugMessage &debugMessage)
{
    qDebug() << "opengl log message" << debugMessage.message();
}

/**
 * @brief compile the opengl shader program from the sources and link it, if the program is not yet linked.
 */
void OpenGLPainter2::initializeOpenGL()
{
    Q_ASSERT_X(!_program.isLinked(), "initializeOpenGL", "OpenGL already initialized");
    qDebug() << "INITIALIZING OPENGL";
    _glFunctions = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_1_3>();
    if (!_glFunctions) {
        qWarning() << "Could not obtain required OpenGL context version";
        exit(1);
    }
    _glFunctions->initializeOpenGLFunctions();
    QOpenGLFunctions functions(QOpenGLContext::currentContext());
    if (!_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":///vertexshader.glsl")) {
        qFatal("Unable to add vertex shader: %s", qPrintable(_program.log()));
    }
    if (!_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":///fragmentshader.glsl")) {
        qFatal("Unable to add fragment shader: %s", qPrintable(_program.log()));
    }
    if (!_program.link()) {
        qFatal("Unable to link shader program: %s", qPrintable(_program.log()));
    }
    QOpenGLContext* glContext = QOpenGLContext::currentContext();
    if (!functions.hasOpenGLFeature(QOpenGLFunctions::NPOTTextures)) {
        qFatal("OpenGL needs to have support for 'Non power of two textures'");
    }
    if (!glContext->hasExtension("GL_ARB_pixel_buffer_object")) {
        qFatal("GL_ARB_pixel_buffer_object is missing");
    }
    if (!functions.hasOpenGLFeature(QOpenGLFunctions::Buffers)) {
        qFatal("OpenGL needs to have support for vertex buffers");
    }
//    QOpenGLDebugLogger *logger = new QOpenGLDebugLogger(this);
//    connect(logger, &QOpenGLDebugLogger::messageLogged, this, &OpenGLPainter2::handleLoggedMessage);
//    logger->initialize();
//    logger->startLogging();

    qDebug() << "generating textures.";
    _glFunctions->glGenTextures(1, &_yTextureId);
    _glFunctions->glGenTextures(1, &_uTextureId);
    _glFunctions->glGenTextures(1, &_vTextureId);

    _openGLInitialized = true;
}

void OpenGLPainter2::initializeVertexCoordinatesBuffer(const QRectF& videoRect)
{
    const QVector<GLfloat> vertexCoordinates =
    {
        GLfloat(videoRect.left()), GLfloat(videoRect.top()),
        GLfloat(videoRect.right() + 1), GLfloat(videoRect.top()),
        GLfloat(videoRect.left()), GLfloat(videoRect.bottom() + 1),
        GLfloat(videoRect.right() + 1), GLfloat(videoRect.bottom() + 1)
    };
    if (_vertexBuffer.isCreated()) {
        _vertexBuffer.destroy();
    }
    _vertexBuffer.create();
    _vertexBuffer.bind();
    _vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _vertexBuffer.allocate(vertexCoordinates.data(), sizeof(GLfloat) * vertexCoordinates.size());
    _vertexBuffer.release();
}

void OpenGLPainter2::adjustPaintAreas(const QRectF& targetRect, Qt::AspectRatioMode aspectRationMode)
{
    if (_sourceSizeDirty || targetRect != _targetRect || aspectRationMode != _aspectRatioMode) {
        _targetRect = targetRect;

        // change size of video to fit the targetRect completely.
        QSizeF sourceSizeF(_sourcePictureSize);
        const QSizeF videoSizeAdjusted(sourceSizeF.scaled(targetRect.size(), aspectRationMode));

        QRectF videoRect = QRectF(QPointF(), videoSizeAdjusted);
        videoRect.moveCenter(targetRect.center());

        initializeVertexCoordinatesBuffer(videoRect);

        if (targetRect.left() == videoRect.left()) {
            // black bars on top and bottom
            _blackBar1 = QRectF(targetRect.topLeft(), videoRect.topRight());
            _blackBar2 = QRectF(videoRect.bottomLeft(), _targetRect.bottomRight());
        } else {
            // black bars on the sidex
            _blackBar1 = QRectF(targetRect.topLeft(), videoRect.bottomLeft());
            _blackBar2 = QRectF(videoRect.topRight(), _targetRect.bottomRight());
        }
        _sourceSizeDirty = false;
    }
}

quint32 OpenGLPainter2::combinedSizeOfTextures()
{
    quint32 size = 0u;
    for (int i = 0; i < 3; ++i) {
        size += _textureWidths[i] * _textureHeights[i];
    }
    return size;
}

void OpenGLPainter2::initializeTextureCoordinatesBuffer()
{
    const QVector<GLfloat> textureCoordinates = {
        0, 0,
        static_cast<GLfloat>(_sourcePictureSize.width()), 0,
        0, static_cast<GLfloat>(_sourcePictureSize.height()),
        static_cast<GLfloat>(_sourcePictureSize.width()), static_cast<GLfloat>(_sourcePictureSize.height())
    };

    if (_textureCoordinatesBuffer.isCreated()) {
        _textureCoordinatesBuffer.destroy();
    }
    _textureCoordinatesBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    _textureCoordinatesBuffer.create();
    _textureCoordinatesBuffer.bind();
    _textureCoordinatesBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _textureCoordinatesBuffer.allocate(textureCoordinates.data(), sizeof(GLfloat) * textureCoordinates.size());
    _textureCoordinatesBuffer.release();
}

void OpenGLPainter2::initYuv420PTextureInfo()
{
    const int bytesPerLineY = (_sourceTotalSize.width() + 3) & ~3;
    const int bytesPerLineUAndV = (_sourceTotalSize.width() / 2 + 3) & ~3;
    _textureWidths[0] = bytesPerLineY;
    _textureHeights[0] = _sourceTotalSize.height();
    _textureOffsets[0] = 0;
    _textureWidths[1] = bytesPerLineUAndV;
    _textureHeights[1] = _sourceTotalSize.height() / 2;
    _textureOffsets[1] = bytesPerLineY * _sourceTotalSize.height();
    _textureWidths[2] = bytesPerLineUAndV;
    _textureHeights[2] = _sourceTotalSize.height() / 2;
    _textureOffsets[2] = bytesPerLineY * _sourceTotalSize.height() + bytesPerLineUAndV * _sourceTotalSize.height()/2;

    initializeTextureCoordinatesBuffer();

    for (quint32 i = 0; i < _pixelBuffers.size(); ++i) {
        if (_pixelBuffers[i].openGlPixelBuffer.isCreated()) {
            _pixelBuffers[i].openGlPixelBuffer.destroy();
        }
        QOpenGLBuffer pixelBuffer(QOpenGLBuffer::PixelUnpackBuffer);
        pixelBuffer.create();
        pixelBuffer.bind();
        pixelBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
        pixelBuffer.allocate(combinedSizeOfTextures());
        pixelBuffer.release();
        _pixelBuffers[i].openGlPixelBuffer = pixelBuffer;
    }
    _texturesInitialized = false;
}

