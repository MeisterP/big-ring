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
#ifndef BIGRINGSETTINGS_H
#define BIGRINGSETTINGS_H

#include <QtCore/QSettings>

/**
 * Wrapper around QSettings, used for application specific settings.
 * Just create a BigRingSettings object on the stack and load and
 * save settings.
 */
class BigRingSettings
{
public:
    BigRingSettings();

    QStringList videoFolders() const;
    bool addVideoFolder(const QString &folder);
    void removeVideoFolder(const QString &folder);

    /** Get the absolute path to folder where tcx ride files are saved */
    QString tcxFolder() const;
    /** Set the absolute path to the folder where tcx ride files are saved */
    void setTcxFolder(const QString &folder);

    qreal userWeight() const;
    void setUserWeight(const qreal userWeight);

    qreal bikeWeight() const;
    void setBikeWeight(const qreal bikeWeight);

    int powerAveragingForDisplayMilliseconds() const;
    void setPowerAveragingForDisplayMilliseconds(const int averagingMilliseconds);

    /** Get the video display aspect ratio option */
    Qt::AspectRatioMode videoAspectRatio() const;
    void setVideoAspectRatio(Qt::AspectRatioMode aspectRatioMode);

    /** Maximum uphill percentage for smart trainers */
    qreal maximumUphillForSmartTrainer() const;
    void setMaximumUphillForSmartTrainer(const qreal maximumDownhill);

    /** Maximum downhill percentage for smart trainers */
    qreal maximumDownhillForSmartTrainer() const;
    void setMaximumDownhillForSmartTrainer(const qreal maximumDownhill);

    int powerForElevationCorrection() const;
    void setPowerForElevationCorrection(const int correction);

    int difficultySetting() const;
    void setDifficultySetting(const int percent);

    /** Get the unique id for this installation */
    QString clientId();
private:
    QSettings _settings;
};

#endif // BIGRINGSETTINGS_H
