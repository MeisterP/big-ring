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

#ifndef VERSIONCHECKER_H
#define VERSIONCHECKER_H

#include <QtCore/QObject>
#include <QtNetwork/QNetworkAccessManager>

/**
 * @brief Checks for new versions of Big Ring that can be downloaded from the website.
 *
 * Use this class to check if there is a new version of Big Ring available on the website.
 *
 * The way to use this class is simple:
 * 1. Create a VersionChecker object.
 * 2. connect to the newVersionAvailable(bool, QString&) signal
 * 3. call checkForNewVersion().
 *
 * The version check will be performed a synchronously.
 */
class VersionChecker : public QObject
{
    Q_OBJECT
public:
    explicit VersionChecker(QObject *parent = 0);

signals:
    /** signals if a new version is available. If so, \param newVersion is set to true, and
     * \param versionString will contain the version string (e.g. 1.5.0).
     */
    void newVersionAvailable(bool newVersion, const QString &versionString);
public slots:
    /** perform the check for a new version */
    void checkForNewVersion();
private slots:
    void replyReceived();
private:
    QString parseVersion(const QString tagName) const;
    bool checkIfNewerVersionAvailable(const QString &latestVersion);
    QNetworkAccessManager *_networkAccessManager;
};

#endif // VERSIONCHECKER_H
