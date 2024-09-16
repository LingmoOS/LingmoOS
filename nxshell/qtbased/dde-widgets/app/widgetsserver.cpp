// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgetsserver.h"
#include "widgetmanager.h"
#include "mainview.h"
#include "displaymodepanel.h"
#include "instancemodel.h"
#include "dbusserver_adaptor.h"
#include <QDebug>

#define DDE_WIDGETS_SERVICE "org.deepin.dde.Widgets1"

WIDGETS_FRAME_USE_NAMESPACE
WidgetsServer::WidgetsServer(QObject *parent)
    : QObject (parent)
    , m_manager (new WidgetManager())
{

}

WidgetsServer::~WidgetsServer()
{
    // WidgetManager need be destroyed before `View`, because IWidget may be released by QObject.
    delete m_manager;
    m_manager = nullptr;
    if (m_mainView) {
        m_mainView->deleteLater();
    }
}

bool WidgetsServer::registerService()
{
    (void) new DBusServer(this);
    auto bus = QDBusConnection::sessionBus();

    if (!bus.registerService(DDE_WIDGETS_SERVICE)) {
        qWarning() << QString("Can't register the %1 service, error:%2.").arg(DDE_WIDGETS_SERVICE).arg(bus.lastError().message());
        return false;
    }
    if (!bus.registerObject("/org/deepin/dde/Widgets1", this)) {
        qWarning() << QString("Can't register to the D-Bus object.");
        return false;
    }
    return true;
}

void WidgetsServer::start()
{
    m_manager->loadPlugins();
//    Show();
}

void WidgetsServer::Toggle()
{
    if (m_mainView && m_mainView->isVisible()) {
        Hide();
    } else {
        Show();
    }
}

void WidgetsServer::Show()
{
    qDebug(dwLog()) << "Show";
    if (!m_mainView) {
        m_mainView = new MainView(m_manager);

        m_mainView->init();
    }

    m_mainView->switchToDisplayMode();
    m_mainView->showView();
}

void WidgetsServer::Hide()
{
    qDebug(dwLog()) << "Hide";
    if (!m_mainView) {
        return;
    }

    m_mainView->hideView();
}

void WidgetsServer::SyncWidgets()
{
    qDebug(dwLog()) << "SyncWidgets";
    const auto removedPluginIds = m_manager->removingPlugins();
    for (auto pluginId : removedPluginIds) {
        m_mainView->removePlugin(pluginId);
    }

    const auto addedPluginIds = m_manager->addingPlugins();
    for (auto pluginId : addedPluginIds) {
        m_mainView->addPlugin(pluginId);
    }
    qDebug(dwLog()) << " removedPlugins:" << removedPluginIds
                    << "addedPlugins:" << addedPluginIds;
}
