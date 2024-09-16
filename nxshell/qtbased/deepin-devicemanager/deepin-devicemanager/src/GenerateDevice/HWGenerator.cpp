// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "HWGenerator.h"
#include "EDIDParser.h"

// Qt库文件
#include <QLoggingCategory>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QDebug>

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
#include "DeviceManager/DeviceBluetooth.h"
#include "DeviceManager/DeviceNetwork.h"
#include "DeviceManager/DeviceMemory.h"

HWGenerator::HWGenerator()
{

}

void HWGenerator::generatorComputerDevice()
{
    const QList<QMap<QString, QString> >  &cmdInfo = DeviceManager::instance()->cmdInfo("cat_os_release");
    DeviceComputer *device = new DeviceComputer() ;

    // home url
    if (cmdInfo.size() > 0) {
        QString value = cmdInfo[0]["HOME_URL"];
        device->setHomeUrl(value.replace("\"", ""));
    }

    // name type
    const QList<QMap<QString, QString> >  &sysInfo = DeviceManager::instance()->cmdInfo("lshw_system");
    if (sysInfo.size() > 0) {
        device->setType(sysInfo[0]["description"]);
//        device->setVendor(sysInfo[0]["vendor"]);
        device->setName(sysInfo[0]["product"]);
    }

    const QList<QMap<QString, QString> >  &dmidecode1List = DeviceManager::instance()->cmdInfo("dmidecode1");
    if (!dmidecode1List.isEmpty() && dmidecode1List[0].contains("Product Name") && !dmidecode1List[0]["Product Name"].isEmpty()) {
        device->setName(dmidecode1List[0]["Product Name"]);
    }

    // set Os Description from /etc/os-version

    QString productName = DeviceGenerator::getProductName();
    device->setOsDescription(productName);

    // os
    const QList<QMap<QString, QString> >  &verInfo = DeviceManager::instance()->cmdInfo("cat_version");
    if (verInfo.size() > 0) {
        QString info = verInfo[0]["OS"].trimmed();
        info = info.trimmed();
        QRegExp reg("\\(gcc [\\s\\S]*(\\([\\s\\S]*\\))\\)", Qt::CaseSensitive);
        int index = reg.indexIn(info);
        if (index != -1) {
            QString tmp = reg.cap(0);
            info.remove(tmp);
            info.insert(index, reg.cap(1));
        }

        QRegExp replaceReg("\\(.*\\@.*\\)", Qt::CaseSensitive);
        info.replace(replaceReg, "");
        device->setOS(info);
    }
    DeviceManager::instance()->addComputerDevice(device);
}

void HWGenerator::generatorCpuDevice()
{
    DeviceGenerator::generatorCpuDevice();
    DeviceManager::instance()->setCpuFrequencyIsCur(false);
}

void HWGenerator::generatorAudioDevice()
{
    getAudioInfoFromCatAudio();
}

void HWGenerator::generatorBluetoothDevice()
{
    getBluetoothInfoFromHciconfig();
    getBlueToothInfoFromHwinfo();
    getBluetoothInfoFromLshw();
    getBluetoothInfoFromCatWifiInfo();
}

void HWGenerator::generatorGpuDevice()
{
    QProcess process;
    process.start("gpuinfo");
    process.waitForFinished(-1);
    int exitCode = process.exitCode();
    if (exitCode == 127 || exitCode == 126) {
        return;
    }

    QString deviceInfo = process.readAllStandardOutput();
    deviceInfo.replace("：", ":");
    QStringList items = deviceInfo.split("\n");

    QMap<QString, QString> mapInfo;
    for (QString itemStr : items) {
        if (itemStr.contains(":"))
            continue;
        QString curItemStr = itemStr.trimmed();
        if (!curItemStr.isEmpty()) {
            mapInfo.insert("Name", curItemStr);
            break;
        }
    }

    for (int i = 1; i < items.size(); ++i) {
        QStringList words = items[i].split(":");
        if (words.size() == 2) {
            mapInfo.insert(words[0].trimmed(), words[1].trimmed());
        }
    }
    if (mapInfo.size() < 2)
        return;

    DeviceGpu *device = new DeviceGpu();
    device->setCanUninstall(false);
    device->setForcedDisplay(true);
    device->setGpuInfo(mapInfo);
    DeviceManager::instance()->addGpuDevice(device);
}

void HWGenerator::generatorNetworkDevice()
{
    QList<DeviceNetwork *> lstDevice;
    const QList<QMap<QString, QString>> &lstHWInfo = DeviceManager::instance()->cmdInfo("hwinfo_network");
    for (QList<QMap<QString, QString> >::const_iterator it = lstHWInfo.begin(); it != lstHWInfo.end(); ++it) {
        // Hardware Class 类型为 network interface
//        if ("network" == (*it)["Hardware Class"]) {
//            continue;
//        }

        // 先判断是否是有效网卡信息
        // 符合两种情况中的一种 1. "HW Address" 和 "Permanent HW Address" 都必须有  2. 有 "unique_id"
        if (((*it).find("HW Address") == (*it).end() || (*it).find("Permanent HW Address") == (*it).end()) && ((*it).find("unique_id") == (*it).end())) {
            continue;
        }

        // 如果(*it)中包含unique_id属性，则说明是从数据库里面获取的，否则是从hwinfo中获取的
        if ((*it).find("unique_id") == (*it).end()) {
            DeviceNetwork *device = new DeviceNetwork();
            device->setInfoFromHwinfo(*it);
            if (!device->available()) {
                device->setCanEnale(false);
                device->setCanUninstall(false);
                device->setForcedDisplay(true);
            }
            lstDevice.append(device);
        } else {
            DeviceNetwork *device = nullptr;
            const QString &unique_id = (*it)["unique_id"];
            for (QList<DeviceNetwork *>::iterator itNet = lstDevice.begin(); itNet != lstDevice.end(); ++itNet) {
                if (!unique_id.isEmpty() && (*itNet)->uniqueID() == unique_id) {
                    device = (*itNet);
                }
            }
            if (device) {
                device->setEnableValue(false);
            }
        }
    }

    // 设置从lshw中获取的信息
    const QList<QMap<QString, QString>> &lstLshw = DeviceManager::instance()->cmdInfo("lshw_network");
    for (QList<QMap<QString, QString> >::const_iterator it = lstLshw.begin(); it != lstLshw.end(); ++it) {
        if ((*it).find("serial") == (*it).end())
            continue;
        const QString &serialNumber = (*it)["serial"];
        for (QList<DeviceNetwork *>::iterator itDevice = lstDevice.begin(); itDevice != lstDevice.end(); ++itDevice) {
            if (!serialNumber.isEmpty() && (*itDevice)->uniqueID() == serialNumber) {
                (*itDevice)->setInfoFromLshw(*it);
                break;
            }
        }
    }

    foreach (DeviceNetwork *device, lstDevice) {
        DeviceManager::instance()->addNetworkDevice(device);
    }
}

void HWGenerator::generatorDiskDevice()
{
    DeviceGenerator::generatorDiskDevice();
    DeviceManager::instance()->checkDiskSize();
}

void HWGenerator::getAudioInfoFromCatAudio()
{
    const QList<QMap<QString, QString>> lstAudio = DeviceManager::instance()->cmdInfo("cat_audio");
    QList<QMap<QString, QString> >::const_iterator it = lstAudio.begin();
    for (; it != lstAudio.end(); ++it) {
        if ((*it).size() < 2)
            continue;

        QMap<QString, QString> tempMap = *it;
        if (tempMap["Name"].contains("da_combine_v5", Qt::CaseInsensitive)) {
            tempMap["Name"] = "Hi6405";
            tempMap["Model"] = "Hi6405";
        }

        if (tempMap.contains("Vendor")) {
            tempMap["Vendor"]= "HUAWEI";
        }

        DeviceAudio *device = new DeviceAudio();
        device->setCanEnale(false);
        device->setCanUninstall(false);
        device->setForcedDisplay(true);
        device->setInfoFromCatAudio(tempMap);
        DeviceManager::instance()->addAudioDevice(device);
    }
}

void HWGenerator::getDiskInfoFromLshw()
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
        }

        DeviceManager::instance()->addLshwinfoIntoStorageDevice(tempMap);
    }
}

void HWGenerator::getDiskInfoFromSmartCtl()
{
    const QList<QMap<QString, QString>> lstMap = DeviceManager::instance()->cmdInfo("smart");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        // 剔除未识别的磁盘
        if (!(*it).contains("ln"))
            continue;

        // KLU的问题特殊处理
        QMap<QString, QString> tempMap;
        foreach (const QString &key, (*it).keys()) {
            tempMap.insert(key, (*it)[key]);
        }

        // 按照HW的需求，如果是固态硬盘就不显示转速
        if (tempMap["Rotation Rate"] == "Solid State Device")
            tempMap["Rotation Rate"] = "HW_SSD";

        DeviceManager::instance()->setStorageInfoFromSmartctl(tempMap["ln"], tempMap);
    }
}

void HWGenerator::getBluetoothInfoFromHciconfig()
{
    const QList<QMap<QString, QString>> lstMap = DeviceManager::instance()->cmdInfo("hciconfig");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        DeviceBluetooth *device = new DeviceBluetooth();
        device->setCanEnale(false);
        device->setForcedDisplay(true);
        device->setInfoFromHciconfig(*it);
        DeviceManager::instance()->addBluetoothDevice(device);
    }
}

void HWGenerator::getBlueToothInfoFromHwinfo()
{
    const QList<QMap<QString, QString>> lstMap = DeviceManager::instance()->cmdInfo("hwinfo_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        if ((*it)["Hardware Class"] == "hub" || (*it)["Hardware Class"] == "mouse" || (*it)["Hardware Class"] == "keyboard") {
            continue;
        }
        if ((*it)["Hardware Class"] == "bluetooth" || (*it)["Driver"] == "btusb" || (*it)["Device"] == "BCM20702A0") {
            if (DeviceManager::instance()->setBluetoothInfoFromHwinfo(*it))
                addBusIDFromHwinfo((*it)["SysFS BusID"]);
        }
    }
}

void HWGenerator::getBluetoothInfoFromLshw()
{
    const QList<QMap<QString, QString>> lstMap = DeviceManager::instance()->cmdInfo("lshw_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        DeviceManager::instance()->setBluetoothInfoFromLshw(*it);
    }
}

void HWGenerator::getBluetoothInfoFromCatWifiInfo()
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
        tempMap["Type"] = "Bluetooth Device";

        DeviceManager::instance()->setBluetoothInfoFromWifiInfo(tempMap);
    }
}

void HWGenerator::getMemoryInfoFromLshw()
{
    // 从lshw中获取内存信息
    const QList<QMap<QString, QString>> &lstMemory = DeviceManager::instance()->cmdInfo("lshw_memory");
    QList<QMap<QString, QString> >::const_iterator it = lstMemory.begin();

    for (; it != lstMemory.end(); ++it) {

        // bug47194 size属性包含MiB
        // 目前处理内存信息时，bank下一定要显示内存信息，否则无法生成内存
        if (!(*it)["size"].contains("GiB") && !(*it)["size"].contains("MiB"))
            continue;

        // KLU的问题特殊处理
        QMap<QString, QString> tempMap;
        foreach (const QString &key, (*it).keys()) {
            tempMap.insert(key, (*it)[key]);
        }
        if (tempMap.contains("clock")) {
            tempMap.remove("clock");
        }

        DeviceMemory *device = new DeviceMemory();
        device->setInfoFromLshw(tempMap);
        DeviceManager::instance()->addMemoryDevice(device);
    }
}

static void parseEDID(QStringList allEDIDS,QString input)
{
    for (auto edid:allEDIDS) {
        QProcess process;
        process.start(QString("hexdump %1").arg(edid));
        process.waitForFinished(-1);

        QString deviceInfo = process.readAllStandardOutput();
        if(deviceInfo.isEmpty())
            continue;

        QString edidStr;
        QStringList lines = deviceInfo.split("\n");
        for (auto line:lines) {
            QStringList words = line.trimmed().split(" ");
            if(words.size() != 9)
                continue;

            words.removeAt(0);
            QString l = words.join("");
            l.append("\n");
            edidStr.append(l);
        }

        lines = edidStr.split("\n");
        if(lines.size() > 3){
            EDIDParser edidParser;
            QString errorMsg;
            edidParser.setEdid(edidStr,errorMsg,"\n", false);

            QMap<QString, QString> mapInfo;
            mapInfo.insert("Vendor",edidParser.vendor());
            mapInfo.insert("Model",edidParser.model());
            mapInfo.insert("Date",edidParser.releaseDate());
            mapInfo.insert("Size",edidParser.screenSize());
            mapInfo.insert("Display Input",input);

            DeviceMonitor *device = new DeviceMonitor();
            device->setInfoFromEdid(mapInfo);
            DeviceManager::instance()->addMonitor(device);
        }
    }
}

void HWGenerator::generatorMonitorDevice()
{
    QString toDir = "/sys/class/drm";
    QDir toDir_(toDir);

    if (!toDir_.exists())
        return;

    QFileInfoList fileInfoList = toDir_.entryInfoList();
    foreach(QFileInfo fileInfo, fileInfoList) {
        if(fileInfo.fileName() == "." || fileInfo.fileName() == ".." || !fileInfo.fileName().startsWith("card"))
            continue;

        if(QFile::exists(fileInfo.filePath() + "/" + "edid")) {
            QStringList allEDIDS_all;
            allEDIDS_all.append(fileInfo.filePath() + "/" + "edid");
            QString interface = fileInfo.fileName().remove("card0-").remove("card1-").remove("card2-");
            parseEDID(allEDIDS_all,interface);
         }
    }
}
