// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FCITX5ADDONSPROXY_H
#define FCITX5ADDONSPROXY_H

#include <addonmodel.h>

#include <QMap>
#include <QObject>
#include <QVariant>

namespace fcitx::kcm {
class DBusProvider;
}

class QDBusPendingCallWatcher;

namespace lingmo {
namespace fcitx5configtool {
class Fcitx5AddonsProxyPrivate;
class Fcitx5AddonsProxy : public QObject
{
    Q_OBJECT

public:
    explicit Fcitx5AddonsProxy(fcitx::kcm::DBusProvider *dbus, QObject *parent = nullptr);
    ~Fcitx5AddonsProxy();

    void clear();

    Q_INVOKABLE void load();
    Q_INVOKABLE QVariantList globalAddons() const;
    Q_INVOKABLE void setEnableAddon(const QString &addon, bool enable);

Q_SIGNALS:
    void requestAddonsFinished();

private:
    friend class Fcitx5AddonsProxyPrivate;
    Fcitx5AddonsProxyPrivate *const d;
};
}   // namespace fcitx5configtool
}   // namespace lingmo 

#endif // !FCITX5ADDONSPROXY_H
