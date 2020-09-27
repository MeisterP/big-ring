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

#ifndef BYTEARRAYTOANTMESSAGECONVERTER_H
#define BYTEARRAYTOANTMESSAGECONVERTER_H

#include <QObject>

class AntMessageGatherer : public QObject
{
    Q_OBJECT
public:
    explicit AntMessageGatherer(QObject *parent = 0);

public slots:
    void submitBytes(QByteArray bytes);
signals:
    void antMessageReceived(const QByteArray& messageBytes);
private:
    bool messageIsTooShort(const QByteArray &messageBytes, int length);
    bool checksumIsOk(const QByteArray& messageBytes);

    QByteArray _bytesReceived;
};

#endif // BYTEARRAYTOANTMESSAGECONVERTER_H
