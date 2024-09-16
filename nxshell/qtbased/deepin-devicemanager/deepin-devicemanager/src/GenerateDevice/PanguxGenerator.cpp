// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "PanguxGenerator.h"

// 其它头文件

#include "../DeviceManager/DeviceAudio.h"
#include "../DeviceManager/DeviceBluetooth.h"
#include "DeviceManager/DeviceManager.h"

PanguXGenerator::PanguXGenerator()
{

}

void PanguXGenerator::getAudioInfoFromCatAudio()
{
    QMap<QString, QString> tempMap;
    tempMap["Name"] = "OnBoard Audio";
    tempMap["Vendor"]= "HUAWEI";
    DeviceAudio *device = new DeviceAudio();
    device->setCanEnale(false);
    device->setCanUninstall(false);
    device->setForcedDisplay(true);
    device->setInfoFromCatAudio(tempMap);
    DeviceManager::instance()->addAudioDevice(device);

    getAudioInfoFromHwinfo();
}

void PanguXGenerator::generatorBluetoothDevice()
{
    DeviceBluetooth *device0 = new DeviceBluetooth();
    QMap<QString, QString> tempMap;
    tempMap["Vendor"] = "HISILICON";
    tempMap["Name"] = "Hi1105C";
    tempMap["Model"] = "Hi1105C";
    device0->setCanEnale(false);
    device0->setForcedDisplay(true);
    device0->setInfoFromTomlOneByOne(tempMap);
    DeviceManager::instance()->addBluetoothDevice(device0);

    const QList<QMap<QString, QString> > lstMap = DeviceManager::instance()->cmdInfo("hwinfo_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        if ((*it)["Hardware Class"] == "hub" || (*it)["Hardware Class"] == "mouse" || (*it)["Hardware Class"] == "keyboard") {
            continue;
        }
        if ((*it)["Hardware Class"] == "bluetooth" || (*it)["Driver"] == "btusb" || (*it)["Device"] == "BCM20702A0"|| (*it)["Device"].contains("bluetooth", Qt::CaseInsensitive)) {
            DeviceBluetooth *device = new DeviceBluetooth();
            device->setCanEnale(false);
            device->setForcedDisplay(true);
            device->setInfoFromHwinfo(*it);
            DeviceManager::instance()->addBluetoothDevice(device);

            addBusIDFromHwinfo((*it)["SysFS BusID"]);
        }
    }
}
void PanguXGenerator::generatorCpuDevice()
{
    HWGenerator::generatorCpuDevice();

    QFile file("/proc/cpuinfo");
    if (!file.open(QIODevice::ReadOnly))
        return;

    QMap<QString, QString> mapInfo;
    QString cpuInfo = file.readAll();
    QStringList infos = cpuInfo.split("\n\n");
    foreach (const QString &info, infos) {
        if (info.isEmpty())
            continue;

        QStringList lines = info.split("\n");
        foreach (const QString &line, lines) {
            if (line.isEmpty())
                continue;
            QStringList words = line.split(QRegExp("[\\s]*:[\\s]*"));
            if (words.size() != 2)
                continue;
            if ("Hardware" == words[0]) {
                mapInfo.insert("Name", words[1]);
            }
        }
    }
    file.close();
    if (mapInfo.find("Name") == mapInfo.end())
        return;

    QList<DeviceBaseInfo *> lst = DeviceManager::instance()->convertDeviceList(DT_Cpu);
    for (int i = 0; i < lst.size(); i++) {
        DeviceBaseInfo *device = lst[i];
        DeviceManager::instance()->tomlDeviceSet(DT_Cpu, device,mapInfo);
    }
}
