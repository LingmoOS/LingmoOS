// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later
#include "settings.h"

#include <QSettings>

SettingsBackend::SettingsBackend(const QString &file, QObject *parent)
    : QObject{parent}
    , m_settings(new QSettings(file, QSettings::IniFormat))
{
}

SettingsBackend::~SettingsBackend()
{

}

QVariant SettingsBackend::value(const QString &key, const QVariant &defaultValue) const
{
    return m_settings->value(key, defaultValue);
}

void SettingsBackend::setValue(const QString& key, const QVariant &value)
{
    return m_settings->setValue(key, value);
}

void SettingsBackend::beginGroup(const QString &group)
{
    return m_settings->beginGroup(group);
}

void SettingsBackend::endGroup()
{
    return m_settings->endGroup();
}

QStringList SettingsBackend::keys() const
{
    return m_settings->allKeys();
}
