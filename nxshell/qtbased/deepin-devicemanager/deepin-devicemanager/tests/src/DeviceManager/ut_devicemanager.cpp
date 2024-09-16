// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceManager.h"
#include "DeviceInput.h"
#include "DeviceCpu.h"
#include "DeviceGpu.h"
#include "DeviceStorage.h"
#include "DeviceMemory.h"
#include "DeviceMonitor.h"
#include "DeviceBios.h"
#include "DeviceBluetooth.h"
#include "DeviceAudio.h"
#include "DeviceNetwork.h"
#include "DeviceImage.h"
#include "DeviceOthers.h"
#include "DevicePower.h"
#include "DevicePrint.h"
#include "DeviceOtherPCI.h"
#include "DeviceComputer.h"
#include "DeviceCdrom.h"

#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QIODevice>

#include <gtest/gtest.h>

class UT_DeviceManager : public UT_HEAD
{
public:
    void SetUp()
    {
    }
    void TearDown()
    {
    }
};

TEST_F(UT_DeviceManager, UT_DeviceManager_clear)
{
    DeviceManager::instance()->clear();
    EXPECT_EQ(0, DeviceManager::instance()->m_cmdInfo.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceMouse.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceStorage.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceMonitor.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceBluetooth.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceAudio.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceImage.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceKeyboard.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceOthers.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDevicePower.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDevicePrint.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceOtherPCI.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceCdrom.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceComputer.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceNetwork.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceBios.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceGPU.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceMemory.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceCPU.size());
}


TEST_F(UT_DeviceManager, UT_DeviceManager_getDeviceTypes)
{
//    DeviceManager::instance()->getDeviceTypes();
//    DeviceManager::instance()->m_CpuNum = 2;
//    EXPECT_EQ(5, DeviceManager::instance()->m_ListDeviceType.size());

//    DeviceManager::instance()->m_CpuNum = 0;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setDeviceListClass)
{
    DeviceManager::instance()->setDeviceListClass();
    EXPECT_EQ(17, DeviceManager::instance()->m_DeviceClassMap.size());
}

TEST_F(UT_DeviceManager, UT_DeviceManager_getDeviceList_001)
{
    QList<DeviceBaseInfo *> lst;
    bool ret = DeviceManager::instance()->getDeviceList(QObject::tr("Overview"), lst);
    EXPECT_FALSE(ret);
}

TEST_F(UT_DeviceManager, UT_DeviceManager_getDeviceList_002)
{
    DeviceInput *device1 = new DeviceInput;
    DeviceInput *device2 = new DeviceInput;
    QList<DeviceBaseInfo *> lstInfo;
    lstInfo.append(device1);
    lstInfo.append(device2);
    DeviceManager::instance()->m_DeviceClassMap.insert("input", lstInfo);

    bool ret = DeviceManager::instance()->getDeviceList("over", lstInfo);
    EXPECT_EQ(2, lstInfo.size());
    EXPECT_TRUE(ret);

    DeviceManager::instance()->m_DeviceClassMap.clear();
    delete device1;
    delete device2;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addMouseDevice)
{
    DeviceInput *device = new DeviceInput;

    DeviceManager::instance()->addMouseDevice(device);
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceMouse.size());

    DeviceManager::instance()->m_ListDeviceMouse.clear();
    delete device;
}

void ut_manager_setLshwMap(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("bus info", "usb@1:8");
    mapinfo.insert("vendor", "Cherry Zhuhai");
    mapinfo.insert("version", "1.07");
    mapinfo.insert("capabilities", "usb-2.00");
    mapinfo.insert("description", "Keyboard");
    mapinfo.insert("driver", "usbhid");
    mapinfo.insert("maxpower", "350mA");
    mapinfo.insert("speed", "12Mbit/s");
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addMouseInfoFromLshw_001)
{
//    DeviceInput *device = new DeviceInput;
//    device->m_KeyToLshw = "usb@1:8";
//    DeviceManager::instance()->m_ListDeviceMouse.append(device);

//    QMap<QString, QString> mapinfo;
//    ut_manager_setLshwMap(mapinfo);
//    bool ret = DeviceManager::instance()->addMouseInfoFromLshw(mapinfo);
//    EXPECT_TRUE(ret);
//    delete device;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addMouseInfoFromLshw_002)
{
    DeviceManager::instance()->m_ListDeviceMouse.clear();
    QMap<QString, QString> mapinfo;
    ut_manager_setLshwMap(mapinfo);
    bool ret = DeviceManager::instance()->addMouseInfoFromLshw(mapinfo);
    EXPECT_FALSE(ret);
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addCpuDevice)
{
    DeviceCpu *cpu = new DeviceCpu;
    DeviceManager::instance()->addCpuDevice(cpu);
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceCPU.size());
    DeviceManager::instance()->m_ListDeviceCPU.clear();
    delete cpu;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addStorageDeivce)
{
    DeviceStorage *st = new DeviceStorage;
    DeviceManager::instance()->addStorageDeivce(st);
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceStorage.size());
    DeviceManager::instance()->m_ListDeviceStorage.clear();
    delete st;
}

void ut_manager_setLshwstorage(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("bus info", "scsi@2:0.0.0");
    mapinfo.insert("interface", "sata");
    mapinfo.insert("capabilities", "gpt-1.00 partitioned partitioned:gpt");
    mapinfo.insert("version", "R013");
    mapinfo.insert("serial", "2002E3E0B393");
    mapinfo.insert("product", "CT240BX500SSD1");
    mapinfo.insert("description", "ATA Disk");
    mapinfo.insert("size", "223GiB (240GB)");
    mapinfo.insert("vendor", "vendor");
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addLshwinfoIntoStorageDevice)
{
    DeviceStorage *device = new DeviceStorage;
    device->m_KeyToLshw = "2:0:0:0";
    DeviceManager::instance()->m_ListDeviceStorage.append(device);

    QMap<QString, QString> mapinfo;
    ut_manager_setLshwstorage(mapinfo);

    DeviceManager::instance()->addLshwinfoIntoStorageDevice(mapinfo);
    EXPECT_STREQ("sata", device->m_Interface.toStdString().c_str());
    EXPECT_STREQ("scsi2", device->m_KeyFromStorage.toStdString().c_str());

    DeviceManager::instance()->m_ListDeviceStorage.clear();
    delete device;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addLshwinfoIntoNVMEStorageDevice)
{
    DeviceStorage *device = new DeviceStorage;
    device->m_NvmeKey = "/devices/pci0000:00/0000:00:17.0/ata3/host2/target2:0:0/2:0.0.0";
    DeviceManager::instance()->m_ListDeviceStorage.append(device);

    QMap<QString, QString> mapinfo;
    ut_manager_setLshwstorage(mapinfo);

    DeviceManager::instance()->addLshwinfoIntoNVMEStorageDevice(mapinfo);
    EXPECT_STREQ("vendor", device->m_Vendor.toStdString().c_str());

    DeviceManager::instance()->m_ListDeviceStorage.clear();
    delete device;
}

void ut_manager_setSmartCtlInfo(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("Firmware Version", "M6CR013");
    mapinfo.insert("SATA Version is", "SATA 3.2, 6.0 Gb/s (current: 6.0 Gb/s)");
    mapinfo.insert("Rotation Rate", "Solid State Device");
    mapinfo.insert("Power_On_Hours", "3717");
    mapinfo.insert("Power_Cycle_Count", "306");
    mapinfo.insert("User Capacity", "240,057,409,536 bytes [240 GB]");
    mapinfo.insert("Device Model", "CT240BX500SSD1");
    mapinfo.insert("Serial Number", "2002E3E0B393");
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setStorageInfoFromSmartctl)
{
    DeviceStorage *device = new DeviceStorage;
    device->m_DeviceFile = "/dev/sda";
    DeviceManager::instance()->m_ListDeviceStorage.append(device);

    QMap<QString, QString> mapinfo;
    ut_manager_setSmartCtlInfo(mapinfo);

    DeviceManager::instance()->setStorageInfoFromSmartctl("sda", mapinfo);
    EXPECT_STREQ("M6CR013", device->m_FirmwareVersion.toStdString().c_str());

    DeviceManager::instance()->m_ListDeviceStorage.clear();
    delete device;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setStorageDeviceMediaType)
{
    DeviceStorage *device = new DeviceStorage;
    device->m_DeviceFile = "/dev/sda";
    DeviceManager::instance()->m_ListDeviceStorage.append(device);

    DeviceManager::instance()->setStorageDeviceMediaType("sda", "1");
    EXPECT_STREQ("HDD", device->m_MediaType.toStdString().c_str());

    DeviceManager::instance()->m_ListDeviceStorage.clear();
    delete device;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setKLUStorageDeviceMediaType)
{
    DeviceStorage *device = new DeviceStorage;
    device->m_DeviceFile = "/dev/sda";
    DeviceManager::instance()->m_ListDeviceStorage.append(device);

    DeviceManager::instance()->setKLUStorageDeviceMediaType("sda", "1");
    EXPECT_STREQ("HDD", device->m_MediaType.toStdString().c_str());

    DeviceManager::instance()->m_ListDeviceStorage.clear();
    delete device;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addGpuDevice)
{
    DeviceGpu *gpu = new DeviceGpu;
    DeviceManager::instance()->addGpuDevice(gpu);
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceGPU.size());
    DeviceManager::instance()->m_ListDeviceGPU.clear();
    delete gpu;
}

void ut_manager_setgpuxrandrmap(QMap<QString, QString> &mapInfo)
{
    mapInfo.insert("minResolution", "minResolution");
    mapInfo.insert("curResolution", "curResolution");
    mapInfo.insert("maxResolution", "maxResolution");
    mapInfo.insert("HDMI", "Unable");
    mapInfo.insert("VGA", "Unable");
    mapInfo.insert("DP", "Unable");
    mapInfo.insert("eDP", "Unable");
    mapInfo.insert("DVI", "Unable");
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setGpuInfoFromXrandr)
{
    QMap<QString, QString> mapinfo;
    ut_manager_setgpuxrandrmap(mapinfo);

    DeviceGpu *gpu = new DeviceGpu;
    DeviceManager::instance()->m_ListDeviceGPU.append(gpu);

    DeviceManager::instance()->setGpuInfoFromXrandr(mapinfo);
    EXPECT_STREQ("minResolution", gpu->m_MinimumResolution.toStdString().c_str());

    DeviceManager::instance()->m_ListDeviceGPU.clear();
    delete gpu;
}

void ut_manager_setgpuLshwInfo(QMap<QString, QString> &mapLshw)
{
    mapLshw.insert("product", "GK208B [GeForce GT 730]");
    mapLshw.insert("vendor", "NVIDIA Corporation");
    mapLshw.insert("bus info", "pci@0000:01:00.0");
    mapLshw.insert("version", "a1");
    mapLshw.insert("width", "64 bits");
    mapLshw.insert("clock", "33MHz");
    mapLshw.insert("irq", "126");
    mapLshw.insert("capabilities", "pm msi pciexpress vga_controller bus_master cap_list rom");
    mapLshw.insert("description", "VGA compatible controller");
    mapLshw.insert("driver", "nouveau");
    mapLshw.insert("ioport", "e000(size=128)");
    mapLshw.insert("memory", "de000000-deffffff  d0000000-d7ffffff  d8000000-d9ffffff  c0000-dffff");
    mapLshw.insert("physical id", "0");
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setGpuInfoFromLshw)
{
    QMap<QString, QString> mapinfo;
    ut_manager_setgpuLshwInfo(mapinfo);

    DeviceGpu *gpu = new DeviceGpu;
    gpu->m_HwinfoToLshw = "0000:01:00.0";
    DeviceManager::instance()->m_ListDeviceGPU.append(gpu);

    DeviceManager::instance()->setGpuInfoFromLshw(mapinfo);
    EXPECT_STREQ("GK208B [GeForce GT 730]", gpu->m_Name.toStdString().c_str());

    DeviceManager::instance()->m_ListDeviceGPU.clear();
    delete gpu;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_networkDriver)
{
    EXPECT_EQ(0, DeviceManager::instance()->networkDriver().size());
}

TEST_F(UT_DeviceManager, UT_DeviceManager_correctPowerInfo)
{
    QMap<QString, QString> map;
    map.insert("name", "name");
    map.insert("Device", "Device");
    map.insert("serial", "serial");
    QMap<QString, QMap<QString, QString>> info;
    info.insert("upower", map);

    DevicePower *p = new DevicePower;
    DeviceManager::instance()->m_ListDevicePower.append(p);

    DeviceManager::instance()->correctPowerInfo(info);
    EXPECT_STREQ("serial", p->m_SerialNumber.toStdString().c_str());

    DeviceManager::instance()->m_ListDevicePower.clear();
    delete p;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setGpuSizeFromDmesg)
{
    DeviceGpu *gpu = new DeviceGpu;
    gpu->m_HwinfoToLshw = "01:00.0";
    DeviceManager::instance()->m_ListDeviceGPU.append(gpu);

    QMap<QString, QString> dmesgInfoMap;
    dmesgInfoMap.insert("Size","01:00.0=2GB");
    DeviceManager::instance()->setGpuSizeFromDmesg(dmesgInfoMap);
    EXPECT_STREQ("2GB", gpu->m_GraphicsMemory.toStdString().c_str());
    DeviceManager::instance()->m_ListDeviceGPU.clear();
    delete gpu;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addMemoryDevice)
{
    DeviceMemory *m = new DeviceMemory;
    DeviceManager::instance()->addMemoryDevice(m);
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceMemory.size());
    DeviceManager::instance()->m_ListDeviceMemory.clear();
    delete m;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addMonitor)
{
    DeviceMonitor *m = new DeviceMonitor;
    DeviceManager::instance()->addMonitor(m);
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceMonitor.size());
    DeviceManager::instance()->m_ListDeviceMonitor.clear();
    delete m;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setMonitorInfoFromXrandr)
{
    DeviceMonitor *monitor = new DeviceMonitor;
    DeviceManager::instance()->addMonitor(monitor);
    monitor->m_Interface = "";
    monitor->m_ProductionWeek = "2020-03";
    QString main = "HDMI-1 connected primary 1920x1080+0+0 (normal left inverted right x axis y axis) 527mm x 296mm";
    QString edid = "00ffffffffffff005a63384001010101\n0d1e010380351d782ece65a657519f27\n";
    QString rate = "60.00Hz";

    DeviceManager::instance()->setMonitorInfoFromXrandr(main, edid, rate);
    EXPECT_STREQ("1920x1080@60.00Hz", monitor->m_CurrentResolution.toStdString().c_str());

    DeviceManager::instance()->m_ListDeviceMonitor.clear();
    delete monitor;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addBiosDevice)
{
    DeviceBios *bios = new DeviceBios;
    DeviceManager::instance()->addBiosDevice(bios);
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceBios.size());
    DeviceManager::instance()->m_ListDeviceBios.clear();
    delete bios;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setLanguageInfo)
{
    DeviceBios *bios = new DeviceBios;
    DeviceManager::instance()->m_ListDeviceBios.append(bios);

    QMap<QString, QString> mapinfo;
    mapinfo.insert("Language Description Format", "language");
    DeviceManager::instance()->setLanguageInfo(mapinfo);
    EXPECT_EQ(0, bios->m_LstOtherInfo.size());

    DeviceManager::instance()->m_ListDeviceBios.clear();
    delete bios;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addBluetoothDevice)
{
    DeviceBluetooth *b = new DeviceBluetooth;
    DeviceManager::instance()->addBluetoothDevice(b);
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceBluetooth.size());
    DeviceManager::instance()->m_ListDeviceBluetooth.clear();
    delete b;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setBluetoothInfoFromHwinfo)
{
    DeviceBluetooth *bth = new DeviceBluetooth;
    bth->m_Vendor = "Vendor";
    DeviceManager::instance()->m_ListDeviceBluetooth.append(bth);

    QMap<QString, QString> mapinfo;
    mapinfo.insert("Vendor", "Vendor");
    mapinfo.insert("Revision", "Revision");
    DeviceManager::instance()->setBluetoothInfoFromHwinfo(mapinfo);
    EXPECT_STREQ("Revision", bth->m_Version.toStdString().c_str());

    DeviceManager::instance()->m_ListDeviceBluetooth.clear();
    delete bth;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setBluetoothInfoFromLshw)
{
    DeviceBluetooth *bth = new DeviceBluetooth;
    bth->m_HwinfoToLshw = "unique";
    DeviceManager::instance()->m_ListDeviceBluetooth.append(bth);

    QMap<QString, QString> mapinfo;
    mapinfo.insert("bus info", "unique");
    mapinfo.insert("vendor", "Vendor");

    DeviceManager::instance()->setBluetoothInfoFromLshw(mapinfo);
    EXPECT_STREQ("Vendor", bth->m_Vendor.toStdString().c_str());

    DeviceManager::instance()->m_ListDeviceBluetooth.clear();
    delete bth;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addAudioDevice)
{
    DeviceAudio *a = new DeviceAudio;
    DeviceManager::instance()->addAudioDevice(a);
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceAudio.size());
    DeviceManager::instance()->m_ListDeviceAudio.clear();
    delete a;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setAudioInfoFromLshw)
{
//    QMap<QString, QString> mapinfo;
//    mapinfo.insert("bus info", "usb@2.0");
//    mapinfo.insert("product", "product");

//    DeviceAudio *audio = new DeviceAudio;
//    DeviceManager::instance()->m_ListDeviceAudio.append(audio);

//    DeviceManager::instance()->setAudioInfoFromLshw(mapinfo);
//    EXPECT_STREQ("product", audio->m_Name.toStdString().c_str());

//    DeviceManager::instance()->m_ListDeviceAudio.clear();
//    delete audio;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setAudioChipFromDmesg)
{
    DeviceAudio *audio = new DeviceAudio;
    DeviceManager::instance()->m_ListDeviceAudio.append(audio);

    DeviceManager::instance()->setAudioChipFromDmesg("st");
    EXPECT_STREQ("st", audio->m_Chip.toStdString().c_str());

    DeviceManager::instance()->m_ListDeviceAudio.clear();
    delete audio;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addNetworkDevice)
{
    DeviceNetwork *n = new DeviceNetwork;
    DeviceManager::instance()->addNetworkDevice(n);
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceNetwork.size());
    DeviceManager::instance()->m_ListDeviceNetwork.clear();
    delete n;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addImageDevice)
{
    DeviceImage *image = new DeviceImage;
    DeviceManager::instance()->addImageDevice(image);
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceImage.size());
    DeviceManager::instance()->m_ListDeviceImage.clear();
    delete image;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addKeyboardDevice)
{
    DeviceInput *device = new DeviceInput;
    DeviceManager::instance()->addKeyboardDevice(device);
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceKeyboard.size());
    DeviceManager::instance()->m_ListDeviceKeyboard.clear();
    delete device;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setKeyboardInfoFromLshw)
{
//    QMap<QString, QString> mapinfo;
//    mapinfo.insert("vendor", "vendor");

//    DeviceInput *device = new DeviceInput;
//    DeviceManager::instance()->m_ListDeviceKeyboard.append(device);

//    DeviceManager::instance()->setKeyboardInfoFromLshw(mapinfo);
//    EXPECT_STREQ("vendor", device->m_Vendor.toStdString().c_str());

//    DeviceManager::instance()->m_ListDeviceKeyboard.clear();
//    delete device;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_correctNetworkLinkStatus)
{
    DeviceNetwork *net = new DeviceNetwork;
    net->m_LogicalName = "en2sp0";
    net->m_Link = "no";
    DeviceManager::instance()->m_ListDeviceNetwork.append(net);

    DeviceManager::instance()->correctNetworkLinkStatus("yes", "en2sp0");
    EXPECT_STREQ("yes", net->m_Link.toStdString().c_str());

    DeviceManager::instance()->m_ListDeviceNetwork.clear();
    delete net;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addOthersDevice)
{
    DeviceOthers *o = new DeviceOthers;
    DeviceManager::instance()->addOthersDevice(o);
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceOthers.size());
    DeviceManager::instance()->m_ListDeviceOthers.clear();
    delete o;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setOthersDeviceInfoFromLshw)
{
//    DeviceOthers *other = new DeviceOthers;
//    other->m_BusInfo = "bus";
//    DeviceManager::instance()->m_ListDeviceOthers.append(other);

//    QMap<QString, QString> mapinfo;
//    mapinfo.insert("bus info", "bus");
//    mapinfo.insert("product", "product");

//    DeviceManager::instance()->setOthersDeviceInfoFromLshw(mapinfo);
//    EXPECT_STREQ("product", other->m_Name.toStdString().c_str());

//    DeviceManager::instance()->m_ListDeviceOthers.clear();
//    delete other;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addPowerDevice)
{
    DevicePower *p = new DevicePower;
    DeviceManager::instance()->addPowerDevice(p);
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDevicePower.size());
    DeviceManager::instance()->m_ListDevicePower.clear();
    delete p;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addPrintDevice)
{
    DevicePrint *print = new DevicePrint;
    DeviceManager::instance()->addPrintDevice(print);
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDevicePrint.size());
    DeviceManager::instance()->m_ListDevicePrint.clear();
    delete print;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addOtherPCIDevice)
{
    DeviceOtherPCI *o = new DeviceOtherPCI;
    DeviceManager::instance()->addOtherPCIDevice(o);
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceOtherPCI.size());
    DeviceManager::instance()->m_ListDeviceOtherPCI.clear();
    delete o;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addComputerDevice)
{
    DeviceComputer *c = new DeviceComputer;
    DeviceManager::instance()->addComputerDevice(c);
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceComputer.size());
    DeviceManager::instance()->m_ListDeviceComputer.clear();
    delete c;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addCdromDevice)
{
    DeviceCdrom *c = new DeviceCdrom;
    DeviceManager::instance()->addCdromDevice(c);
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceCdrom.size());
    DeviceManager::instance()->m_ListDeviceCdrom.clear();
    delete c;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addLshwinfoIntoCdromDevice)
{
//    DeviceCdrom *cd = new DeviceCdrom;
//    cd->m_HwinfoToLshw = "key";
//    DeviceManager::instance()->m_ListDeviceCdrom.append(cd);

//    QMap<QString, QString> mapinfo;
//    mapinfo.insert("bus info", "usb@key");
//    mapinfo.insert("product", "product");

//    DeviceManager::instance()->addLshwinfoIntoCdromDevice(mapinfo);
//    EXPECT_STREQ("product", cd->m_Name.toStdString().c_str());

//    DeviceManager::instance()->m_ListDeviceCdrom.clear();
//    delete cd;
}


TEST_F(UT_DeviceManager, UT_DeviceManager_addBusId)
{
    QStringList lst;
    lst << "bus";

    DeviceManager::instance()->addBusId(lst);
    EXPECT_EQ(1, DeviceManager::instance()->m_BusIdList.size());

    DeviceManager::instance()->m_BusIdList.clear();
}

TEST_F(UT_DeviceManager, UT_DeviceManager_getBusId)
{
    QStringList lst;
    lst << "bus";
    DeviceManager::instance()->addBusId(lst);
    QStringList ret = DeviceManager::instance()->getBusId();

    EXPECT_EQ(1, ret.size());
    DeviceManager::instance()->m_BusIdList.clear();
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addCmdInfo_001)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("name", "name");
    QList<QMap<QString, QString>> info;
    info.append(mapinfo);
    QMap<QString, QList<QMap<QString, QString>>> cmdInfo;
    cmdInfo.insert("audio", info);
    DeviceManager::instance()->addCmdInfo(cmdInfo);
    EXPECT_EQ(1, DeviceManager::instance()->m_cmdInfo.size());
    EXPECT_EQ(1, DeviceManager::instance()->m_cmdInfo["audio"].size());
}

void ut_manager_setcmdinfo()
{
    QString key = "audio";
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Name", "Name");
    QList<QMap<QString, QString>> lst;
    lst.append(mapinfo);
    DeviceManager::instance()->m_cmdInfo.insert(key, lst);
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addCmdInfo_002)
{
    ut_manager_setcmdinfo();
    QMap<QString, QString> mapinfo;
    mapinfo.insert("name", "name");
    QList<QMap<QString, QString>> info;
    info.append(mapinfo);
    QMap<QString, QList<QMap<QString, QString>>> cmdInfo;
    cmdInfo.insert("audio", info);
    DeviceManager::instance()->addCmdInfo(cmdInfo);
    EXPECT_EQ(1, DeviceManager::instance()->m_cmdInfo.size());
    EXPECT_EQ(2, DeviceManager::instance()->m_cmdInfo["audio"].size());
}

TEST_F(UT_DeviceManager, UT_DeviceManager_cmdInfo)
{
    QList<QMap<QString, QString>> lst = DeviceManager::instance()->cmdInfo("audio");
    EXPECT_EQ(2, lst.size());
}

TEST_F(UT_DeviceManager, UT_DeviceManager_getDeviceOverview)
{
    DeviceManager::instance()->getDeviceOverview();
    EXPECT_EQ(0, DeviceManager::instance()->m_OveriewMap.size());
}

void ut_manager_setClassMap()
{
    DeviceCpu *cpu = new DeviceCpu;
    cpu->m_Driver = "";
    QList<DeviceBaseInfo *> lstcpu;
    lstcpu.append(dynamic_cast<DeviceBaseInfo *>(cpu));
    DeviceManager::instance()->m_DeviceClassMap.insert("cpu", lstcpu);

    DeviceAudio *audio = new DeviceAudio;
    audio->m_Driver = "video";
    audio->m_Name = "name";
    QList<DeviceBaseInfo *> lstaudio;
    lstaudio.append(dynamic_cast<DeviceBaseInfo *>(audio));
    DeviceManager::instance()->m_DeviceClassMap.insert("audio", lstaudio);
}

void ut_manager_setdriverpool()
{
    QMap<QString, QStringList> mapaudio;
    QString keyaudio;
    QStringList strLstAudio;
    mapaudio.insert(keyaudio, strLstAudio);
    DeviceManager::instance()->m_DeviceDriverPool.insert("video", mapaudio);
}

TEST_F(UT_DeviceManager, UT_DeviceManager_getDeviceDriverPool)
{
    ut_manager_setClassMap();
    ut_manager_setdriverpool();
    DeviceManager::instance()->getDeviceDriverPool();

    EXPECT_EQ(1, DeviceManager::instance()->m_DeviceDriverPool.size());
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addInputInfo)
{
    DeviceManager::instance()->m_InputDeviceInfo.clear();
    QMap<QString, QString> mapinfo;
    mapinfo.insert("name", "name");
    DeviceManager::instance()->addInputInfo("key", mapinfo);
    EXPECT_EQ(1, DeviceManager::instance()->m_InputDeviceInfo.size());

    DeviceManager::instance()->m_InputDeviceInfo.clear();
}

TEST_F(UT_DeviceManager, UT_DeviceManager_inputInfo)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("name", "name");
    DeviceManager::instance()->addInputInfo("key", mapinfo);
    QMap<QString, QString> map = DeviceManager::instance()->inputInfo("key");
    EXPECT_EQ(mapinfo, map);
}

QList<QMap<QString, QString>> &ut_manager_cmd_btdevice()
{
    static QList<QMap<QString, QString>> lst;
    QMap<QString, QString> map;
    map.insert("blue1", "blue1");
    lst.append(map);
    return lst;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_isDeviceExistInPairedDevice_001)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_manager_cmd_btdevice);
    bool ret = DeviceManager::instance()->isDeviceExistInPairedDevice("blue1");
    EXPECT_TRUE(ret);
}

TEST_F(UT_DeviceManager, UT_DeviceManager_isDeviceExistInPairedDevice_002)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_manager_cmd_btdevice);
    bool ret = DeviceManager::instance()->isDeviceExistInPairedDevice("blue2");
    EXPECT_FALSE(ret);
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setCpuNum)
{
    DeviceManager::instance()->setCpuNum(2);
    EXPECT_EQ(2, DeviceManager::instance()->m_CpuNum);

    DeviceManager::instance()->m_CpuNum = 0;
}

