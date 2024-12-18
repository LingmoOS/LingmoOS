// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmodefendersettingsInvoker.h"

#include <QGSettings>
#include <QVariant>

LingmoDefenderSettingsInvoker::LingmoDefenderSettingsInvoker(const QByteArray &schema_id, const QByteArray &path, QObject *parent)
    : QObject(parent)
{
    if (!parent) {
        parent = this;
    }
    m_settings = new QGSettings(schema_id, path, parent);
}

QVariant LingmoDefenderSettingsInvoker::GetValue(const QString &key) const
{
    if (m_settings) {
        return m_settings->get(key);
    }

    return QVariant(QVariant::Type::Invalid);
}

void LingmoDefenderSettingsInvoker::SetValue(const QString &key, const QVariant &value)
{
    if (m_settings) {
        m_settings->set(key, value);
    }
}
