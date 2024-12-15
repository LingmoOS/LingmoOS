// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FCITX5ADDONSPROXY_P_H
#define FCITX5ADDONSPROXY_P_H

#include <fcitxqtdbustypes.h>

#include <QObject>

class QDBusPendingCallWatcher;

namespace fcitx::kcm {
class DBusProvider;
class AddonModel;
class AddonProxyModel;
}

namespace lingmo {
namespace fcitx5configtool {
class Fcitx5AddonsProxy;
class Fcitx5AddonsProxyPrivate : public QObject
{
    Q_OBJECT
    friend class Fcitx5AddonsProxy;
    Fcitx5AddonsProxy *const q;

public:
    explicit Fcitx5AddonsProxyPrivate(Fcitx5AddonsProxy *parent, fcitx::kcm::DBusProvider *dbus);

private Q_SLOTS:
    void fetchAddonsFinished(QDBusPendingCallWatcher *);

private:
    fcitx::kcm::DBusProvider *dbusprovider;

    QMap<QString, fcitx::FcitxQtAddonInfoV2> nameToAddonMap;
    QMap<QString, QStringList> reverseDependencies;
    QMap<QString, QStringList> reverseOptionalDependencies;
    QList<QPair<int, QStringList>> addonEntryList;
};
}   // namespace fcitx5configtool
}   // namespace lingmo

#endif // FCITX5ADDONSPROXY_P_H 
