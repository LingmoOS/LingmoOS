// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FCITX5CONFIGPROXY_H
#define FCITX5CONFIGPROXY_H

#include <QMap>
#include <QObject>
#include <QVariant>

namespace fcitx::kcm {
class DBusProvider;
}

class QDBusPendingCallWatcher;

namespace lingmo {
namespace fcitx5configtool {
class Fcitx5ConfigProxyPrivate;
class Fcitx5ConfigProxy : public QObject
{
    Q_OBJECT

public:
    explicit Fcitx5ConfigProxy(fcitx::kcm::DBusProvider *dbus,
                         const QString &path,
                         QObject *parent = nullptr);
    ~Fcitx5ConfigProxy();

    void clear();

Q_SIGNALS:
    void requestConfigFinished();

public Q_SLOTS:
    QVariant value(const QString &path) const;
    void setValue(const QString &path, const QVariant &value, bool isKey = false);
    void save();
    QVariantList globalConfigTypes() const;
    QVariantList globalConfigOptions(const QString &type, bool all = false) const;
    QVariant globalConfigOption(const QString &type, const QString &option) const;
    void restoreDefault(const QString &type);
    void requestConfig(bool sync);

private Q_SLOTS:
    void onRequestConfigFinished(QDBusPendingCallWatcher *watcher);

private:
    friend class Fcitx5ConfigProxyPrivate;
    Fcitx5ConfigProxyPrivate *const d;
};
}   // namespace fcitx5configtool
}   // namespace lingmo 

#endif // !FCITX5CONFIGPROXY_H
