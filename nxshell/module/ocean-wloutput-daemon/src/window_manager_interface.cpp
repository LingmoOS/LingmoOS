// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "window_manager_interface.h"
#include <QtDBus/QDBusMessage>
#include <QDBusInterface>

PlasmaWindowManagerInterface::PlasmaWindowManagerInterface(PlasmaWindowManagement* plasma_window_manager)
    : QDBusAbstractAdaptor(plasma_window_manager)
    , m_plasmaWindowManager(plasma_window_manager)
{
    setAutoRelaySignals(true);

    InitConnect();
}

PlasmaWindowManagerInterface::~PlasmaWindowManagerInterface()
{
//    if(m_plasmaWindowManager != nullptr)
//        m_plasmaWindowManager->deleteLater();
}

void PlasmaWindowManagerInterface::InitConnect()
{
    connect(m_plasmaWindowManager, &PlasmaWindowManagement::interfaceAboutToBeReleased, this, &PlasmaWindowManagerInterface::InterfaceAboutToBeReleased);
    connect(m_plasmaWindowManager, &PlasmaWindowManagement::interfaceAboutToBeDestroyed, this, &PlasmaWindowManagerInterface::InterfaceAboutToBeDestroyed);
    connect(m_plasmaWindowManager, &PlasmaWindowManagement::showingDesktopChanged, this, &PlasmaWindowManagerInterface::ShowingDesktopChanged);
    connect(m_plasmaWindowManager, &PlasmaWindowManagement::activeWindowChanged, this, &PlasmaWindowManagerInterface::ActiveWindowChanged);
}

void PlasmaWindowManagerInterface::windowAdd(const QString& dbus)
{
    m_windowsDbusPath.append(dbus);
    Q_EMIT WindowCreated(dbus);
    Q_EMIT ActiveWindowChanged();
}

void PlasmaWindowManagerInterface::windowRemove(const QString& dbus)
{
    m_windowsDbusPath.removeOne(dbus);
    Q_EMIT WindowRemove(dbus);
}

bool PlasmaWindowManagerInterface::IsValid() const
{
    if(m_plasmaWindowManager != nullptr)
        return m_plasmaWindowManager->isValid();

    return false;
}

bool PlasmaWindowManagerInterface::IsShowingDesktop() const
{
    if(m_plasmaWindowManager != nullptr)
        return m_plasmaWindowManager->isShowingDesktop();

    return false;
}

void PlasmaWindowManagerInterface::SetShowingDesktop(bool show)
{
    if(m_plasmaWindowManager != nullptr)
        m_plasmaWindowManager->setShowingDesktop(show);
}

void PlasmaWindowManagerInterface::ShowDesktop()
{
    if(m_plasmaWindowManager != nullptr)
        m_plasmaWindowManager->showDesktop();
}

void PlasmaWindowManagerInterface::HideDesktop()
{
    if(m_plasmaWindowManager != nullptr)
        m_plasmaWindowManager->hideDesktop();
}

QList<QVariant> PlasmaWindowManagerInterface::Windows() const
{
    return m_windowsDbusPath;
}

quint32 PlasmaWindowManagerInterface::ActiveWindow() const
{
    if(m_plasmaWindowManager != nullptr && m_plasmaWindowManager->activeWindow()) {
        return m_plasmaWindowManager->activeWindow()->internalId();
    }

    return 0;
}
