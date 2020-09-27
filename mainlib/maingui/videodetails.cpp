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
#include "videodetails.h"
#include "ui_videodetails.h"
#include <QtCore/QtDebug>
#include "createnewcoursedialog.h"
#include "generalgui/quantityprinter.h"

VideoDetails::VideoDetails(QWidget *parent) :
    QWidget(parent),
    _quantityPrinter(new QuantityPrinter(this)),
    ui(new Ui::VideoDetails)
{
    ui->setupUi(this);
}

VideoDetails::~VideoDetails()
{
    delete ui;
}

void VideoDetails::setVideo(RealLifeVideo &rlv)
{
    _currentRlv = rlv;
    ui->nameLabel->setText(rlv.name());

    ui->distanceLabel->setText(QString("%1 %2").arg(_quantityPrinter->printDistance(rlv.totalDistance())).arg(_quantityPrinter->unitString(QuantityPrinter::Quantity::Distance)));
    ui->videoScreenshotWidget->setVideo(rlv);
    ui->altitudeProfileWidget->setVideo(rlv);

    ui->courseListWidget->clear();
    for (const Course& course: rlv.courses()) {
        new QListWidgetItem(course.name(), ui->courseListWidget);
    }
    ui->courseListWidget->setCurrentRow(0);
}

void VideoDetails::on_startButton_clicked()
{
    emit playClicked(_currentRlv, ui->courseListWidget->currentRow());
}

void VideoDetails::on_courseListWidget_currentRowChanged(int currentRow)
{
    _courseIndex = currentRow;
    ui->altitudeProfileWidget->setCourseIndex(_courseIndex);
    if (currentRow >= 0) {
        ui->videoScreenshotWidget->setDistance(_currentRlv.courses()[currentRow].start());
        qDebug() << "course selected:" << _currentRlv.courses()[currentRow].name();
    }
}

void VideoDetails::on_newCourseButton_clicked()
{
    CreateNewCourseDialog createNewCourseDialog(_currentRlv);
    const int result = createNewCourseDialog.exec();
    if (result == QDialog::Accepted) {
        const int startDistanceOfCustomRun = createNewCourseDialog.startDistanceInMeters();
        const int endDistanceOfCustomRun = createNewCourseDialog.endDistanceInMeters();
        const QString courseName = createNewCourseDialog.courseName();
        _currentRlv.addCustomCourse(startDistanceOfCustomRun, endDistanceOfCustomRun, courseName);

        QSettings settings;
        settings.beginGroup(QString("%1.custom_courses").arg(_currentRlv.name()));
        settings.setValue(courseName, QVariant::fromValue(startDistanceOfCustomRun));
        settings.setValue(courseName + "_end", QVariant::fromValue(endDistanceOfCustomRun));
        settings.endGroup();

        ui->courseListWidget->clear();
        for (const Course& course: _currentRlv.courses()) {
            new QListWidgetItem(course.name(), ui->courseListWidget);
        }
        ui->courseListWidget->setCurrentRow(0);
    }
}
