// Copyright (C) 2021 ~ 2021 Lingmo Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LINGMO_DEFENDER_SETTINGS_H
#define LINGMO_DEFENDER_SETTINGS_H

#include "invokerinterface.h"

#include <QObject>

class QGSettings;

class LingmoDefenderSettingsInvoker : public QObject, public SettingsInvokerInterface
{
    Q_OBJECT
public:
    explicit LingmoDefenderSettingsInvoker(const QByteArray &schema_id,
                                           const QByteArray &path = QByteArray(),
                                           QObject *parent = nullptr);

    virtual ~LingmoDefenderSettingsInvoker() override { }

    virtual QVariant GetValue(const QString &key) const override;
    virtual void SetValue(const QString &key, const QVariant &value) override;
    virtual void Reset(const QString &key) override;

private:
    QGSettings *m_settings;

    // 禁用拷贝构造，没有这种操作的必要，并且会增加资源控制的麻烦
    LingmoDefenderSettingsInvoker(LingmoDefenderSettingsInvoker &);
    LingmoDefenderSettingsInvoker &operator=(const LingmoDefenderSettingsInvoker &);
};

#endif // LINGMO_DEFENDER_SETTINGS_H
