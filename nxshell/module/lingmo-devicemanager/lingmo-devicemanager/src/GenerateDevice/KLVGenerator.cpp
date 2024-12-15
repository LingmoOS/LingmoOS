// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "KLVGenerator.h"

// Qt库文件
#include <QLoggingCategory>
#include <QProcess>

// 其它头文件
#include "DeviceManager/DeviceManager.h"
#include "DeviceManager/DeviceMonitor.h"
#include "DeviceManager/DeviceNetwork.h"
#include "DeviceManager/DeviceImage.h"

KLVGenerator::KLVGenerator()
{

}

void KLVGenerator::generatorMonitorDevice()
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

    HWGenerator::generatorMonitorDevice();
}

void KLVGenerator::generatorNetworkDevice()
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

void KLVGenerator::getNetworkInfoFromCatWifiInfo()
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

void KLVGenerator::getDiskInfoFromLshw()
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
            // 应HW的要求，添加interface   UFS 3.1
            tempMap["interface"] = "UFS 3.1";
        }

        DeviceManager::instance()->addLshwinfoIntoStorageDevice(tempMap);
    }
}

void KLVGenerator::getImageInfoFromHwinfo()
{
    //  加载从hwinfo中获取的图像设备信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("hwinfo_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 3)
            continue;

        // hwinfo中对camera的分类不明确，通过camera等关键字认定图像设备
        if (!(*it)["Model"].contains("camera", Qt::CaseInsensitive) &&
                !(*it)["Device"].contains("camera", Qt::CaseInsensitive) &&
                !(*it)["Driver"].contains("uvcvideo", Qt::CaseInsensitive) &&
                !(*it)["Model"].contains("webcam", Qt::CaseInsensitive) &&
                (*it)["Hardware Class"] != "camera") {
            continue;
        }

        // KLU的问题特殊处理
        QMap<QString, QString> tempMap;
        foreach (const QString &key, (*it).keys()) {
            if ("Vendor" != key)
                tempMap.insert(key, (*it)[key]);
        }

        // 判断该摄像头是否存在，被禁用的和被拔出
        QString path = pciPath(tempMap);
        if (path.contains("usb")) {
            // 判断authorized是否存在，不存在则直接返回
            if (!QFile::exists("/sys" + path + "/authorized")) {
                continue;
            }
        }

        // 判断重复设备数据
        QString unique_id = uniqueID(tempMap);
        DeviceImage *device = dynamic_cast<DeviceImage *>(DeviceManager::instance()->getImageDevice(unique_id));
        if (device) {
            device->setEnableValue(false);
            device->setInfoFromHwinfo(tempMap);
            continue;
        } else {
            if (tempMap.contains("path")) {
                continue;
            }
        }

        device = new DeviceImage();
        device->setInfoFromHwinfo(tempMap);
        DeviceManager::instance()->addImageDevice(device);
        addBusIDFromHwinfo(tempMap["SysFS BusID"]);
    }
}

void KLVGenerator::getImageInfoFromLshw()
{
    //  加载从lshw中获取的图像设备信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("lshw_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 2)
            continue;

        QMap<QString, QString> tempMap;
        foreach (const QString &key, (*it).keys()) {
            if ("vendor" != key)
                tempMap.insert(key, (*it)[key]);
        }

        DeviceManager::instance()->setCameraInfoFromLshw(tempMap);
    }
}

