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

#ifndef DISTANCEENTRYCOLLECTION_H
#define DISTANCEENTRYCOLLECTION_H

#include <functional>
#include <vector>

template <typename T>
class DistanceEntryCollection {
public:
    explicit DistanceEntryCollection();
    explicit DistanceEntryCollection(const std::vector<T> &entries, std::function<qreal(const T&)> distanceFunction);
    explicit DistanceEntryCollection(const DistanceEntryCollection& other);

    DistanceEntryCollection<T> &operator =(const DistanceEntryCollection& other);

    bool empty() const {
        return _entries.empty();
    }

    const std::vector<T> &entries() const {
        return _entries;
    }

    const typename std::vector<T>::const_iterator iteratorForDistance(const qreal distance);

    const T *entryForDistance(const qreal distance);

    bool isEndEntryIterator(const typename std::vector<T>::const_iterator &it) const;
private:
    std::vector<T> _entries;
    std::function<qreal(const T&)> _distanceFunction;

    typename std::vector<T>::const_iterator _currentEntry;
};

template <typename T>
DistanceEntryCollection<T>::DistanceEntryCollection()
{
    // empty
}

template <typename T>
DistanceEntryCollection<T>::DistanceEntryCollection(const std::vector<T> &entries, std::function<qreal(const T &)> distanceFunction):
    _entries(entries), _distanceFunction(distanceFunction), _currentEntry(_entries.begin())
{
    // empty
}

template <typename T>
DistanceEntryCollection<T>::DistanceEntryCollection(const DistanceEntryCollection &other):
    DistanceEntryCollection<T>(other._entries, other._distanceFunction)
{

}

template <typename T>
DistanceEntryCollection<T> &DistanceEntryCollection<T>::operator =(const DistanceEntryCollection<T> &other)
{
    _entries = other._entries;
    _distanceFunction = other._distanceFunction;
    _currentEntry = _entries.begin();

    return *this;
}

template <typename T>
const typename std::vector<T>::const_iterator DistanceEntryCollection<T>::iteratorForDistance(const qreal distance)
{
    if (_entries.empty()) {
        return _entries.end();
    }
    auto nextEntry = _currentEntry + 1;
    const bool atLastEntry = _currentEntry == _entries.end() || nextEntry == _entries.end();
    const bool isDistanceBiggerThenEndOfCurrent = !atLastEntry && distance > _distanceFunction(*(nextEntry));

    // optimization for the common case. With most of these distance entry collections, we're going through them
    // from beginning to end. Rather than doing a binary search right away, we'll first check if the distance that
    // is requested is bigger than the start of the next entry, but smaller than the start of the entry after that.
    // If so, we can just use the next entry, without going to search mode.
    if (!atLastEntry && isDistanceBiggerThenEndOfCurrent) {
        auto entryAfterNext = nextEntry + 1;
        const bool distanceSmallerThanEndOfNextEntry = entryAfterNext != _entries.end() && distance < _distanceFunction(*entryAfterNext);
        if (distanceSmallerThanEndOfNextEntry) {
            _currentEntry = nextEntry;
            return _currentEntry;
        }
    }
    const bool isDistanceSmallerThenStartOfCurrent = distance < _distanceFunction(*_currentEntry);
    // if the distance we're looking for is smaller than the current entry, or bigger than the
    // start of the next entry, then we need to search which entry to use. If not, we can
    // simply use the current entry.
    if (isDistanceSmallerThenStartOfCurrent || (!atLastEntry && isDistanceBiggerThenEndOfCurrent)) {
        // optimization, determine bounds. If distance is before current entry start at begin, otherwise, start at nextEntry.
        auto begin = isDistanceSmallerThenStartOfCurrent ? _entries.begin() : nextEntry;
        // if distance is after current entry, end at end of vector, otherwise, end at the current entry.
        auto end = isDistanceBiggerThenEndOfCurrent ? _entries.end(): _currentEntry;

        auto it = std::lower_bound(begin, end, distance, [this](const T &entry, qreal distance) {
            return _distanceFunction(entry) < distance;
        });

        // lower bound gives us the first entry that is not smaller then distance, so we need to
        // go back one step, unless we're already at the beginning.
        if (it != _entries.begin() && (it == _entries.end() || _distanceFunction(*it) > distance) ) {
            it--;
        }
        _currentEntry = it;
    }
    return _currentEntry;
}

template <typename T>
const T *DistanceEntryCollection<T>::entryForDistance(const qreal distance)
{
    auto it = iteratorForDistance(distance);
    if (it == _entries.end()) {
        return nullptr;
    }
    return &(*it);
}

template <typename T>
bool DistanceEntryCollection<T>::isEndEntryIterator(const typename std::vector<T>::const_iterator &it) const
{
    return it == _entries.end();
}

#endif // DISTANCEENTRYCOLLECTION_H
