/*
 * Copyright (C) 2022 ~ 2023 Deepin Technology Co., Ltd.
 *
 * Author:     weizhixiang <weizhixiang@uniontech.com>
 *
 * Maintainer: weizhixiang <weizhixiang@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dconfig_settings.h"

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
