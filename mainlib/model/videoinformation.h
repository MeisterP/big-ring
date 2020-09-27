#ifndef VIDEOINFORMATION_H
#define VIDEOINFORMATION_H

#include <QtCore/QString>

class VideoInformation
{
public:
    explicit VideoInformation(const QString &videoFilename, float frameRate);
    explicit VideoInformation();

    const QString &videoFilename() const { return _videoFilename; }
    float frameRate() const { return _frameRate; }

private:
    QString _videoFilename;
    float _frameRate;
};
#endif // VIDEOINFORMATION_H
