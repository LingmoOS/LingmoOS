// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Dconfig.h"

#include <QDebug>
#include <QSharedPointer>

#ifdef Dtk6Core_FOUND

DconfigSettings::DconfigSettings() { }

DconfigSettings::~DconfigSettings() { }

DConfig *DconfigSettings::ConfigPtr(const QString &appId,
                                    const QString &name,
                                    const QString &subpath,
                                    QObject *parent)
{
    auto config = DConfig::create(appId, name, subpath, parent);
    if (!config) {
        qWarning() << "ConfigPtr config is failed";
        return nullptr;
    }

    if (config->isValid()) {
        return config;
    }

    qWarning() << "Cannot find dconfigs, name:" << subpath;
    return nullptr;
}

const QVariant DconfigSettings::ConfigValue(const QString &appId,
                                            const QString &name,
                                            const QString &key,
                                            const QVariant &fallback)
{
    QScopedPointer<DConfig> config(ConfigPtr(appId, name));
    if (config && config->isValid() && config->keyList().contains(key)) {
        QVariant v = config->value(key);
        return v;
    }

    qWarning() << " key:" << key << "Use fallback value:" << fallback;
    return fallback;
}

bool DconfigSettings::ConfigSaveValue(const QString &appId,
                                      const QString &name,
                                      const QString &key,
                                      const QVariant &value)
{
    QScopedPointer<DConfig> config(ConfigPtr(appId, name));
    if (config && config->isValid() && config->keyList().contains(key)) {
        config->setValue(key, value);
        // dconf频繁读写，大概率有写失败错误
        qDebug() << "set value: " << key << value;
        return true;
    }

    qWarning() << " key:" << key;
    return false;
}

#endif
