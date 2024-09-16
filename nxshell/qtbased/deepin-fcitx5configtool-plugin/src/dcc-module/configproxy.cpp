// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configproxy.h"

#include <dbusprovider.h>
#include <varianthelper.h>

ConfigProxy::ConfigProxy(fcitx::kcm::DBusProvider *dbus, const QString &path, QObject *parent)
    : QObject(parent)
    , m_dbus(dbus)
    , m_path(path)
{
}

ConfigProxy::~ConfigProxy() = default;

void ConfigProxy::requestConfig(bool sync)
{
    if (!m_dbus->controller()) {
        return;
    }
    auto call = m_dbus->controller()->GetConfig(m_path);
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher,
            &QDBusPendingCallWatcher::finished,
            this,
            &ConfigProxy::onRequestConfigFinished);
    if (sync) {
        watcher->waitForFinished();
    }
}

void ConfigProxy::onRequestConfigFinished(QDBusPendingCallWatcher *watcher)
{
    watcher->deleteLater();
    QDBusPendingReply<QDBusVariant, fcitx::FcitxQtConfigTypeList> reply = *watcher;
    if (reply.isError()) {
        qWarning() << reply.error();
        return;
    }

    auto value = reply.argumentAt<0>().variant();

    QVariantMap map;
    if (value.canConvert<QDBusArgument>()) {
        auto argument = qvariant_cast<QDBusArgument>(value);
        argument >> map;
    } else {
        map = value.toMap();
    }

    std::swap(m_value, map);

    Q_EMIT requestConfigFinished();
}

QVariant ConfigProxy::value(const QString &path) const
{
    return fcitx::kcm::readVariant(m_value, path);
}

void ConfigProxy::setValue(const QString &path, const QVariant &value)
{
    fcitx::kcm::writeVariant(m_value, path, value);
}

void ConfigProxy::save()
{
    if (!m_dbus->controller()) {
        return;
    }

    m_dbus->controller()->SetConfig(m_path, QDBusVariant(QVariant(m_value)));
}
