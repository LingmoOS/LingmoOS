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
