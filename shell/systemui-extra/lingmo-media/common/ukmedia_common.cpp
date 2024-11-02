/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "ukmedia_common.h"
#include <QString>
#include <QDebug>

UkmediaCommon::UkmediaCommon()
{

}

UkmediaCommon& UkmediaCommon::getInstance()
{
    static UkmediaCommon instance;
    return instance;
}

SystemVersion UkmediaCommon::getSystemVersion()
{
    auto systemName = QString(QLatin1String(kdk_system_get_systemName()));
    auto version = QString(QLatin1String(kdk_system_get_version(0)));

    if (systemName.compare("lingmo", Qt::CaseInsensitive) == 0 &&
        version.contains("nile", Qt::CaseInsensitive)) {
        return SystemVersion::SYSTEM_VERSION_OKNILE;
    }
    else if (systemName.compare("lingmo", Qt::CaseInsensitive) == 0) {
        return SystemVersion::SYSTEM_VERSION_OKYANGTZE;
    }
    else if (systemName.compare("lingmo", Qt::CaseInsensitive) == 0) {
        return SystemVersion::SYSTEM_VERSION_LINGMO;
    }

    return SystemVersion::SYSTEM_VERSION_UNKNOWN;
}

bool UkmediaCommon::isHuaweiPlatform()
{
    bool isHuaweiPlatform = false;
    const char *cpuInfo = kdk_cpu_get_model();
    QString cpuStr = QString::fromLocal8Bit(cpuInfo);

    if (cpuStr.contains("HUAWEI", Qt::CaseInsensitive) || cpuStr.contains("Kirin", Qt::CaseInsensitive))
    {
        if (cpuStr.contains("990") || cpuStr.contains("9006C") || cpuStr.contains("9000C"))
            isHuaweiPlatform = true;
    }
    else if (cpuStr.contains("PANGU", Qt::CaseInsensitive) && cpuStr.contains("M900", Qt::CaseInsensitive)) {
        isHuaweiPlatform = true;
    }

    qInfo() << __func__ << isHuaweiPlatform << cpuStr;
    return isHuaweiPlatform;
}

bool UkmediaCommon::isHWKLanguageEnv(const QString &str)
{
    return ("ug_CN" == str || "kk_KZ" == str || "ky_KG" == str);
}
