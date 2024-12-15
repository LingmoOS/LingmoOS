// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCONFIG_SETTINGS
#define DCONFIG_SETTINGS

#include <DConfig>

#include <QObject>
#include <QString>

DCORE_USE_NAMESPACE

// Dconfig 配置类
class DconfigSettings
{
public:
    explicit DconfigSettings();
    virtual ~DconfigSettings();

public:
    static const QVariant ConfigValue(const QString &appId, const QString &name, const QString &key, const QVariant &fallback);
    static bool ConfigSaveValue(const QString &appId, const QString &name, const QString &key, const QVariant &value);
    static DConfig *ConfigPtr(const QString &appId, const QString &name);
};

#endif // DCONFIG_SETTINGS
