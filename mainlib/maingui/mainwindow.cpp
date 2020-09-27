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

#include "mainwindow.h"

#include <functional>

#include <QtCore/QtDebug>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QVBoxLayout>

#include "ui_aboutdialog.h"
#include "videolistview.h"
#include "settingsdialog.h"
#include "ant/antcentraldispatch.h"
#include "model/cyclist.h"
#include "model/simulation.h"
#include "network/versionchecker.h"
#include "ridegui/run.h"
#include "ridegui/newvideowidget.h"


MainWindow::MainWindow(bool showDebugOutput, QWidget *parent) :
    QWidget(parent, Qt::Window),
    _antCentralDispatch(new indoorcycling::AntCentralDispatch(this)),
    _menuBar(new QMenuBar),
    _stackedWidget(new QStackedWidget),
    _showDebugOutput(showDebugOutput),
    _listView(new VideoListView(this))
{
    Q_INIT_RESOURCE(icons);
    _antCentralDispatch->initialize();
    setupMenuBar();
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    _stackedWidget->addWidget(_listView);

    layout->addWidget(_menuBar);
    layout->addWidget(_stackedWidget);

    connect(_listView, &VideoListView::videoSelected, _listView, [=](RealLifeVideo& rlv, int courseNr) {
        startRun(rlv, courseNr);
    });
}

MainWindow::~MainWindow()
{
    // empty
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "received key" << event->key() << event->text();

    if (!_run || !_videoWidget->handleKeyPress(event)) {
        switch(event->key()) {
        case Qt::Key_F:
            if (_run) {
                if (isFullScreen()) {
                    showNormal();
                    setGeometry(_savedGeometry);
                    _menuBar->show();
                } else {
                    _savedGeometry = geometry();
                    if (_run) {
                        _menuBar->hide();
                        showFullScreen();
                    }
                }
            }
            break;
        case Qt::Key_M:
            if (isMaximized()) {
                showNormal();
            } else {
                showMaximized();
            }
            break;
        case Qt::Key_Escape:
            if (_run) {
                _run->handleStopRun(this);
            }
        default:
            QWidget::keyPressEvent(event);
            return;
            break;
        }
    }
    event->accept();
}

void MainWindow::importFinished(RealLifeVideoList rlvs)
{
    _listView->setVideos(rlvs);
}

void MainWindow::removeDisplayMessage()
{
    if (_videoWidget) {
        _videoWidget->displayMessage("");
    }
}

// We'll only show the new version information if there's no run currently under way
void MainWindow::newVersionAvailable(bool newVersion, const QString &version)
{
    if (newVersion && !_run) {
        const QString detailedText = tr("There is a new version of Big Ring available. Download version %1 from "
                                        "<a href='https://github.com/MeisterP/big-ring/releases/'>https://github.com/MeisterP/big-ring/releases/</a>.").arg(version);
        QMessageBox informationBox(this);
        informationBox.setWindowTitle(tr("New version available"));
        informationBox.setTextFormat(Qt::RichText);
        informationBox.setIcon(QMessageBox::Information);
        informationBox.setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        informationBox.setText(detailedText);
        informationBox.exec();
    }
}

/**
 * Asynchronously load all videos.
 */
void MainWindow::loadVideos()
{
    if (BigRingSettings().videoFolders().isEmpty()) {
        QMessageBox::StandardButton choice = QMessageBox::question(
                    this, tr("Video folder not configured"),
                    tr("No location was configured for the Real Life Videos.\n"
                       "Do you want to open Preferences now?"),
                    QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
                    QMessageBox::Yes);
        if (choice == QMessageBox::Yes) {
            _showPreferencesAction->trigger();
        }
        return;
    }

    RealLifeVideoImporter *importer = new RealLifeVideoImporter(this);

    QProgressDialog *progressDialog = new QProgressDialog("Importing Videos", QString(), 0, 0, this);

    connect(importer, &RealLifeVideoImporter::importFinished, this, [=](RealLifeVideoList list) {
        this->importFinished(list);
        importer->deleteLater();
        progressDialog->deleteLater();
    });
    connect(importer, &RealLifeVideoImporter::rlvFilesFound, importer, [progressDialog](int nr) {
        progressDialog->setMaximum(nr);
        progressDialog->setValue(0);
    });
    connect(importer, &RealLifeVideoImporter::rlvImported, importer, [progressDialog]() {
        qDebug() << "rlv imported!" << progressDialog->maximum() << progressDialog->value();
        int currentValue = progressDialog->value();
        progressDialog->setValue(currentValue + 1);
    });
    progressDialog->setValue(0);
    importer->importRealLiveVideoFilesFromDir();
    progressDialog->exec();
}

void MainWindow::setupMenuBar()
{
    QMenu* fileMenu = _menuBar->addMenu(tr("File"));

    _showPreferencesAction = new QAction(tr("Preferences"), this);
    connect(_showPreferencesAction, &QAction::triggered, _showPreferencesAction, [=]() {
        std::function<void(void)>  videoLoadFunction = [=]() -> void {
            this->loadVideos();
        };
        SettingsDialog dialog(_antCentralDispatch, videoLoadFunction, this);
        dialog.exec();
        update();
    });
    fileMenu->addAction(_showPreferencesAction);

    QAction* quitAction = new QAction(tr("&Quit"), this);
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(quitAction);
    this->addAction(quitAction);

    QMenu* helpMenu = _menuBar->addMenu(tr("Help"));
    QAction* aboutBigRingAction = new QAction(tr("About Big Ring"), this);
    connect(aboutBigRingAction, &QAction::triggered, this, [=]() {
        QDialog dialog;
        Ui::BigRingAboutDialog aboutDialog;
        aboutDialog.setupUi(&dialog);
        dialog.exec();
    });
    helpMenu->addAction(aboutBigRingAction);
    QAction* aboutQtAction = new QAction(tr("About Qt"), this);
    connect(aboutQtAction, &QAction::triggered, this, [=]() {
        QMessageBox::aboutQt(this);
    });
    helpMenu->addAction(aboutQtAction);
}

void MainWindow::startRun(RealLifeVideo rlv, int courseNr)
{
    Course course = rlv.courses()[courseNr];
    _run = make_qobject_unique(new Run(_antCentralDispatch, rlv, course));
    _videoWidget.reset(new NewVideoWidget(_showDebugOutput));
    _videoWidget->setRealLifeVideo(rlv);
    _videoWidget->setCourseIndex(courseNr);
    _stackedWidget->addWidget(_videoWidget.data());
    _videoWidget->setSimulation(_run->simulation());

    _stackedWidget->setCurrentIndex(_stackedWidget->indexOf(_videoWidget.data()));
    _savedGeometry = geometry();
    if (isMaximized()) {
        _menuBar->hide();
        _guiFullScreen = true;
        showFullScreen();
    } else {
        _guiFullScreen = false;
    }
    connect(&_run->simulation().cyclist(), &Cyclist::distanceChanged, _videoWidget.data(), &NewVideoWidget::setDistance);
    connect(_videoWidget.data(), &NewVideoWidget::readyToPlay, this, [this](bool ready) {
        if (ready) {
            _run->start();
            _videoWidget->displayMessage(tr("Begin cyling to start ride"));
        }
    });
    connect(_run.get(), &Run::newInformationMessage, _videoWidget.data(), &NewVideoWidget::displayInformationBox);
    connect(_run.get(), &Run::stopped, _run.get(), [this]() {
        _run.reset();
        _stackedWidget->setCurrentIndex(_stackedWidget->indexOf(_listView));
        _stackedWidget->removeWidget(_videoWidget.data());
        _videoWidget.reset();
        _menuBar->show();
        if (_guiFullScreen) {
            showMaximized();
        } else {
            showNormal();
            setGeometry(_savedGeometry);
        }
        setGeometry(_savedGeometry);
        raise();
    });
    connect(_run.get(), &Run::riding, this, &MainWindow::removeDisplayMessage);
    connect(_run.get(), &Run::finished, _run.get(), [this]() {
        const QString text = QString("Ride Finished<br>Finished in %1").arg(_run->time().toString());
        _videoWidget->displayMessage(text);
        QTimer::singleShot(10000, this, SLOT(removeDisplayMessage()));
    });
    connect(_run.get(), &Run::paused, _run.get(), [this]() {
        const QString text = QString("Paused").arg(_run->time().toString());
        _videoWidget->displayMessage(text);
    });
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // if the user chooses not stop the run, just ignore the event.
    if (_run && !_run->handleStopRun(this)) {
        event->ignore();
        return;
    }
    qDebug() << "closing main window";
    event->accept();
}

void MainWindow::showEvent(QShowEvent *showEvent)
{
    QWidget::showEvent(showEvent);
    if (!showEvent->spontaneous()) {
        QTimer::singleShot(0, this, &MainWindow::initialize);
    }
}

void MainWindow::initialize()
{
    loadVideos();
    VersionChecker *versionChecker = new VersionChecker(this);
    connect(versionChecker, &VersionChecker::newVersionAvailable, this, &MainWindow::newVersionAvailable);
    connect(versionChecker, &VersionChecker::newVersionAvailable, versionChecker, &VersionChecker::deleteLater);
    versionChecker->checkForNewVersion();
}
