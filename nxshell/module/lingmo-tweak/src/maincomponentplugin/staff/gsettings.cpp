// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later
#include "gsettings.h"
#include <QQmlEngine>
#include <QVariant>

GSettingsBackend::GSettingsBackend(const QByteArray &schema_id,
                                   const QByteArray &path,
                                   QObject *parent)
    : QObject(parent)
    , m_settings(new QGSettings(schema_id, path))
{
    connect(m_settings.get(), &QGSettings::changed, this, [=](const QString &key) {
        QJSValue callback = m_callback;
        if (callback.isCallable()) {
            QVariantMap map{{"key", key}, {"value", get(key)}};
            QJSValue value = m_callback.engine()->toScriptValue<QVariantMap>(map);
            callback.call({value});
        }
    });
}

void GSettingsBackend::changed(QJSValue callback)
{
    m_callback = callback;
}

bool GSettingsBackend::contains(const QString &key) const
{
    const QStringList list = m_settings->keys();
    return list.contains(key);
}

QVariant GSettingsBackend::get(const QString &key) const
{
    return contains(key) ? m_settings->get(key) : QVariant();
}

bool GSettingsBackend::set(const QString &key, const QVariant &value)
{
    return m_settings->trySet(key, value);
}

QStringList GSettingsBackend::keys() const
{
    return m_settings->keys();
}

QVariantList GSettingsBackend::choices(const QString &key) const
{
    return m_settings->choices(key);
}

void GSettingsBackend::reset(const QString &key)
{
    return m_settings->reset(key);
}
