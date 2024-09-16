// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugin.h"

#include <QJsonArray>
#include <QStandardPaths>

Plugin::Plugin(QObject *parent, const KPluginMetaData &metaData)
    : QObject(parent)
    , m_enabled(true)
    , m_metaData(metaData)
{
}

int Plugin::priority() const
{
    const int priority = m_metaData.rawData()[QStringLiteral("X-KDE-Priority")].toInt();
    return (priority > 0 ? priority : 0);
}

bool Plugin::isEnabled() const
{
    return m_enabled;
}

void Plugin::setEnabled(bool enabled)
{
    m_enabled = enabled;
    emit enabledChanged();
}

bool Plugin::isReadWrite() const
{
    const bool isDeclaredReadWrite = m_metaData.rawData()[QStringLiteral("X-KDE-Kerfuffle-ReadWrite")].toBool();
    return isDeclaredReadWrite && findExecutables(readWriteExecutables());
}

QStringList Plugin::readOnlyExecutables() const
{
    QStringList readOnlyExecutables;

    const QJsonArray array = m_metaData.rawData()[QStringLiteral("X-KDE-Kerfuffle-ReadOnlyExecutables")].toArray();
    for (const QJsonValue &value : array) {
        readOnlyExecutables << value.toString();
    }

    return readOnlyExecutables;
}

QStringList Plugin::readWriteExecutables() const
{
    QStringList readWriteExecutables;

    const QJsonArray array = m_metaData.rawData()[QStringLiteral("X-KDE-Kerfuffle-ReadWriteExecutables")].toArray();
    for (const QJsonValue &value : array) {
        readWriteExecutables << value.toString();
    }

    return readWriteExecutables;
}

KPluginMetaData Plugin::metaData() const
{
    return m_metaData;
}

bool Plugin::hasRequiredExecutables() const
{
    return findExecutables(readOnlyExecutables());
}

bool Plugin::isValid() const
{
    return isEnabled() && m_metaData.isValid() && hasRequiredExecutables();
}

bool Plugin::findExecutables(const QStringList &executables)
{
    for (const QString &executable : executables) {
        if (executable.isEmpty()) {
            continue;
        }

        if (QStandardPaths::findExecutable(executable).isEmpty()) {
            return false;
        }
    }

    return true;
}
