#ifndef COMMON_H
#define COMMON_H

#include <QtTest/QTest>
#define QCOMPARE_BYTE(actual, expected) \
do {\
    if (!QTest::qCompare(static_cast<const quint8>(actual), static_cast<quint8>(expected), #actual, #expected, __FILE__, __LINE__)) {\
        return;\
    }\
} while (0)

#endif // COMMON_H
