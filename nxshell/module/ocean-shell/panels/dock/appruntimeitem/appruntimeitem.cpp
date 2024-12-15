// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appruntimeitem.h"
#include "applet.h"
#include "pluginfactory.h"

#include "../constants.h"
#include <DDBusSender>
#include <DDciIcon>
#include <DGuiApplicationHelper>

#include <QGuiApplication>
#include <QBuffer>
#include<QQuickView>
#include<QQmlContext>
#include <QtQml>

DGUI_USE_NAMESPACE

namespace dock {
AppRuntimeItem::AppRuntimeItem(QObject *parent)
    : DApplet(parent)
    , m_visible(true)
    , m_appRuntimeVisible(false){}

void AppRuntimeItem::toggleruntimeitem()
{
    m_xcbGetInfo.reset(new XcbGetInfo());
    m_windowManager.reset(new WindowManager());
    connect(m_xcbGetInfo.data(), &XcbGetInfo::windowInfoChangedForeground,
            m_windowManager.data(), &WindowManager::setWindowInfoForeground,
            Qt::QueuedConnection);
    connect(m_xcbGetInfo.data(), &XcbGetInfo::windowInfoChangedBackground,
            m_windowManager.data(), &WindowManager::setWindowInfoBackground,
            Qt::QueuedConnection);
    connect(m_xcbGetInfo.data(), &XcbGetInfo::windowDestroyChanged,
            m_windowManager.data(), &WindowManager::WindowDestroyInfo,
            Qt::QueuedConnection);
    connect(m_xcbGetInfo.data(), &XcbGetInfo::windowLeaveChangedInactiveName,
            m_windowManager.data(), &WindowManager::setWindowInfoInActive,
            Qt::QueuedConnection);
    connect(m_xcbGetInfo.data(), &XcbGetInfo::windowEnterChangedActiveName,
            m_windowManager.data(), &WindowManager::setWindowInfoActive,
            Qt::QueuedConnection);
    qmlRegisterSingletonInstance<XcbGetInfo>("XcbModule", 1, 0, "XcbGetInstance", m_xcbGetInfo.data());
    qmlRegisterSingletonInstance<WindowManager>("WindowModule", 1, 0, "WindowManagerInstance", m_windowManager.data());
}

DockItemInfo AppRuntimeItem::dockItemInfo()
{
    DockItemInfo info;
    info.name = "appruntime";
    info.displayName = tr("appruntime");
    info.itemKey = "appruntime";
    info.settingKey = "appruntime";
    info.visible = m_visible;
    info.oceanuiIcon = DCCIconPath + "appruntime.svg";
    return info;
}
void AppRuntimeItem::setVisible(bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;

        Q_EMIT visibleChanged(visible);
    }
}

void AppRuntimeItem::onappruntimeVisibleChanged(bool visible)
{
    if (m_appRuntimeVisible != visible) {
        m_appRuntimeVisible = visible;
        Q_EMIT appruntimeVisibleChanged(visible);
    }
}

D_APPLET_CLASS(AppRuntimeItem)
}

#include "appruntimeitem.moc"
