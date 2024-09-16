// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLASMA_WINDOW_MANAGER_INTERFACE_H
#define PLASMA_WINDOW_MANAGER_INTERFACE_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QtDBus/QtDBus>
#include <QtDBus/QDBusConnection>

#include <DWayland/Client/plasmawindowmanagement.h>

using namespace KWayland::Client;

const QString MANAGER_PATH = "/org/deepin/dde/KWayland1/WindowManager";

class PlasmaWindowManagerInterface : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.dde.KWayland1.WindowManager")

public:
    explicit PlasmaWindowManagerInterface(PlasmaWindowManagement* plasma_window_manager);
    virtual ~PlasmaWindowManagerInterface();
    bool InitDBus();
    void windowAdd(const QString& dbus);
    void windowRemove(const QString& dbus);

public Q_SLOTS:
    bool IsValid() const;
    bool IsShowingDesktop() const;
    void SetShowingDesktop(bool show);
    void ShowDesktop();
    void HideDesktop();
    QList<QVariant> Windows() const;
    quint32 ActiveWindow() const;

signals:
    void InterfaceAboutToBeReleased();
    void InterfaceAboutToBeDestroyed();
    void ShowingDesktopChanged(bool);
    void WindowCreated(const QString& dbus);
    void WindowRemove(const QString& dbus);
    void ActiveWindowChanged();

private:
    void InitConnect();

private:
    PlasmaWindowManagement* m_plasmaWindowManager{nullptr};
    QList<QVariant> m_windowsDbusPath;
};

#endif // PLASMA_WINDOW_MANAGER_INTERFACE_H
