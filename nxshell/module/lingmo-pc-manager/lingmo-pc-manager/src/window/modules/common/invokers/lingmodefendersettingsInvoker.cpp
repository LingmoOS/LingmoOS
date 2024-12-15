// Copyright (C) 2021 ~ 2021 Lingmo Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmodefendersettingsInvoker.h"

#include <QVariant>

// TODO:GSettings 要改为 DConfig

LingmoDefenderSettingsInvoker::LingmoDefenderSettingsInvoker(const QByteArray &schema_id,
                                                             const QByteArray &path,
                                                             QObject *parent)
    : QObject(parent)
{
    if (!parent) {
        parent = this;
    }
    Q_UNUSED(schema_id);
    Q_UNUSED(path);
    // m_settings = new QGSettings(schema_id, path, parent);
}

QVariant LingmoDefenderSettingsInvoker::GetValue(const QString &key) const
{
    Q_UNUSED(key);
    // if (m_settings) {
    //     return m_settings->get(key);
    // }

    return QVariant(QVariant::Type::Invalid);
}

void LingmoDefenderSettingsInvoker::SetValue(const QString &key, const QVariant &value)
{
    Q_UNUSED(key);
    Q_UNUSED(value);
    // if (m_settings) {
    //     m_settings->set(key, value);
    // }
}

void LingmoDefenderSettingsInvoker::Reset(const QString &key)
{
    Q_UNUSED(key);
    // if (m_settings) {
    //     m_settings->reset(key);
    // }
}
