// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FCITX5CONFIGPROXY_P_H
#define FCITX5CONFIGPROXY_P_H

#include <fcitxqtdbustypes.h>

#include <QObject>

class QTimer;

namespace fcitx::kcm {
class DBusProvider;
}

namespace lingmo {
namespace fcitx5configtool {
class Fcitx5ConfigProxy;
class Fcitx5ConfigProxyPrivate : public QObject
{
    Q_OBJECT
    friend class Fcitx5ConfigProxy;
    Fcitx5ConfigProxy *const q;

public:
    explicit Fcitx5ConfigProxyPrivate(Fcitx5ConfigProxy *parent,
                                     fcitx::kcm::DBusProvider *dbus,
                                     const QString &path);
    QStringList formatKey(const QString &shortcut);
    QString formatKeys(const QStringList &keys);
    QVariant readDBusValue(const QVariant &value);

private:
    fcitx::kcm::DBusProvider *dbusprovider;
    QString path;
    QVariantMap configValue;
    fcitx::FcitxQtConfigTypeList configTypes;
    QTimer *timer {nullptr};
};
}   // namespace fcitx5configtool
}   // namespace lingmo

#endif // FCITX5CONFIGPROXY_P_H 
