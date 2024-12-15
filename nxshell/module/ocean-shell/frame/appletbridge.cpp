// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appletbridge.h"
#include "containment.h"
#include "private/appletbridge_p.h"
#include "private/applet_p.h"

#include "pluginloader.h"
#include <QQueue>

DS_BEGIN_NAMESPACE

DAppletBridgePrivate::DAppletBridgePrivate(DAppletBridge *qq)
    : DTK_CORE_NAMESPACE::DObjectPrivate(qq)
{
}

DAppletBridgePrivate::~DAppletBridgePrivate()
{
}

QList<DApplet *> DAppletBridgePrivate::applets() const
{
    QList<DApplet *> applets;
    auto rootApplet = DPluginLoader::instance()->rootApplet();
    auto root = qobject_cast<DContainment *>(rootApplet);

    QQueue<DContainment *> containments;
    containments.enqueue(root);
    while (!containments.isEmpty()) {
        DContainment *containment = containments.dequeue();
        for (const auto applet : containment->applets()) {
            if (auto item = qobject_cast<DContainment *>(applet)) {
                containments.enqueue(item);
            }
            if (applet->pluginId() == m_pluginId)
                applets << applet;
        }
    }
    return applets;
}

DAppletBridge::DAppletBridge(const QString &pluginId, QObject *parent)
    : DAppletBridge(*new DAppletBridgePrivate(this), parent)
{
    d_func()->m_pluginId = pluginId;
}

DAppletBridge::DAppletBridge(DAppletBridgePrivate &dd, QObject *parent)
    : QObject(parent)
    , DObject(dd)
{
}

DAppletBridge::~DAppletBridge()
{
}

bool DAppletBridge::isValid() const
{
    D_DC(DAppletBridge);
    const auto plugin = DPluginLoader::instance()->plugin(d->m_pluginId);
    return plugin.isValid();
}

QString DAppletBridge::pluginId() const
{
    D_DC(DAppletBridge);
    return d->m_pluginId;
}

QList<DAppletProxy *> DAppletBridge::applets() const
{
    D_DC(DAppletBridge);
    if (!isValid())
        return {};
    QList<DAppletProxy *> ret;
    for (const auto item : d->applets()) {
        if (auto proxy = item->d_func()->appletProxy()) {
            ret << proxy;
        }
    }
    return ret;
}

DAppletProxy *DAppletBridge::applet() const
{
    D_DC(DAppletBridge);
    if (!isValid())
        return {};
    for (const auto item : d->applets()) {
        if (auto proxy = item->d_func()->appletProxy()) {
            return proxy;
        }
    }
    return nullptr;
}

DS_END_NAMESPACE
