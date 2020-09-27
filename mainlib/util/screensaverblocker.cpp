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

#include "screensaverblocker.h"

#include <QtCore/QDebug>

#include <QtCore/QTimer>
#ifdef Q_OS_LINUX
#include <QtCore/QProcess>
#endif
#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace
{
const QString GNOME_RESET_SCREEEN_SAVER_COMMAND = "dbus-send --session --dest=org.gnome.ScreenSaver "
        "--type=method_call /org/gnome/ScreenSaver org.gnome.ScreenSaver.SimulateUserActivity";
const QString KDE_RESET_SCREEN_SAVER_COMMAND = "dcop kdesktop KScreensaverIface quit";
const QString XSCREENSAVER_RESET_SCREEN_SAVER_COMMAND = "xset s reset";
}
namespace indoorcycling
{
ScreenSaverBlocker::ScreenSaverBlocker(QWidget* parent) :
    QObject(parent), _window(parent)
{
    blockScreenSaver();
}

ScreenSaverBlocker::~ScreenSaverBlocker()
{
    unblockScreenSaver();
}

#ifdef Q_OS_LINUX

// On Linux, there seems to be no single way of blocking the screensaver.
// Using xdg-screensaver works on most systems, so we'll use that. This
// is an external process, which we'll call with a QProcess. It needs
// the window id of the window that requests the screen saver to be blocked.
// When unblocking, we call xdg-screensaver again, with the 'resume' command.
void ScreenSaverBlocker::blockScreenSaver()
{
    qDebug() << "Blocking screensaver.";
    QProcess* const process = new QProcess;

    const QString command = QString("xdg-screensaver suspend %1").arg(_window->winId());

    connect(process, SIGNAL(error(QProcess::ProcessError)), this,
            SLOT(handleError(QProcess::ProcessError)));
    // delete process when it's finished.
    connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));

    process->start(command);

    // we'll also start a timer to send an "activity signal" every 5 seconds.
    QTimer * const activityTimer = new QTimer(this);
    connect(activityTimer, &QTimer::timeout, this, &ScreenSaverBlocker::resetScreenSaver);
    activityTimer->setInterval(5000);
    activityTimer->start();
}

void ScreenSaverBlocker::unblockScreenSaver()
{
    qDebug() << "Allowing screensaver to work again";
    QProcess* const process = new QProcess;

    const QString command = QString("xdg-screensaver resume %1").arg(_window->winId());
    connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));
    process->start(command);
}

void ScreenSaverBlocker::resetScreenSaver()
{
    for (const QString &command: { GNOME_RESET_SCREEEN_SAVER_COMMAND, KDE_RESET_SCREEN_SAVER_COMMAND, XSCREENSAVER_RESET_SCREEN_SAVER_COMMAND}) {
        resetScreenSaverForDesktopEnvironment(command);
    }
}

/** Start a process to reset the screen saver for a desktop environment */
void ScreenSaverBlocker::resetScreenSaverForDesktopEnvironment(const QString &command)
{
    QProcess* const process = new QProcess;

    connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));
    process->start(command);
}
#else

#ifdef Q_OS_WIN

// On Windows, blocking the screen saver is easy. We call the
// SetThreadExecutionState function
// (https://msdn.microsoft.com/en-us/library/aa373208%28VS.85%29.aspx),
// with ES_DISPLAY_REQUIRED when we want to block the screen saver.
// When blocking is no longer needed, we call SetThreadExecutionState again,
// with only ES_CONTINUOUS.
void ScreenSaverBlocker::blockScreenSaver()
{
    SetThreadExecutionState(ES_DISPLAY_REQUIRED | ES_CONTINUOUS);
}

void ScreenSaverBlocker::unblockScreenSaver()
{
    SetThreadExecutionState(ES_CONTINUOUS);
}
#endif
#endif


#ifndef Q_OS_WIN
// this method will only be called on Linux, so we wont implement
// anything for Windows.
void ScreenSaverBlocker::handleError(QProcess::ProcessError error)
{
    QObject* const process = sender();
    switch(error) {
    case QProcess::FailedToStart:
        qDebug() << "screensaver blocker could not be started.";
        break;
    default:
        qDebug() << "screensaver blocker error.";
    }
    process->deleteLater();
}
#endif
}
