// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "enableutils.h"
#include "enablesqlmanager.h"

#include <QStringList>
#include <QMap>
#include <QFile>
#include <QProcess>
#include <QCryptographicHash>

#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define LEAST_NUM 10
#define REG_ADDRESS "^[0-9a-z]{2}:[0-9a-z]{2}:[0-9a-z]{2}:[0-9a-z]{2}:[0-9a-z]{2}:[0-9a-z]{2}$"

EnableUtils::EnableUtils()
{

}

void EnableUtils::disableOutDevice(const QString &info)
{
    QStringList items = info.split("\n\n");
    foreach (const QString &item, items) {
        QMap<QString, QString> mapItem;
        if (!getMapInfo(item, mapItem))
            continue;

        // 获取设备的唯一标识
        // 网卡的唯一标识为网卡的物理地址
        // 有序列号id的设备使用序列号id作为唯一标识
        QString uniqueID;
        if (mapItem.find("Permanent HW Address") != mapItem.end()) {
            uniqueID = mapItem["Permanent HW Address"];
        } else if (mapItem.find("Serial ID") != mapItem.end()) {
            uniqueID = mapItem["Serial ID"];
        } else {
            uniqueID = mapItem["Module Alias"];
            uniqueID.replace(QRegExp("[0-9a-zA-Z]{10}$"), "");
        }

        if (mapItem.contains("Vendor") && mapItem.contains("Device") && (mapItem.contains("SysFS ID") || mapItem.contains("SysFS Device Link"))
                && mapItem["Vendor"].contains("0x") && mapItem["Device"].contains("0x")) {
            QStringList vendorlist = mapItem["Vendor"].split(" ");
            QStringList devicelist = mapItem["Device"].split(" ");
            if (vendorlist.size() > 1 && devicelist.size() > 1  && ((mapItem.contains("SysFS ID") && !mapItem["SysFS ID"].isEmpty())
                                                                    || (mapItem.contains("SysFS Device Link") && !mapItem["SysFS Device Link"].isEmpty()))) {
                QString valueStr = vendorlist[1].trimmed() + devicelist[1].remove("0x", Qt::CaseSensitive).trimmed();
                QCryptographicHash Hash(QCryptographicHash::Md5);
                QByteArray buf;
                buf.append(valueStr);
                if (mapItem.contains("SysFS Device Link") && !mapItem["SysFS Device Link"].isEmpty()) {
                    buf.append(mapItem["SysFS Device Link"].trimmed());
                } else {
                    buf.append(mapItem["SysFS ID"].trimmed());
                }
                Hash.addData(buf);
                uniqueID = QString::fromStdString(Hash.result().toBase64().toStdString());
            }
        }
        if (uniqueID.isEmpty()) {
            continue;
        }

        // 获取设备信息路径
        // 有 SysFS Device Link 的使用 SysFS Device Link
        // 没有 SysFS Device Link 的使用 SysFS ID
        // 如果是网卡则使用逻辑名称作为path
        QString path;
        if (mapItem.find("SysFS Device Link") != mapItem.end()) {
            path = mapItem["SysFS Device Link"];
        } else {
            path = mapItem["SysFS ID"];
        }
        path.replace(QRegExp("[1-9]$"), "0");


        // 网卡采用ioctl的方式禁用
        QRegExp reg(REG_ADDRESS);
        if (reg.exactMatch(uniqueID)) {
            path = mapItem["Device File"];
            if (EnableSqlManager::getInstance()->uniqueIDExisted(uniqueID) &&
                    EnableUtils::ioctlOperateNetworkLogicalName(path, false))
                EnableSqlManager::getInstance()->updateDataToAuthorizedTable(uniqueID, path);
            continue;
        }


        // 先判断设备是否被记录在数据库，如果在则禁用
        if (EnableSqlManager::getInstance()->uniqueIDExisted(uniqueID)) {
            QFile file("/sys" + path + QString("/authorized"));
            if (!file.open(QIODevice::ReadWrite)) {
                return;
            }
            file.write("0");
            file.close();
            // 更数据库信息，方式更换usb接口
            EnableSqlManager::getInstance()->updateDataToAuthorizedTable(uniqueID, path);
        }
    }
}

void EnableUtils::disableInDevice()
{
    // 网卡通过ioctl禁用
    QList<QPair<QString, QString> > lstAuthPair;
    EnableSqlManager::getInstance()->authorizedPathUniqueIDList(lstAuthPair);
    for (QList<QPair<QString, QString>>::iterator it = lstAuthPair.begin() ; it != lstAuthPair.end(); ++it) {
        QRegExp reg(REG_ADDRESS);
        if (reg.exactMatch((*it).second)) {
            EnableUtils::ioctlOperateNetworkLogicalName((*it).first, false);
            continue;
        }
    }


    // 其它通过remove文件禁用
    QList<QPair<QString, QString> > lstRemovePair;
    EnableSqlManager::getInstance()->removePathUniqueIDList(lstRemovePair);
    for (QList<QPair<QString, QString>>::iterator it = lstRemovePair.begin() ; it != lstRemovePair.end(); ++it) {
        QString pathT = "/sys" + (*it).first + QString("/remove");
        if (!QFile::exists(pathT)) {
            pathT = "/sys" + (*it).first + QString("/reset");
        }

        QFile file(pathT);
        if (file.open(QIODevice::WriteOnly)) {
            file.write("1");
            file.close();
        }
    }
}

bool EnableUtils::ioctlOperateNetworkLogicalName(const QString &logicalName, bool enable)
{
    // 1. 通过ioctl禁用
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return false;
    struct ifreq ifr;
    strncpy(ifr.ifr_name, logicalName.toStdString().c_str(),IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    short flag;
    if (enable) {
        flag = IFF_UP | IFF_PROMISC;
    } else {
        flag = ~(IFF_UP | IFF_PROMISC);
    }
    // 先获取标识
    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
        close(fd);
        return false;
    }
    // 获取后重新设置标识
    if (enable) {
        ifr.ifr_ifru.ifru_flags |= flag;
    } else {
        ifr.ifr_ifru.ifru_flags &= flag;
    }

    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
        close(fd);
        return false;
    }
    return true;
}

bool EnableUtils::getMapInfo(const QString &item, QMap<QString, QString> &mapInfo)
{
    QStringList lines = item.split("\n");
    // 行数太少则为无用信息
    if (lines.size() <= LEAST_NUM) {
        return false;
    }

    foreach (const QString &line, lines) {
        QStringList words = line.split(": ");
        if (words.size() != 2)
            continue;
        mapInfo.insert(words[0].trimmed(), words[1].replace("\"", "").trimmed());
    }

    // hub为usb接口，可以直接过滤
    if (mapInfo["Hardware Class"] == "hub") {
        return false;
    }

    // 没有总线信息的设备可以过滤
    if (mapInfo.find("SysFS BusID") == mapInfo.end()) {
        return false;
    }

    return true;
}
