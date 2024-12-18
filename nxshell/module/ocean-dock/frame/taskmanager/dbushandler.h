// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSHANDLER_H
#define DBUSHANDLER_H

#include "com_lingmo_wm.h"
#include "org_lingmo_ocean_launcher1.h"
#include "org_lingmo_ocean_wmswitcher1.h"
#include "org_lingmo_ocean_xeventmonitor1.h"
#include "org_lingmo_ocean_kwayland_windowmanager.h"
#include "org_lingmo_ocean_kwayland_plasmawindow.h"

#include "windowinfok.h"

#include <QObject>
#include <QDBusConnection>
#include <QDBusMessage>

class TaskManager;

// 处理DBus交互
class DBusHandler : public QObject
{
    Q_OBJECT
public:
    explicit DBusHandler(TaskManager *taskmanager, QObject *parent = nullptr);

    /************************* KWayland/WindowManager ***************************/
    void listenWaylandWMSignals();
    void loadClientList();

    bool wlShowingDesktop();
    uint wlActiveWindow();

    /************************* WMSwitcher ***************************/
    QString getCurrentWM();

    /************************* StartManager ***************************/
    void launchApp(QString desktopFile, uint32_t timestamp, QStringList files);
    void launchAppAction(QString desktopFile, QString action, uint32_t timestamp);
    void launchAppUsingApplication1Manager(QString desktopFile, uint32_t timestamp, QStringList files);
    void launchAppActionUsingApplication1Manager(QString desktopFile, QString action, uint32_t timestamp);
    void launchAppUsingApplicationManager1(QString desktopFile, uint32_t timestamp, QStringList files);
    void launchAppActionUsingApplicationManager1(QString desktopFile, QString action, uint32_t timestamp);

    /************************* AlRecorder1 ***************************/
    void markAppLaunched(const QString &filePath);

    /************************* KWayland.PlasmaWindow ***************************/
    void listenKWindowSignals(WindowInfoK *windowInfo);
    void removePlasmaWindowHandler(PlasmaWindow *window);

    PlasmaWindow *createPlasmaWindow(QString objPath);

    /************************* WM ***************************/
    void presentWindows(QList<uint> windows);
    void previewWindow(uint xid);
    void cancelPreviewWindow();

    /************************* bamf ***************************/
    // XWindow -> desktopFile
    QString getDesktopFromWindowByBamf(XWindow windowId);

    bool newStartManagerAvaliable();
    void sendFailedDockNotification(const QString &appName);

Q_SIGNALS:
    void appUninstalled(const QDBusObjectPath &objectPath, const QStringList &interfaces);

private Q_SLOTS:
    void handleWlActiveWindowChange();
    void onActiveWindowButtonRelease(int type, int x, int y, const QString &key);

private:
    QString desktopEscapeToObjectPath(QString desktopFilePath);

private:
    QString m_activeWindowMonitorKey;
    TaskManager *m_taskmanager;

    com::lingmo::wm *m_wm;
    org::lingmo::ocean::WMSwitcher1 *m_wmSwitcher;
    org::lingmo::ocean::kwayland1::WindowManager *m_kwaylandManager;
    org::lingmo::ocean::XEventMonitor1 *m_xEventMonitor;
    org::lingmo::ocean::Launcher1 *m_launcher;

    std::once_flag m_isNewStartManagerAvaliableInited;
};

#endif // DBUSHANDLER_H
