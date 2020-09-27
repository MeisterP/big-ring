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
#ifndef VIDEODETAILS_H
#define VIDEODETAILS_H

#include <QWidget>

#include "model/reallifevideo.h"

class QuantityPrinter;

namespace Ui {
class VideoDetails;
}

class VideoDetails : public QWidget
{
    Q_OBJECT

public:
    explicit VideoDetails(QWidget *parent = 0);
    ~VideoDetails();

public slots:
    void setVideo(RealLifeVideo& rlv);
signals:
    void playClicked(RealLifeVideo& rlv, int courseNr);
private slots:
    void on_startButton_clicked();

    void on_courseListWidget_currentRowChanged(int currentRow);

    void on_newCourseButton_clicked();

private:
    RealLifeVideo _currentRlv;
    int _courseIndex;
    QuantityPrinter* const _quantityPrinter;
    Ui::VideoDetails *ui;
};

#endif // VIDEODETAILS_H
