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

#include "distanceentrycollectiontest.h"

namespace {
class SimpleEntry {
public:
    explicit SimpleEntry(qreal distance, qreal height):
        _distance(distance), _height(height) {
    }
    qreal distance() const { return _distance; }

    bool operator ==(const SimpleEntry &other) const {
        return _distance == other._distance && _height == other._height;
    }

private:
    qreal _distance;
    qreal _height;
};

std::function<qreal(const SimpleEntry&)> indexFunction = [](const SimpleEntry& entry) {
    return entry.distance();
};
}
#include <QtTest/QTest>
DistanceEntryCollectionTest::DistanceEntryCollectionTest(QObject *parent) :
    QObject(parent)
{
}

void DistanceEntryCollectionTest::testWithEmptyEntries()
{
    std::vector<SimpleEntry> entries = {};
    DistanceEntryCollection<SimpleEntry> indexed(entries, indexFunction);

    const SimpleEntry *entry = indexed.entryForDistance(10.0);

    QVERIFY2(entry == nullptr, "No entry should be found");
}

void DistanceEntryCollectionTest::testWithSingleEntry()
{
    std::vector<SimpleEntry> entries = { SimpleEntry(10.0, 0.0) };
    DistanceEntryCollection<SimpleEntry> indexed(entries, indexFunction);

    const SimpleEntry *entry = indexed.entryForDistance(1.0);

    QVERIFY2(entry, "Entry should not be null");
    QVERIFY2(*entry == entries[0], "First entry should be found");
}

void DistanceEntryCollectionTest::testWithFourEntries()
{
    std::vector<SimpleEntry> entries = { SimpleEntry(0.0, 0.0), SimpleEntry(10.0, 1.0), SimpleEntry(20.0, 3.0), SimpleEntry(30.0, 4.0) };
    DistanceEntryCollection<SimpleEntry> indexed(entries, indexFunction);

    const SimpleEntry *entry = indexed.entryForDistance(1.0);

    QVERIFY2(entry, "Entry should not be null");
    QVERIFY2(*entry == entries[0], "First entry should be found");

    entry = indexed.entryForDistance(25.0);
    QVERIFY2(entry, "Entry should not be null");
    QVERIFY2(*entry == entries[2], "Third entry should be found");

    entry = indexed.entryForDistance(20.0);
    QVERIFY2(entry, "Entry should not be null");
    QVERIFY2(*entry == entries[2], "Third entry should be found");

    entry = indexed.entryForDistance(10.0);
    QVERIFY2(entry, "Entry should not be null");
    QVERIFY2(*entry == entries[1], "Second entry should be found");

    for (int i = 0; i < 300; ++i) {
        entry = indexed.entryForDistance(i / 10.0);
        QVERIFY2(entry, "Entry should not be null");
    }
}
