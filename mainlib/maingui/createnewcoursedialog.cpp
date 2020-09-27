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
#include "createnewcoursedialog.h"
#include "ui_createnewcoursedialog.h"

#include <QtWidgets/QPushButton>

#include "video/thumbnailer.h"
#include "generalgui/quantityprinter.h"

CreateNewCourseDialog::CreateNewCourseDialog(RealLifeVideo &rlv, QWidget *parent) :
    QDialog(parent), ui(new Ui::CreateNewCourseDIalog),
    _thumbnailer(new Thumbnailer(this)), _quantityPrinter(new QuantityPrinter(this)),
    _rlv(rlv)
{
    ui->setupUi(this);

    ui->startDistanceSlider->setMaximum(_endDistanceInMeters);
    ui->startDistanceSlider->setValue(_startDistanceInMeters);

    ui->endDistanceSlider->setMaximum(_endDistanceInMeters);
    ui->endDistanceSlider->setValue(_endDistanceInMeters);


    ui->startVideoScreenshotWidget->setVideo(rlv);
    ui->endVideoScreenshotWidget->setVideo(rlv);
    ui->endVideoScreenshotWidget->setDistance(rlv.totalDistance());
    ui->distanceValueLabel->setText(QString("%1 %2").arg(_quantityPrinter->printDistance(distanceInMeters()))
                               .arg(_quantityPrinter->unitString(QuantityPrinter::Quantity::Distance)));
    ui->altitudeProfileWidget->setVideo(rlv);
    ui->altitudeProfileWidget->setStartAndEndDistance(_startDistanceInMeters, _endDistanceInMeters);

    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
}

CreateNewCourseDialog::~CreateNewCourseDialog()
{
    delete ui;
}

int CreateNewCourseDialog::startDistanceInMeters() const
{
    return _startDistanceInMeters;
}

int CreateNewCourseDialog::endDistanceInMeters() const
{
    return _endDistanceInMeters;
}

const QString CreateNewCourseDialog::courseName() const
{
    return ui->courseNameEdit->text();
}

void CreateNewCourseDialog::on_startDistanceSlider_valueChanged(int value)
{
    if (value > _endDistanceInMeters) {
        ui->startDistanceSlider->setValue(_endDistanceInMeters);
        value = _endDistanceInMeters;
    }
    _startDistanceInMeters = value;
    updateUi();
}

void CreateNewCourseDialog::on_endDistanceSlider_valueChanged(int value)
{
    if (value < _startDistanceInMeters) {
        ui->endDistanceSlider->setValue(_startDistanceInMeters);
        value = _startDistanceInMeters;
    }
    _endDistanceInMeters = value;
    updateUi();
}

void CreateNewCourseDialog::on_startDistanceSlider_sliderReleased()
{
    ui->startVideoScreenshotWidget->setDistance(_startDistanceInMeters);
}

void CreateNewCourseDialog::on_endDistanceSlider_sliderReleased()
{
    ui->endVideoScreenshotWidget->setDistance(_endDistanceInMeters);
}

int CreateNewCourseDialog::distanceInMeters() const
{
    return _endDistanceInMeters - _startDistanceInMeters;
}

void CreateNewCourseDialog::updateUi()
{
    int distance = _endDistanceInMeters - _startDistanceInMeters;
    ui->distanceValueLabel->setText(QString("%1 %2").arg(_quantityPrinter->printDistance(distance))
                               .arg(_quantityPrinter->unitString(QuantityPrinter::Quantity::Distance)));
    ui->altitudeProfileWidget->setStartAndEndDistance(_startDistanceInMeters, _endDistanceInMeters);
}

void CreateNewCourseDialog::on_courseNameEdit_textChanged(const QString &courseName)
{
    if (!courseName.isEmpty()) {
        ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
    } else {
        ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
    }
}

