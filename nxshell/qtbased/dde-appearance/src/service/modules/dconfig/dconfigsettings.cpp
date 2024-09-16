// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dconfigsettings.h"

#include <QSharedPointer>
#include <QDebug>

DconfigSettings::DconfigSettings() {}
DconfigSettings::~DconfigSettings() {}

DConfig *DconfigSettings::ConfigPtr(const QString &appId, const QString &name)
{
    auto config = DConfig::create(appId, name);
    if (!config) {
        qWarning() << "ConfigPtr config is failed";
        return nullptr;
    }

    if (config->isValid()) {
        return config;
    }

    qWarning() << "Cannot find dconfigs, name:" << name;
    return nullptr;
}

const QVariant DconfigSettings::ConfigValue(const QString &appId, const QString &name, const QString &key, const QVariant &fallback)
{
    // 每次get set都重新创建一个对象，防止dconfig-daemon崩溃，pm中维持了一个错误的对象；组策略对重复对象做了缓存。
    QScopedPointer<DConfig> config(ConfigPtr(appId, name));
    if (config && config->isValid() && config->keyList().contains(key)) {
        QVariant v = config->value(key);
        return v;
    }

    qWarning() << " key:" << key
             << "Use fallback value:" << fallback;
    return fallback;
}

bool DconfigSettings::ConfigSaveValue(const QString &appId, const QString &name, const QString &key, const QVariant &value)
{
    // 每次get set都重新创建一个对象，防止dconfig-daemon崩溃，pm中维持了一个错误的对象；组策略对重复对象做了缓存。
    QScopedPointer<DConfig> config(ConfigPtr(appId, name));
    if (config && config->isValid() && config->keyList().contains(key)) {
        config->setValue(key, value);
        // dconf频繁读写，大概率有写失败错误
        qWarning() << "set value: " << key << value;
        return true;
    }

    qDebug() << " key:" << key;
    return false;
}
