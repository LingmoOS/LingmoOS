// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "applet.h"
#include "clipboarditem.h"
#include "pluginfactory.h"
#include "../constants.h"

#include <DDBusSender>
#include <DDciIcon>
#include <DGuiApplicationHelper>

#include <QGuiApplication>
#include <QBuffer>
#include <QDBusServiceWatcher>
#include <QDBusConnectionInterface>

DGUI_USE_NAMESPACE

namespace dock {

const QString clipboardService = "org.deepin.dde.Clipboard1";
const QString clipboardPath = "/org/deepin/dde/Clipboard1";
const QString clipboardInterface = "org.deepin.dde.Clipboard1";

static DDBusSender clipboardDbus()
{
    return DDBusSender().service(clipboardService)
        .path(clipboardPath)
        .interface(clipboardInterface);
}

static  QDBusServiceWatcher dbusWatcher(clipboardService, QDBusConnection::sessionBus(),
                                                QDBusServiceWatcher::WatchForOwnerChange);

ClipboardItem::ClipboardItem(QObject *parent)
    : DApplet(parent)
    , m_visible(true)
    , m_clipboardVisible(false)
{
    QDBusConnection::sessionBus().connect(clipboardService, clipboardPath, clipboardInterface,
                                          "clipboardVisibleChanged", this, SLOT(onClipboardVisibleChanged(bool)));

    auto func = [this](){
        QDBusInterface clipboardInter(clipboardService, clipboardPath, clipboardInterface, QDBusConnection::sessionBus());
        if (clipboardInter.isValid()) {
            m_clipboardVisible = clipboardInter.property("clipboardVisible").toBool();
        }
    };
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(clipboardService)) {
        func();
    }

    connect(&dbusWatcher, &QDBusServiceWatcher::serviceRegistered, this, [this, func](){
        func();
    });

    connect(&dbusWatcher, &QDBusServiceWatcher::serviceUnregistered, this, [this](){
        m_clipboardVisible = false;
    });
}

void ClipboardItem::toggleClipboard()
{
    clipboardDbus().method("Toggle").call();
}

DockItemInfo ClipboardItem::dockItemInfo()
{
    DockItemInfo info;
    info.name = "clipboard";
    info.displayName = tr("Clipboard");
    info.itemKey = "clipboard";
    info.settingKey = "clipboard";
    info.visible = m_visible;
    info.dccIcon = DCCIconPath + "clipboard.svg";
    return info;
}

void ClipboardItem::setVisible(bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;

        Q_EMIT visibleChanged(visible);
    }
}

void ClipboardItem::onClipboardVisibleChanged(bool visible)
{
    if (m_clipboardVisible != visible) {
        m_clipboardVisible = visible;
        Q_EMIT clipboardVisibleChanged(visible);
    }
}

D_APPLET_CLASS(ClipboardItem)
}


#include "clipboarditem.moc"
