#include "distancemappingentry.h"

DistanceMappingEntry::DistanceMappingEntry(float distance, quint32 frameNumber, float metersPerFrame):
    _distance(distance), _frameNumber(frameNumber), _metersPerFrame(metersPerFrame)
{
}

DistanceMappingEntry::DistanceMappingEntry():
    DistanceMappingEntry(0, 0u, 0)
{
}

float DistanceMappingEntry::distance() const
{
    return _distance;
}

quint32 DistanceMappingEntry::frameNumber() const
{
    return _frameNumber;
}

float DistanceMappingEntry::metersPerFrame() const
{
    return _metersPerFrame;
}
