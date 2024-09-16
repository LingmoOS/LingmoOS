// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "KLUGenerator.h"

// Qt库文件
#include <QLoggingCategory>
#include <QProcess>

// 其它头文件
#include "DeviceManager/DeviceManager.h"
#include "DeviceManager/DeviceGpu.h"
#include "DeviceManager/DeviceMonitor.h"
#include "DeviceManager/DeviceOthers.h"
#include "DeviceManager/DeviceStorage.h"
#include "DeviceManager/DeviceAudio.h"
#include "DeviceManager/DeviceComputer.h"
#include "DeviceManager/DevicePower.h"
#include "DeviceManager/DeviceInput.h"
#include "DeviceManager/DeviceNetwork.h"

KLUGenerator::KLUGenerator()
{

}

void KLUGenerator::getDiskInfoFromLshw()
{
    QString bootdevicePath("/proc/bootdevice/product_name");
    QString modelStr = "";
    QFile file(bootdevicePath);
    if (file.open(QIODevice::ReadOnly)) {
        modelStr = file.readLine().simplified();
        file.close();
    }

    const QList<QMap<QString, QString>> lstDisk = DeviceManager::instance()->cmdInfo("lshw_disk");
    QList<QMap<QString, QString> >::const_iterator dIt = lstDisk.begin();
    for (; dIt != lstDisk.end(); ++dIt) {
        if ((*dIt).size() < 2)
            continue;

        // KLU的问题特殊处理
        QMap<QString, QString> tempMap;
        foreach (const QString &key, (*dIt).keys()) {
            tempMap.insert(key, (*dIt)[key]);
        }

//        qCInfo(appLog) << tempMap["product"] << " ***** " << modelStr << " " << (tempMap["product"] == modelStr);
        // HW写死
        if (tempMap["product"] == modelStr) {
            // 应HW的要求，将描述固定为   Universal Flash Storage
            tempMap["description"] = "Universal Flash Storage";
            // 应HW的要求，添加interface   UFS 3.0
            tempMap["interface"] = "UFS 3.0";

            // 读取interface版本
            QProcess process;
            process.start("cat /sys/devices/platform/f8200000.ufs/host0/scsi_host/host0/wb_en");
            process.waitForFinished(-1);
            int exitCode = process.exitCode();
            if (exitCode != 127 && exitCode != 126) {
                QString deviceInfo = process.readAllStandardOutput();
                if (deviceInfo.trimmed() == "true") {
                    process.start("cat /sys/block/sdd/device/spec_version");
                    process.waitForFinished(-1);
                    exitCode = process.exitCode();
                    if (exitCode != 127 && exitCode != 126) {
                        deviceInfo = process.readAllStandardOutput();
                        if (deviceInfo.trimmed() == "310") {
                            tempMap["interface"] = "UFS 3.1";
                        }
                    }
                }
            }
        }

        DeviceManager::instance()->addLshwinfoIntoStorageDevice(tempMap);
    }
}

void KLUGenerator::generatorNetworkDevice()
{
    const QList<QMap<QString, QString>> lstInfo = DeviceManager::instance()->cmdInfo("lshw_network");
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2) {
            continue;
        }
        QMap<QString, QString> tempMap = *it;
        /*
        capabilities: ethernet physical wireless
        configuration: broadcast=yes ip=10.4.6.115 multicast=yes wireless=IEEE 802.11
        */
        if ((tempMap["configuration"].indexOf("wireless=IEEE 802.11") > -1) ||
                (tempMap["capabilities"].indexOf("wireless") > -1)) {
            continue;
        }

        DeviceNetwork *device = new DeviceNetwork();
        device->setInfoFromLshw(*it);
        device->setCanEnale(false);
        device->setCanUninstall(false);
        device->setForcedDisplay(true);
        DeviceManager::instance()->addNetworkDevice(device);
    }
    // HW 要求修改名称,制造商以及类型
    getNetworkInfoFromCatWifiInfo();
}

void KLUGenerator::getNetworkInfoFromCatWifiInfo()
{
    QList<QMap<QString, QString> >  lstWifiInfo;
    QString wifiDevicesInfoPath("/sys/hisys/wal/wifi_devices_info");
    QFile file(wifiDevicesInfoPath);
    if (file.open(QIODevice::ReadOnly)) {
        QMap<QString, QString>  wifiInfo;
        QString allStr = file.readAll();
        file.close();

        // 解析数据
        QStringList items = allStr.split("\n");
        foreach (const QString &item, items) {
            if (item.isEmpty())
                continue;

            QStringList strList = item.split(':', QString::SkipEmptyParts);
            if (strList.size() == 2)
                wifiInfo[strList[0] ] = strList[1];
        }

        if (!wifiInfo.isEmpty())
            lstWifiInfo.append(wifiInfo);
    }
    if (lstWifiInfo.size() == 0) {
        return;
    }
    QList<QMap<QString, QString> >::const_iterator it = lstWifiInfo.begin();

    for (; it != lstWifiInfo.end(); ++it) {
        if ((*it).size() < 3) {
            continue;
        }

        // KLU的问题特殊处理
        QMap<QString, QString> tempMap;
        foreach (const QString &key, (*it).keys()) {
            tempMap.insert(key, (*it)[key]);
        }

        // cat /sys/hisys/wal/wifi_devices_info  获取结果为 HUAWEI HI103
        if (tempMap["Chip Type"].contains("HUAWEI", Qt::CaseInsensitive)) {
            tempMap["Chip Type"] = tempMap["Chip Type"].remove("HUAWEI").trimmed();
        }

        // 按照华为的需求，设置蓝牙制造商和类型
        tempMap["Vendor"] = "HISILICON";
        tempMap["Type"] = "Wireless network";

        DeviceManager::instance()->setNetworkInfoFromWifiInfo(tempMap);
    }
}

void KLUGenerator::generatorMonitorDevice()
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Name", "LCD");
//    mapInfo.insert("Vendor", "HUAWEI");
    mapInfo.insert("CurResolution", "2160x1440@60Hz");
    mapInfo.insert("SupportResolution", "2160x1440@60Hz");
    mapInfo.insert("Size", "14 Inch");
//    mapInfo.insert("Date", "2019年7月");

    DeviceMonitor *monitor = new  DeviceMonitor();
    monitor->setInfoFromSelfDefine(mapInfo);
    DeviceManager::instance()->addMonitor(monitor);
}

