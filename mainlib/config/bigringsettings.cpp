/*
 * Copyright (c) 2015 Ilja Booij (ibooij@gmail.com)
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
#include "bigringsettings.h"

#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/QUuid>

namespace
{
// By default we'll average power over 3 seconds.
const int POWER_AVERAGING_MILLISECONDS_DEFAULT = 3000;

const qreal USER_WEIGHT_KILOGRAMS_DEFAULT = 75.0;
const qreal BIKE_WEIGHT_KILOGRAMS_DEFAULT = 10.0;

const qreal DEFAULT_UP_AND_DOWNHILL_CAPS = 25.0;
const int DEFAULT_DIFFICULTY_SETTING = 100;
}

BigRingSettings::BigRingSettings()
{
    // empty
}

QStringList BigRingSettings::videoFolders() const
{
    QStringList videoFolders;
    QSettings settings;

    // There could be a pre-2.8 videoFolder setting. We need to handle this.
    if (settings.contains("videoFolder")) {
        QString oldVideoFolderSetting = settings.value("videoFolder").toString();
        if (!oldVideoFolderSetting.isEmpty()) {
            videoFolders << oldVideoFolderSetting;
        }
    }

    const int size = settings.beginReadArray("videoFolders");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        const QString videoFolder = settings.value("videoFolder").toString();
        if (!videoFolders.contains(videoFolder)) {
            videoFolders << videoFolder;
        }
    }
    settings.endArray();

    std::sort(videoFolders.begin(), videoFolders.end());

    return videoFolders;
}

bool BigRingSettings::addVideoFolder(const QString &folder)
{
    QStringList currentVideoFolders = videoFolders();
    if (currentVideoFolders.contains(folder)) {
        return false;
    }
    QSettings settings;
    int size = settings.beginReadArray("videoFolders");
    settings.endArray();
    settings.beginWriteArray("videoFolders");
    settings.setArrayIndex(size);
    settings.setValue("videoFolder", folder);
    settings.endArray();

    return true;
}

void BigRingSettings::removeVideoFolder(const QString &folder)
{
    // It could be that the legacy setting for videoFolder contained this folder
    // name. If so, remove it here.
    if (folder == _settings.value("videoFolder").toString()) {
        _settings.remove("videoFolder");
    }

    // We'll just get all current folders, remove the one we have to remove and rewrite the array.
    QStringList currentVideoFolders = videoFolders();
    currentVideoFolders.removeOne(folder);
    _settings.beginWriteArray("videoFolders");
    for (int i = 0; i < currentVideoFolders.size(); ++i) {
        _settings.setArrayIndex(i);
        _settings.setValue("videoFolder", currentVideoFolders[i]);
    }
    _settings.endArray();
}

QString BigRingSettings::tcxFolder() const
{
    if (_settings.contains("tcxSaveFolder")) {
        return _settings.value("tcxSaveFolder").toString();
    } else {
        const QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        const QString directoryName = QString("Big-Ring/Rides");

        return documentsPath + "/" + directoryName;
    }
}

void BigRingSettings::setTcxFolder(const QString &folder)
{
    _settings.setValue("tcxSaveFolder", QVariant::fromValue(folder));
}

qreal BigRingSettings::userWeight() const
{
    return _settings.value("user.weight", QVariant::fromValue(USER_WEIGHT_KILOGRAMS_DEFAULT)).toDouble();
}

void BigRingSettings::setUserWeight(const qreal userWeight)
{
    _settings.setValue("user.weight", QVariant::fromValue(userWeight));
}

qreal BigRingSettings::bikeWeight() const
{
    return _settings.value("bike.weight", QVariant::fromValue(BIKE_WEIGHT_KILOGRAMS_DEFAULT)).toDouble();
}

void BigRingSettings::setBikeWeight(const qreal bikeWeight)
{
    _settings.setValue("bike.weight", QVariant::fromValue(bikeWeight));
}

int BigRingSettings::powerAveragingForDisplayMilliseconds() const
{
    QSettings settings;
    settings.beginGroup("display");
    const int milliseconds = settings.value("powerAveragingMilliseconds", POWER_AVERAGING_MILLISECONDS_DEFAULT).toInt();
    settings.endGroup();
    return milliseconds;
}

void BigRingSettings::setPowerAveragingForDisplayMilliseconds(const int averagingMilliseconds)
{
    QSettings settings;
    settings.beginGroup("display");
    settings.setValue("powerAveragingMilliseconds", averagingMilliseconds);
    settings.endGroup();
}

Qt::AspectRatioMode BigRingSettings::videoAspectRatio() const
{
    QSettings settings;
    settings.beginGroup("display");

    Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatioByExpanding;
    QString aspectRatioModeString = settings.value("aspectRatioMode", "KeepAspectRatioByExpanding").toString();
    if (aspectRatioModeString == "KeepAspectRatio") {
        aspectRatioMode = Qt::KeepAspectRatio;
    }
    settings.endGroup();

    return aspectRatioMode;
}

void BigRingSettings::setVideoAspectRatio(Qt::AspectRatioMode aspectRatioMode)
{
    _settings.beginGroup("display");
    const QString aspectRatioModeSetting = (aspectRatioMode == Qt::KeepAspectRatio) ? "KeepAspectRatio" : "KeepAspectRatioByExpanding";
    _settings.setValue("aspectRatioMode", QVariant::fromValue(aspectRatioModeSetting));
    _settings.endGroup();
}

qreal BigRingSettings::maximumDownhillForSmartTrainer() const
{
    QSettings settings;
    settings.beginGroup("simulation");
    return settings.value("maximumDownhillForSmartTrainer", QVariant::fromValue(-DEFAULT_UP_AND_DOWNHILL_CAPS)).toDouble();
}

void BigRingSettings::setMaximumDownhillForSmartTrainer(const qreal maximumDownhill)
{
    _settings.beginGroup("simulation");
    _settings.setValue("maximumDownhillForSmartTrainer", QVariant::fromValue(maximumDownhill));
    _settings.endGroup();
}

int BigRingSettings::powerForElevationCorrection() const
{
    QSettings settings;
    settings.beginGroup("simulation");
    return settings.value("powerForElevationCorrection", QVariant::fromValue(0)).toInt();
}

void BigRingSettings::setPowerForElevationCorrection(const int powerForElevationCorrection)
{
    _settings.beginGroup("simulation");
    _settings.setValue("powerForElevationCorrection", QVariant::fromValue(powerForElevationCorrection));
    _settings.endGroup();
}

int BigRingSettings::difficultySetting() const
{
    QSettings settings;
    settings.beginGroup("simulation");
    return settings.value("difficultySetting", QVariant::fromValue(DEFAULT_DIFFICULTY_SETTING)).toInt();
}

void BigRingSettings::setDifficultySetting(const int percent)
{
    _settings.beginGroup("simulation");
    const int actualPercent = qBound(0, percent, 100);
    _settings.setValue("difficultySetting", QVariant::fromValue(actualPercent));
    _settings.endGroup();
}

qreal BigRingSettings::maximumUphillForSmartTrainer() const
{
    QSettings settings;
    settings.beginGroup("simulation");
    return settings.value("maximumUphillForSmartTrainer", QVariant::fromValue(DEFAULT_UP_AND_DOWNHILL_CAPS)).toDouble();
}

void BigRingSettings::setMaximumUphillForSmartTrainer(const qreal maximumUphill)
{
    _settings.beginGroup("simulation");
    _settings.setValue("maximumUphillForSmartTrainer", QVariant::fromValue(maximumUphill));
    _settings.endGroup();
}

QString BigRingSettings::clientId()
{
    if (!_settings.contains("clientId")) {
        const QUuid clientId = QUuid::createUuid();
        const QString asString = clientId.toRfc4122().toHex();
        _settings.setValue("clientId", QVariant::fromValue(asString));
    }
    return _settings.value("clientId").toString();
}
