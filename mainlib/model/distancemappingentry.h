#ifndef DISTANCEMAPPINGENTRY_H
#define DISTANCEMAPPINGENTRY_H

#include <QtCore/QtGlobal>
/**
 * Maps a distance to a frame number and meters per frame for this entry.
 */
class DistanceMappingEntry
{
public:
    explicit DistanceMappingEntry(float distance, quint32 frameNumber, float metersPerFrame);
    explicit DistanceMappingEntry();

    float distance() const;
    quint32 frameNumber() const;
    float metersPerFrame() const;

private:
    float _distance;
    quint32 _frameNumber;
    float _metersPerFrame;
};
#endif // DISTANCEMAPPINGENTRY_H
