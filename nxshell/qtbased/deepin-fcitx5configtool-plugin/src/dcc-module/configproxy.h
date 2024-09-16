// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCC_MODULE_CONFIGPROXY_H
#define DCC_MODULE_CONFIGPROXY_H

#include <QMap>
#include <QObject>

namespace fcitx::kcm {
class DBusProvider;
}

class QDBusPendingCallWatcher;

class ConfigProxy : public QObject
{
    Q_OBJECT

public:
    explicit ConfigProxy(fcitx::kcm::DBusProvider *dbus,
                         const QString &path,
                         QObject *parent = nullptr);
    ~ConfigProxy();

    void requestConfig(bool sync);

    QVariant value(const QString &path) const;
    void setValue(const QString &path, const QVariant &value);
    void save();

Q_SIGNALS:
    void requestConfigFinished();

private Q_SLOTS:
    void onRequestConfigFinished(QDBusPendingCallWatcher *watcher);

private:
    void setValue(const QVariant &value);

private:
    fcitx::kcm::DBusProvider *m_dbus;
    QString m_path;

    QVariantMap m_value;
};

#endif // !DCC_MODULE_CONFIGPROXY_H
