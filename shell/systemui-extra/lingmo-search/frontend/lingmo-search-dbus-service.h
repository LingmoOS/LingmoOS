/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *
 */
#ifndef LINGMOSEARCHDBUSSERVICE_H
#define LINGMOSEARCHDBUSSERVICE_H

#include <QDBusConnection>
#include <QObject>
#include <QDBusContext>

#include "mainwindow.h"
#include "service_interface.h"

class ServiceAdaptor;
namespace LingmoUISearch {

class LingmoUISearchDbusServices: public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface","org.lingmo.search.service")

public:
    explicit LingmoUISearchDbusServices(MainWindow *m, QObject *parent = nullptr);

public Q_SLOTS:
    void showWindow();
    void searchKeyword(QString keyword);
    void mainWindowSwitch();

Q_SIGNALS:
    void showWindowSignal(const QString &display);
    void searchKeywordSignal(const QString &display, QString keyword);
    void mainWindowSwitchSignal(const QString &display);

private Q_SLOTS:
    void onShowWindow(const QString &display);
    void onSearchKeyword(const QString &display, const QString &keyword);
    void onMainWindowSwitch(const QString &display);

private:
    void initWatcher();
    bool registerService();
    void onServiceOwnerChanged(const QString &service, const QString &oldOwner, const QString &newOwner);
    void connectToService();
    void disConnectToService();
    QString checkDisplay();
    QString displayFromPid(uint pid);

    MainWindow *m_mainWindow = nullptr;
    QDBusServiceWatcher *m_watcher = nullptr;
    QString m_display;
    OrgLingmoUISearchServiceInterface *m_serviceIface = nullptr;
    ServiceAdaptor * m_adaptor = nullptr;


};
}

#endif // LINGMOSEARCHDBUSSERVICE_H
