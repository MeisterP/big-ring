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
#ifndef CREATENEWCOURSEDIALOG_H
#define CREATENEWCOURSEDIALOG_H


#include <QtWidgets/QDialog>
#include "model/reallifevideo.h"

class QuantityPrinter;
class Thumbnailer;

namespace Ui {
class CreateNewCourseDIalog;
}

class CreateNewCourseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateNewCourseDialog(RealLifeVideo& rlv, QWidget *parent = 0);
    ~CreateNewCourseDialog();

    int startDistanceInMeters() const;
    int endDistanceInMeters() const;
    const QString courseName() const;

private slots:
    void on_startDistanceSlider_valueChanged(int value);

    void on_startDistanceSlider_sliderReleased();

    void on_courseNameEdit_textChanged(const QString &arg1);

    void on_endDistanceSlider_valueChanged(int value);

    void on_endDistanceSlider_sliderReleased();

private:
    int distanceInMeters() const;
    void updateUi();
    Ui::CreateNewCourseDIalog *ui;
    Thumbnailer* _thumbnailer;
    QuantityPrinter* _quantityPrinter;
    RealLifeVideo _rlv;
    int _startDistanceInMeters = 0;
    int _endDistanceInMeters = static_cast<int>(_rlv.totalDistance());


};

#endif // CREATENEWCOURSEDIALOG_H
