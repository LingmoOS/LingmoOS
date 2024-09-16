// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _DIM_DCONFIG_SETTINGS
#define _DIM_DCONFIG_SETTINGS

#include <QObject>
#include <QString>

#ifdef Dtk6Core_FOUND

#include <DConfig>
using Dtk::Core::DConfig;

// Dconfig 配置类
class DconfigSettings
{
public:
    explicit DconfigSettings();
    virtual ~DconfigSettings();

public:
    static const QVariant ConfigValue(const QString &appId,
                                      const QString &name,
                                      const QString &key,
                                      const QVariant &fallback);
    static bool ConfigSaveValue(const QString &appId,
                                const QString &name,
                                const QString &key,
                                const QVariant &value);
    static DConfig *ConfigPtr(const QString &appId,
                              const QString &name,
                              const QString &subpath = QString(),
                              QObject *parent = nullptr);
};

#endif

#endif // _DIM_DCONFIG_SETTINGS
