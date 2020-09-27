#ifndef PROFILETEST_H
#define PROFILETEST_H

#include <QObject>

class ProfileTest : public QObject
{
    Q_OBJECT
public:
    explicit ProfileTest(QObject *parent = 0);
private slots:
    void testMaximumAltitude();
};

#endif // PROFILETEST_H
