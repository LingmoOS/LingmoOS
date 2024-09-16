// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trayplugin.h"
#include "constants.h"
#include "traywidget.h"
#include "sniprotocolhandler.h"
#include "util.h"
#include "xembedprotocolhandler.h"
#include "ddeindicatortrayprotocol.h"
#include "pluginproxyinterface.h"

#include <QDebug>

namespace tray {
TrayPlugin::TrayPlugin(QObject *parent)
{
    qRegisterMetaType<TrayList>("TrayList");
    qDBusRegisterMetaType<TrayList>();

    registerDBusToolTipMetaType();
    registerDBusImageListMetaType();
}

TrayPlugin::~TrayPlugin()
{
}

const QString TrayPlugin::pluginName() const
{
    return "application-tray";
}
    
const QString TrayPlugin::pluginDisplayName() const
{
    return tr("Application Tray");
}

QWidget *TrayPlugin::itemTipsWidget(const QString &itemKey)
{
    return m_tooltip.value(itemKey);
}

void TrayPlugin::init(PluginProxyInterface *proxyInter)
{
    m_proyInter = proxyInter;

    auto sniProtocol = new SniTrayProtocol();
    auto xembedProtocol = new XembedProtocol();
    auto indicatorProtocol = new DDEindicatorProtocol();

    connect(sniProtocol, &SniTrayProtocol::trayCreated, this, &TrayPlugin::onTrayhandlerCreatd);
    connect(indicatorProtocol, &SniTrayProtocol::trayCreated, this, &TrayPlugin::onTrayhandlerCreatd);
    if (UTIL->isXAvaliable()) {
        connect(xembedProtocol, &SniTrayProtocol::trayCreated, this, &TrayPlugin::onTrayhandlerCreatd);
    } else {
        xembedProtocol->deleteLater();
    }
}

QWidget *TrayPlugin::itemWidget(const QString &itemKey)
{
    return m_widget.value(itemKey);
}

Dock::PluginFlags TrayPlugin::flags() const
{
    return Dock::Attribute_CanDrag | Dock::Attribute_CanInsert | Dock::Type_Tray;
}

void TrayPlugin::onTrayhandlerCreatd(QPointer<AbstractTrayProtocolHandler> handler)
{
    auto id = handler->id();
    m_widget.insert(id, new TrayWidget(handler));

    auto remove = [this, id]() {
        m_proyInter->itemRemoved(this, id);
        auto widget = m_widget.value(id);
        if (widget) {
            widget->deleteLater();
        }

        m_widget.remove(id);
    };

    auto showWidget = [this, handler, id](){
        if (handler->enabled()) {
            m_proyInter->itemAdded(this, id);
        } else {
            m_proyInter->itemRemoved(this, id);
        }
    };

    auto getToolTip = [this, handler, id] {
        if (handler->tooltip()) {
            m_tooltip.insert(id, handler->tooltip());
        }
    };

    connect(handler.data(), &QObject::destroyed, this, remove);
    connect(handler.data(), &AbstractTrayProtocolHandler::enabledChanged, this, showWidget);
    connect(handler.data(), &AbstractTrayProtocolHandler::tooltiChanged, this, getToolTip);
    showWidget();
    getToolTip();
}
}
