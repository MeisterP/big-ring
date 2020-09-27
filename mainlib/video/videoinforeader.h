#ifndef VIDEOINFOREADER_H
#define VIDEOINFOREADER_H

#include <QFileInfo>

struct VideoInfo {
    float frameRate;
};

#include "genericvideoreader.h"
class VideoInfoReader : public GenericVideoReader
{
    Q_OBJECT
public:
    explicit VideoInfoReader(QObject *parent = 0);
    virtual ~VideoInfoReader();

    VideoInfo videoInfoForVideo(const QFileInfo &videoFileInfo);
signals:

public slots:

};

#endif // VIDEOINFOREADER_H
