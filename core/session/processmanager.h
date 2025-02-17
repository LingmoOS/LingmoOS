/*
 * Copyright (C) 2023-2024 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <QApplication>
#include <QAbstractNativeEventFilter>
#include <QObject>
#include <QProcess>
#include <QEventLoop>
#include <QMap>
#include <memory>

#include "daemon-helper.h"

class Application;
class ProcessManager : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit ProcessManager(Application *app, QObject *parent = nullptr);
    ~ProcessManager();

    void start();
    void logout();

    void startWindowManager();
    void startDesktopProcess();
    void startDaemonProcess();
    void checkAndDeactivateScreenSaver();

    /**
     * @brief Start the user defined autostart process.
     *        Typically, they are in <home>/.config/autostart/xxx.desktop
     */
    void loadAutoStartProcess();

    bool nativeEventFilter(const QByteArray & eventType, void * message, long * result) override;

private:
    Application *m_app;
    QMap<QString, QProcess *> m_systemProcess;
    QMap<QString, QProcess *> m_autoStartProcess;

    // Daemon helper for desktop components
    std::shared_ptr<LINGMO_SESSION::Daemon> m_desktopAutoStartD;

    // Daemon helper for other daemon components
    std::shared_ptr<LINGMO_SESSION::Daemon> m_daemonAutoStartD;

    // Daemon helper for User Auto Start Process
    std::shared_ptr<LINGMO_SESSION::Daemon> m_userAutoStartD;

    bool m_wmStarted;
    QEventLoop *m_waitLoop;
};

#endif // PROCESSMANAGER_H
