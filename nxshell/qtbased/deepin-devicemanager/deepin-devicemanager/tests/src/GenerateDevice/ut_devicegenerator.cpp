// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceGenerator.h"
#include "DeviceFactory.h"
#include "GenerateDevicePool.h"
#include "DeviceManager.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

QList<QMap<QString, QString> > lstMap;
class UT_DeviceGenerator : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceGenerator = new DeviceGenerator;
        QMap<QString, QString> mapInfo1;
        mapInfo1.insert("Manufacturer", "Samsung");
        mapInfo1.insert("product", "M378A1K43CB2-CTD");
        mapInfo1.insert("width", "64 bits");
        mapInfo1.insert("OS", "Linux version 4.19.0-amd64-desktop (uos@x86-compile-PC) (gcc version 8.3.0 (Uos 8.3.0.5-1+dde)) #4016 SMP Wed Jun 23 13:35:29 CST 2021");
        mapInfo1.insert("physical", "8");
        mapInfo1.insert("core", "8");
        mapInfo1.insert("logical", "8");
        mapInfo1.insert("size", "8GiB");
        mapInfo1.insert("Capacity", "931 GB");
        mapInfo1.insert("SysFS BusID", "0:0:0:0");
        mapInfo1.insert("Model", "camera");
        mapInfo1.insert("logical name", "enth0");
        mapInfo1.insert("serial", "0000000");
        mapInfo1.insert("Hardware Class", "bluetooth");
        lstMap.append(mapInfo1);
        QMap<QString, QString> mapInfo2;
        mapInfo2.insert("Manufacturer", "Samsung");
        mapInfo2.insert("product", "M378A1K43CB2-CTD");
        mapInfo2.insert("width", "64 bits");
        mapInfo2.insert("OS", "Linux version 4.19.0-amd64-desktop (uos@x86-compile-PC) (gcc version 8.3.0 (Uos 8.3.0.5-1+dde)) #4016 SMP Wed Jun 23 13:35:29 CST 2021");
        mapInfo2.insert("size", "8GiB");
        mapInfo2.insert("Capacity", "931 GB");
        mapInfo2.insert("SysFS BusID", "0:0:0:0");
        mapInfo2.insert("Model", "camera");
        mapInfo2.insert("logical name", "enth1");
        mapInfo2.insert("serial", "0000000");
        mapInfo2.insert("Hardware Class", "bluetooth");
        lstMap.append(mapInfo2);
    }
    void TearDown()
    {
        delete m_deviceGenerator;
        lstMap.clear();
    }
    DeviceGenerator *m_deviceGenerator = nullptr;
};

//virtual void generatorComputerDevice();
const QList<QMap<QString, QString> > &ut_DeviceGenerator_cmdInfo()
{
    return lstMap;
}
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_generatorComputerDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->generatorComputerDevice();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceComputer.size());
}

//virtual void generatorCpuDevice();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_generatorCpuDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->generatorCpuDevice();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceCPU.size());
}

//virtual void generatorBiosDevice();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_generatorBiosDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->generatorBiosDevice();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceBios.size());
}

//virtual void generatorMemoryDevice();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_generatorMemoryDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->generatorMemoryDevice();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceMemory.size());
}

//virtual void generatorDiskDevice();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_generatorDiskDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->generatorDiskDevice();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceStorage.size());
}

//virtual void generatorGpuDevice();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_generatorGpuDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->generatorGpuDevice();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceGPU.size());
}

//virtual void generatorMonitorDevice();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_generatorMonitorDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->generatorMonitorDevice();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceMonitor.size());
}

const QList<QMap<QString, QString> > &ut_DeviceGenerator_cmdInfo_hwinfonetwork(void *obj, const QString &key)
{
    if ("hwinfo_network" == key) {
        QMap<QString, QString> mapInfo;
        mapInfo.insert("Unique ID", "Ij4C.ndpeucax6V1");
        mapInfo.insert("Parent ID", "c3qJ.NibVsBb6xb5");
        mapInfo.insert("SysFS ID", "/class/net/enp2s0");
        mapInfo.insert("SysFS Device Link", "/devices/pci0000:00/0000:00:1c.0/0000:02:00.0");
        mapInfo.insert("Hardware Class", "network interface");
        mapInfo.insert("Model", "Ethernet network interface");
        mapInfo.insert("Driver", "r8169");
        mapInfo.insert("Driver Modules", "r8169");
        mapInfo.insert("Device File", "enp2s0");
        mapInfo.insert("HW Address", "f4:b5:20:24:5e:4f");
        mapInfo.insert("Permanent HW Address", "f4:b5:20:24:5e:4f");
        mapInfo.insert("Link detected", "yes");
        mapInfo.insert("Config Status", "cfg=new, avail=yes, need=no, active=unknown");
        mapInfo.insert("Attached to", "#6 (Ethernet controller)");
        lstMap.clear();
        lstMap.append(mapInfo);
        QMap<QString, QString> mapInfo1;
        mapInfo1.insert("Unique ID", "Ij4C.ndpeucax6V1");
        mapInfo1.insert("Parent ID", "c3qJ.NibVsBb6xb5");
        mapInfo1.insert("SysFS ID", "/class/net/enp2s0");
        mapInfo1.insert("SysFS Device Link", "/devices/pci0000:00/0000:00:1c.0/0000:02:00.0");
        mapInfo1.insert("Hardware Class", "network");
        lstMap.append(mapInfo);
    } else if ("lshw_network" == key) {
        QMap<QString, QString> mapInfo;
        mapInfo.insert("description", "Ethernet interface");
        mapInfo.insert("product", "RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller");
        mapInfo.insert("vendor", "Realtek Semiconductor Co., Ltd.");
        mapInfo.insert("physical id", "0");
        mapInfo.insert("bus info", "pci@0000:02:00.0");
        mapInfo.insert("logical name", "enp2s0");
        mapInfo.insert("version", "15");
        mapInfo.insert("serial", "f4:b5:20:24:5e:4f");
        mapInfo.insert("size", "1Gbit/s");
        mapInfo.insert("capacity", "1Gbit/s");
        mapInfo.insert("width", "64 bits");
        mapInfo.insert("clock", "33MHz");
        mapInfo.insert("capabilities", "pm msi pciexpress msix bus_master cap_list ethernet");
        lstMap.clear();
        lstMap.append(mapInfo);
    }

    return lstMap;
}

//virtual void generatorNetworkDevice();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_generatorNetworkDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo_hwinfonetwork);
    m_deviceGenerator->generatorNetworkDevice();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceNetwork.size());
}

//virtual void generatorAudioDevice();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_generatorAudioDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->generatorAudioDevice();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceAudio.size());
}

//virtual void generatorBluetoothDevice();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_generatorBluetoothDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->generatorBluetoothDevice();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceBluetooth.size());
}

//virtual void generatorKeyboardDevice();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_generatorKeyboardDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->generatorKeyboardDevice();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceKeyboard.size());
}

//virtual void generatorMouseDevice();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_generatorMouseDevice)
{
//    Stub stub;
//    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
//    m_deviceGenerator->generatorMouseDevice();
//    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceMouse.size());
}

//virtual void generatorPrinterDevice();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_generatorPrinterDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->generatorPrinterDevice();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDevicePrint.size());
}

//virtual void generatorCameraDevice();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_generatorCameraDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->generatorCameraDevice();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceImage.size());
}

//virtual void generatorCdromDevice();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_generatorCdromDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->generatorCdromDevice();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceCdrom.size());
}

//virtual void generatorOthersDevice();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_generatorOthersDevice)
{
//    Stub stub;
//    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
//    m_deviceGenerator->generatorOthersDevice();
//    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceOthers.size());
}

//virtual void generatorPowerDevice();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_generatorPowerDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->generatorPowerDevice();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDevicePower.size());
}

//void addBusIDFromHwinfo(const QString &sysfsBusID);
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_addBusIDFromHwinfo)
{
    m_deviceGenerator->addBusIDFromHwinfo("1-1:1-0");
    EXPECT_TRUE(m_deviceGenerator->m_ListBusID.size());
}
//const QStringList &getBusIDFromHwinfo();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getBusIDFromHwinfo)
{
    m_deviceGenerator->addBusIDFromHwinfo("1-1:1-0");
    EXPECT_TRUE(m_deviceGenerator->getBusIDFromHwinfo().size());
}

//static const QString getProductName();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getProductName)
{
    EXPECT_FALSE(m_deviceGenerator->getProductName().isEmpty());
}

//virtual void getBiosInfo();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getBiosInfo)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getBiosInfo();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceBios.size());
}

//virtual void getSystemInfo();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getSystemInfo)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getSystemInfo();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceBios.size());
}

//virtual void getBaseBoardInfo();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getBaseBoardInfo)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getBaseBoardInfo();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceBios.size());
}

//virtual void getChassisInfo();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getChassisInfo)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getChassisInfo();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceBios.size());
}

//virtual void getBiosMemoryInfo();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getBiosMemoryInfo)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getBiosMemoryInfo();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceBios.size());
}

//virtual void getMemoryInfoFromLshw();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getMemoryInfoFromLshw)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getMemoryInfoFromLshw();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceMemory.size());
}

//virtual void getMemoryInfoFromDmidecode();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getMemoryInfoFromDmidecode)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getMemoryInfoFromLshw();
    m_deviceGenerator->getMemoryInfoFromDmidecode();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceMemory.size());
}

//virtual void getDiskInfoFromHwinfo();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getDiskInfoFromHwinfo)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getDiskInfoFromHwinfo();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceStorage.size());
}

//virtual void getDiskInfoFromLshw();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getDiskInfoFromLshw)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getDiskInfoFromHwinfo();
    m_deviceGenerator->getDiskInfoFromLshw();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceStorage.size());
}

//virtual void getDiskInfoFromLsblk();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getDiskInfoFromLsblk)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getDiskInfoFromHwinfo();
    m_deviceGenerator->getDiskInfoFromLshw();
    m_deviceGenerator->getDiskInfoFromLsblk();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceStorage.size());
}

//virtual void getDiskInfoFromSmartCtl();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getDiskInfoFromSmartCtl)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getDiskInfoFromLshw();
    m_deviceGenerator->getDiskInfoFromLsblk();
    m_deviceGenerator->getDiskInfoFromSmartCtl();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceStorage.size());
}

//virtual void getGpuInfoFromHwinfo();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getGpuInfoFromHwinfo)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getGpuInfoFromHwinfo();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceGPU.size());
}

//virtual void getGpuInfoFromLshw();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getGpuInfoFromLshw)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getGpuInfoFromHwinfo();
    m_deviceGenerator->getGpuInfoFromLshw();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceGPU.size());
}

//virtual void getGpuInfoFromXrandr();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getGpuInfoFromXrandr)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getGpuInfoFromHwinfo();
    m_deviceGenerator->getGpuInfoFromLshw();
    m_deviceGenerator->getGpuInfoFromXrandr();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceGPU.size());
}

//virtual void getGpuSizeFromDmesg();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getGpuSizeFromDmesg)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getGpuInfoFromHwinfo();
    m_deviceGenerator->getGpuInfoFromLshw();
    m_deviceGenerator->getGpuInfoFromXrandr();
    m_deviceGenerator->getGpuSizeFromDmesg();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceGPU.size());
}

//virtual void getMonitorInfoFromHwinfo();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getMonitorInfoFromHwinfo)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getMonitorInfoFromHwinfo();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceMonitor.size());
}

//virtual void getMonitorInfoFromXrandrVerbose();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getMonitorInfoFromXrandrVerbose)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getMonitorInfoFromHwinfo();
    m_deviceGenerator->getMonitorInfoFromXrandrVerbose();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceMonitor.size());
}

//virtual void getAudioInfoFromHwinfo();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getAudioInfoFromHwinfo)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getAudioInfoFromHwinfo();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceAudio.size());
}

//virtual void getAudioInfoFromLshw();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getAudioInfoFromLshw)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getAudioInfoFromHwinfo();
    m_deviceGenerator->getAudioInfoFromLshw();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceAudio.size());
}

//virtual void getAudioInfoFromCatInput();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getAudioInfoFromCatInput)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getAudioInfoFromHwinfo();
    m_deviceGenerator->getAudioInfoFromLshw();
    m_deviceGenerator->getAudioInfoFromCatInput();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceAudio.size());
}
//virtual void getAudioChipInfoFromDmesg();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getAudioChipInfoFromDmesg)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getAudioInfoFromHwinfo();
    m_deviceGenerator->getAudioInfoFromLshw();
    m_deviceGenerator->getAudioInfoFromCatInput();
    m_deviceGenerator->getAudioChipInfoFromDmesg();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceAudio.size());
}

//virtual void getBluetoothInfoFromHciconfig();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getBluetoothInfoFromHciconfig)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getBluetoothInfoFromHciconfig();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceBluetooth.size());
}

//virtual void getBlueToothInfoFromHwinfo();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getBlueToothInfoFromHwinfo)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getBluetoothInfoFromHciconfig();
    m_deviceGenerator->getBlueToothInfoFromHwinfo();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceBluetooth.size());
}

//virtual void getKeyboardInfoFromHwinfo();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getKeyboardInfoFromHwinfo)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getKeyboardInfoFromHwinfo();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceKeyboard.size());
}

//virtual void getKeyboardInfoFromLshw();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getKeyboardInfoFromLshw)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getKeyboardInfoFromHwinfo();
    m_deviceGenerator->getKeyboardInfoFromLshw();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceKeyboard.size());
}

//virtual void getKeyboardInfoFromCatDevices();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getKeyboardInfoFromCatDevices)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getKeyboardInfoFromHwinfo();
    m_deviceGenerator->getKeyboardInfoFromLshw();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceKeyboard.size());
}

//virtual void getMouseInfoFromHwinfo();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getMouseInfoFromHwinfo)
{
//    Stub stub;
//    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
//    m_deviceGenerator->getMouseInfoFromHwinfo();
//    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceMouse.size());
}

//virtual void getMouseInfoFromLshw();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getMouseInfoFromLshw)
{
//    Stub stub;
//    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
//    m_deviceGenerator->getMouseInfoFromHwinfo();
//    m_deviceGenerator->getMouseInfoFromLshw();
//    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceMouse.size());
}

//virtual void getMouseInfoFromCatDevices();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getMouseInfoFromCatDevices)
{
//    Stub stub;
//    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
//    m_deviceGenerator->getMouseInfoFromHwinfo();
//    m_deviceGenerator->getMouseInfoFromLshw();
//    m_deviceGenerator->getMouseInfoFromCatDevices();
//    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceMouse.size());
}

//virtual void getImageInfoFromHwinfo();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getImageInfoFromHwinfo)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getImageInfoFromHwinfo();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceImage.size());
}

//virtual void getImageInfoFromLshw();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getImageInfoFromLshw)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getImageInfoFromHwinfo();
    m_deviceGenerator->getImageInfoFromLshw();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceImage.size());
}

//virtual void getCdromInfoFromHwinfo();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getCdromInfoFromHwinfo)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getCdromInfoFromHwinfo();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceCdrom.size());
}

//virtual void getCdromInfoFromLshw();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getCdromInfoFromLshw)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
    m_deviceGenerator->getCdromInfoFromHwinfo();
    m_deviceGenerator->getCdromInfoFromLshw();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceCdrom.size());
}

//virtual void getOthersInfoFromHwinfo();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getOthersInfoFromHwinfo)
{
//    Stub stub;
//    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
//    m_deviceGenerator->getOthersInfoFromHwinfo();
//    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceOthers.size());
}

//virtual void getOthersInfoFromLshw();
TEST_F(UT_DeviceGenerator, UT_DeviceGenerator_getOthersInfoFromLshw)
{
//    Stub stub;
//    stub.set(ADDR(DeviceManager, cmdInfo), ut_DeviceGenerator_cmdInfo);
//    m_deviceGenerator->getOthersInfoFromHwinfo();
//    m_deviceGenerator->getOthersInfoFromLshw();
//    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceOthers.size());
}
