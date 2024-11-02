/*
 * liblingmosdk-base's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Shengjie Ji <jishengjie@kylinos.cn>
 *
 */

#include <QFile>
#include <QIODevice>

#include "log.hpp"
#include "system_information.hpp"

namespace kdk
{
namespace kabase
{
namespace
{
    constexpr char osReleasePath[] = "/etc/os-release";
    constexpr char osProjectCodeField[] = "PROJECT_CODENAME";
}

SystemInformation::SystemInformation() = default;

SystemInformation::~SystemInformation() = default;

QString SystemInformation::getLsbReleaseInformation(QString key)
{
#if 0
    QString ret;
    ret.clear();

    if (key.isEmpty()) {
        error << "kabase : key is empty!";
        return ret;
    }

    ret = QString(KDKGetLSBRelease(key.toStdString()).c_str());

    return ret;
#endif

    return "";
}

QString SystemInformation::getOsReleaseInformation(QString key)
{
#if 0
    QString ret;
    ret.clear();

    if (key.isEmpty()) {
        error << "kabase : key is empty!";
        return ret;
    }

    ret = QString(KDKGetOSRelease(key.toStdString()).c_str());

    return ret;
#endif

    return "";
}

/* 重写 */
QString SystemInformation::getProjectCodeName(void)
{
    QFile file(osReleasePath);
    if (!file.exists()) {
        error << "kabase : os-release file not exists";
        return "";
    }

    if (!file.open(QIODevice::ReadOnly)) {
        error << "kabase : open os-release file fail!";
        return "";
    }

    char buf[512];
    memset(buf , '\0' , sizeof(buf));
    while(file.readLine(buf , sizeof(buf)) != -1) {
        QString line = QString(buf).trimmed();
        if (line.contains(osProjectCodeField)) {
            QStringList lineList = line.split(QChar('='));
            if (lineList.size() >= 2) {
                /* 找到具体值 , 直接返回 */
                return lineList.at(1);
            }
        }
        memset(buf , '\0' , sizeof(buf));
    }

    return "";
}

QString SystemInformation::getCpuModelName(void)
{
    return "";

    // return QString(KDKGetCpuModelName().c_str());
}

QString SystemInformation::getHdPlatform(void)
{
    return "";
    // return QString(KDKGetSpecHDPlatform().c_str());
}

} /* namespace kabase */
} /* namespace kdk */
