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

#include "versionchecker.h"
#include "util/util.h"
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
namespace {
const QString RELEASES_URL = "https://api.github.com/repos/MeisterP/big-ring/releases/latest";
}

VersionChecker::VersionChecker(QObject *parent) :
    QObject(parent), _networkAccessManager(new QNetworkAccessManager(this))
{
}

void VersionChecker::checkForNewVersion()
{
    QNetworkRequest request(RELEASES_URL);

    QNetworkReply *reply = _networkAccessManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &VersionChecker::replyReceived);
}

void VersionChecker::replyReceived()
{
    QString latestRelease = "";
    bool newVersion = false;

    QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
    if (reply->error() == QNetworkReply::NoError) {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (statusCode == 200) {
            const QByteArray data = reply->readAll();
            QJsonParseError error;
            const QJsonDocument json = QJsonDocument::fromJson(data,&error);
            if (error.error == QJsonParseError::NoError && json.isObject()) {
                const QJsonValue releaseValue = json.object()["tag_name"];
                if (releaseValue.isString()) {
                    latestRelease = parseVersion(releaseValue.toString());
                    newVersion = checkIfNewerVersionAvailable(latestRelease);
                }
            }
        }
    }
    reply->deleteLater();
    emit newVersionAvailable(newVersion, latestRelease);
}

QString VersionChecker::parseVersion(const QString tagName) const
{
    if (tagName.startsWith("release-")) {
        return tagName.mid(QString("release-").length());
    }
    return "";
}

bool VersionChecker::checkIfNewerVersionAvailable(const QString &latestVersion)
{
    const QStringList thisVersionParts = QString(APP_VERSION).split(".");
    const QStringList latestVersionParts = latestVersion.split(".");

    if (latestVersionParts.size() < 3) {
        return false;
    }

    for (int i = 0; i < 3; ++i) {
        const QString thisVersionPartString = thisVersionParts[i];
        const QString latestVersionPartString = latestVersionParts[i];
        bool ok;
        int thisVersionPart = thisVersionPartString.toInt(&ok);
        if (!ok) {
            return false;
        }
        int latestVersionPart = latestVersionPartString.toInt(&ok);
        if (!ok) {
            return false;
        }
        if (latestVersionPart > thisVersionPart) {
            return true;
        } else if (latestVersionPart < thisVersionPart) {
            return false;
        }
    }
    return false;
}
