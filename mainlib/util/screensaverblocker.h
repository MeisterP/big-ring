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

#ifndef SCREENSAVERBLOCKER_H
#define SCREENSAVERBLOCKER_H

#include <QtCore/QObject>
#include <QtWidgets/QWidget>

#ifdef Q_OS_LINUX
#include <QtCore/QProcess>
#endif

namespace indoorcycling
{
/**
 * Block the screen saver, so it will not be started when the user is
 * performing a training session.
 */
class ScreenSaverBlocker : public QObject
{
    Q_OBJECT
public:
    /**
     * Create a ScreenSaverBlocker blocker object. As long as the object is 'alive',
     * the screen saver will be blocked.
     * @param parent the widget that requests the screen saver to be blocked. Make
     * sure that this widget lives longer than the ScreenSaverBlocker instance.
     */
    explicit ScreenSaverBlocker(QWidget* parent);
    /**
     * Clean up, unblock the screen saver.
     */
    virtual ~ScreenSaverBlocker();

public slots:
#ifndef Q_OS_WIN
    void handleError(QProcess::ProcessError error);
    void resetScreenSaver();
    void resetScreenSaverForDesktopEnvironment(const QString &command);
#endif
private:
    void blockScreenSaver();
    void unblockScreenSaver();

    QWidget* const _window;
};
}
#endif // SCREENSAVERBLOCKER_H
