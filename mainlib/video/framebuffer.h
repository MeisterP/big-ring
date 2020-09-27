#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <functional>
#include <QtCore/QMutex>
#include <QtCore/QSize>

class FrameBuffer
{
public:
    FrameBuffer(void *mappedPixelBuffer, const QSize &frameSize, int index):
        _ptr(mappedPixelBuffer), _frameSize(frameSize), _index(index)
    {
        // empty
    }

    ~FrameBuffer() {
        // no need to do anything
    }

    /**
     * Call the supplied function, but only after having acquired the mutex. If the mutex cannot be acquired,
     * for instance reset() is being called at the same moment, don't do anything.
     */
    void withMutex(const std::function<void(void *ptr, const QSize &frameSize, int index)> func) {
        if (_mutex.tryLock()) {
            if (_ptr) {
                func(_ptr, _frameSize, _index);
            }
            // always unlock the mutex!
            _mutex.unlock();
        }
    }

    /**
     * Reset the buffer so it cannot be used anymore.
     */
    void reset() {
        QMutexLocker locker(&_mutex);
        _ptr = nullptr;
    }

private:
    void* _ptr;
    const QSize _frameSize;
    const int _index;
    QMutex _mutex;
};

#endif // FRAMEBUFFER_H
