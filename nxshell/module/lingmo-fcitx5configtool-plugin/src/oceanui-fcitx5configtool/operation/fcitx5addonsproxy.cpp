// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fcitx5addonsproxy.h"
#include "private/fcitx5addonsproxy_p.h"

#include <dbusprovider.h>
#include <varianthelper.h>

#include <fcitx-utils/standardpath.h>

#include <QDBusPendingCallWatcher>

using namespace lingmo::fcitx5configtool;

Fcitx5AddonsProxyPrivate::Fcitx5AddonsProxyPrivate(Fcitx5AddonsProxy *parent, fcitx::kcm::DBusProvider *dbus)
    : q(parent), dbusprovider(dbus)
{
}

void Fcitx5AddonsProxyPrivate::fetchAddonsFinished(QDBusPendingCallWatcher *watcher)
{
    watcher->deleteLater();

    if (watcher->isError()) {
        return;
    }
    QDBusPendingReply<fcitx::FcitxQtAddonInfoV2List> reply(*watcher);
    nameToAddonMap.clear();
    reverseDependencies.clear();
    reverseOptionalDependencies.clear();
    auto list = reply.value();
    for (const auto &addon : list) {
        nameToAddonMap[addon.uniqueName()] = addon;
    }

    addonEntryList.clear();
    QMap<int, int> addonCategoryMap;
    for (const auto &addon : list) {
        for (const auto &dep : addon.dependencies()) {
            if (!nameToAddonMap.contains(dep)) {
                continue;
            }
            reverseDependencies[dep].append(addon.uniqueName());
        }
        for (const auto &dep : addon.optionalDependencies()) {
            if (!nameToAddonMap.contains(dep)) {
                continue;
            }
            reverseOptionalDependencies[dep].append(addon.uniqueName());
        }

        int idx;
        if (!addonCategoryMap.contains(addon.category())) {
            idx = addonEntryList.count();
            addonCategoryMap[addon.category()] = idx;
            addonEntryList.append(QPair<int, QStringList>(addon.category(), QStringList()));
        } else {
            idx = addonCategoryMap[addon.category()];
        }
        addonEntryList[idx].second.append(addon.uniqueName());
    }

    Q_EMIT q->requestAddonsFinished();
}

Fcitx5AddonsProxy::Fcitx5AddonsProxy(fcitx::kcm::DBusProvider *dbus, QObject *parent)
    : QObject(parent), d(new Fcitx5AddonsProxyPrivate(this, dbus))
{
}

Fcitx5AddonsProxy::~Fcitx5AddonsProxy() = default;

void Fcitx5AddonsProxy::clear()
{
    d->nameToAddonMap.clear();
    d->reverseDependencies.clear();
    d->reverseOptionalDependencies.clear();
    d->addonEntryList.clear();
}

void Fcitx5AddonsProxy::load()
{
    if (!d->dbusprovider->controller()) {
        return;
    }

    auto call = d->dbusprovider->controller()->GetAddonsV2();
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, d,
            &Fcitx5AddonsProxyPrivate::fetchAddonsFinished);
}

QVariantList Fcitx5AddonsProxy::globalAddons() const
{
    QVariantList ret;
    for (const auto &entry : d->addonEntryList) {
        for (const auto &addonUniqueName : entry.second) {
            const auto &addon = d->nameToAddonMap[addonUniqueName];
            QVariantMap addonMap;
            addonMap["name"] = addon.name();
            addonMap["uniqueName"] = addon.uniqueName();
            addonMap["comment"] = addon.comment();
            addonMap["category"] = addon.category();
            addonMap["configurable"] = addon.configurable();
            addonMap["enabled"] = addon.enabled();
            addonMap["onDemand"] = addon.onDemand();
            addonMap["dependencies"] = addon.dependencies();
            addonMap["optionalDependencies"] = addon.optionalDependencies();
            ret.append(QVariant::fromValue(addonMap));
        }
    }
    return ret;
}

void Fcitx5AddonsProxy::setEnableAddon(const QString &addonStr, bool enable)
{
    if (!d->nameToAddonMap.contains(addonStr)) {
        return;
    }
    
    fcitx::FcitxQtAddonStateList list;
    auto &addon = d->nameToAddonMap[addonStr];
    addon.setEnabled(enable);
    fcitx::FcitxQtAddonState state;
    state.setUniqueName(addon.uniqueName());
    state.setEnabled(enable);
    list.append(state);
                
    if (list.size()) {
        d->dbusprovider->controller()->SetAddonsState(list);
        QProcess p;
        p.start("killall", QStringList() << "fcitx5");
        p.waitForFinished();
        QProcess::startDetached(QString::fromStdString(fcitx::StandardPath::fcitxPath("bindir", "fcitx5")), QStringList());
    }
}
