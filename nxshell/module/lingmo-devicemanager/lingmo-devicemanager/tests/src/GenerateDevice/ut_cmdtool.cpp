// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CmdTool.h"
#include "DeviceFactory.h"
#include "GenerateDevicePool.h"
#include "DBusInterface.h"
#include "DeviceManager.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_CmdTool : public UT_HEAD
{
public:
    void SetUp()
    {
        m_cmdTool = new CmdTool;
    }
    void TearDown()
    {
        delete m_cmdTool;
    }
    CmdTool *m_cmdTool = nullptr;
};

TEST_F(UT_CmdTool, UT_CmdTool_loadCmdInfo)
{
    m_cmdTool->loadCmdInfo("lshw", "lshw.txt");
    m_cmdTool->loadCmdInfo("printer", "printer.txt");
    m_cmdTool->loadCmdInfo("dmidecode0", "dmidecode_0.txt");
    m_cmdTool->loadCmdInfo("dr_config", "dr_config.txt");
    m_cmdTool->loadCmdInfo("lscpu", "lscpu.txt");
    m_cmdTool->loadCmdInfo("xrandr", "xrandr.txt");
    m_cmdTool->loadCmdInfo("lsblk_d", "lsblk_d.txt");
    m_cmdTool->loadCmdInfo("hwinfo", "hwinfo.txt");
    m_cmdTool->loadCmdInfo("hciconfig", "hciconfig.txt");
    m_cmdTool->loadCmdInfo("upower", "upower_dump.txt");
    m_cmdTool->loadCmdInfo("ls_sg", "ls_sg.txt");
    m_cmdTool->loadCmdInfo("cat_version", "/proc/version");
    m_cmdTool->loadCmdInfo("cat_devices", "/proc/bus/input/devices");
    m_cmdTool->loadCmdInfo("dmesg", "dmesg.txt");
    m_cmdTool->loadCmdInfo("bt_device", "bt_device.txt");
    m_cmdTool->loadCmdInfo("cat_audio", "/proc/asound/card0/codec#0");
    m_cmdTool->loadCmdInfo("cat_gpuinfo", "/proc/gpuinfo_0");
    m_cmdTool->loadCmdInfo("bootdevice", "bootdevice.txt");
    m_cmdTool->loadCmdInfo("gpuinfo", "gpuinfo.txt");
    m_cmdTool->loadCmdInfo("cat_os_release", "/etc/os-release");
    EXPECT_TRUE(m_cmdTool->m_cmdInfo["cat_os_release"].size());
    EXPECT_TRUE(m_cmdTool->m_cmdInfo["cat_devices"].size());
    EXPECT_TRUE(m_cmdTool->m_cmdInfo["cat_version"].size());
}

QByteArray ut_readAllStandardOutput_getCurNetworkLinkStatus()
{
    return "eno1: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500\ninet 10.4.22.72  netmask 255.255.255.0  broadcast 10.4.22.255\n\nlo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536" ;
}
bool ut_waitForFinished_getCurNetworkLinkStatus()
{
    return true;
}
void ut_CmdTool_start()
{
    return;
}
TEST_F(UT_CmdTool, UT_CmdTool_getCurNetworkLinkStatus)
{
    Stub stub;
    stub.set(((void (QProcess::*)(const QString &, QIODevice::OpenMode))ADDR(QProcess, start)), ut_CmdTool_start); // void start(const QString &command, OpenMode mode = ReadWrite);
    stub.set(ADDR(QProcess, waitForFinished), ut_waitForFinished_getCurNetworkLinkStatus);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_readAllStandardOutput_getCurNetworkLinkStatus);
    EXPECT_STREQ("yes", m_cmdTool->getCurNetworkLinkStatus("eno1").toStdString().c_str());
    EXPECT_STREQ("no", m_cmdTool->getCurNetworkLinkStatus("lo").toStdString().c_str());
}

QByteArray ut_readAllStandardOutput_getCurPowerInfo()
{
    return "Device: /org/freedesktop/UPower/devices/battery_Battery\n"
           "  native-path:          Battery\n"
           "  power supply:         yes\n"
           "  has history:          yes\n"
           "  has statistics:       yes\n"
           "\n\nDevice: /org/freedesktop/UPower/devices/DisplayDevice\n"
           "  power supply:         yes\n"
           "  updated:              2021年09月04日 星期六 10时48分32秒 (11 seconds ago)\n"
           "  has history:          no\n"
           "  has statistics:       no\n"
           "\n\nDaemon:\n"
           "  daemon-version:  0.99.10\n"
           "  on-battery:      yes\n"
           "  lid-is-closed:   no\n"
           "  lid-is-present:  yes\n"
           ;
}
TEST_F(UT_CmdTool, UT_CmdTool_getCurPowerInfo)
{
    Stub stub;
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_readAllStandardOutput_getCurPowerInfo);
    QMap<QString, QMap<QString, QString>> mapMapInfo = m_cmdTool->getCurPowerInfo();
    EXPECT_EQ(mapMapInfo.size(), 2);
    EXPECT_EQ(mapMapInfo["Daemon"].size(), 4);
    EXPECT_EQ(mapMapInfo["upower"].size(), 5);
}

TEST_F(UT_CmdTool, UT_CmdTool_cmdInfo)
{
    EXPECT_TRUE(m_cmdTool->cmdInfo().isEmpty());
}

bool ut_getDeviceInfo_loadLshwInfo(void *obj, QString &deviceInfo, const QString &file)
{
    deviceInfo = "description: Desktop Computer\n"
                 "vendor: LENOVO\n"
                 "product: 10SMCTO1WW (LENOVO_MT_10SM_BU_Think_FM_ThinkCentre M920t-N000)\n"

                 "\n\n     *-cpu\n"
                 "product: Intel(R) Core(TM) i7-8700 CPU @ 3.20GHz\n"
                 "vendor: Intel Corp.\n"
                 "capacity: 4600MHz\n"

                 "\n\n     *-disk\n"
                 "description: SCSI Disk\n"
                 "product: Disk 3.0\n"
                 "vendor: USB\n"
                 "physical id: 0.0.0\n"

                 "\n\n    *-storage\n"
                 "description: SCSI Disk\n"
                 "product: Disk 3.0\n"
                 "vendor: USB\n"
                 "physical id: 0.0.0\n"

                 "\n\n    *-bank\n"
                 "description: DIMM DDR4 Synchronous 2667 MHz (0.4 ns)\n"
                 "product: M378A1K43CB2-CTD\n"

                 "\n\n    *-display\n"
                 "product: CometLake-S GT2 [UHD Graphics 630]\n"
                 "vendor: Intel Corporation\n"

                 "\n\n    *-multimedia\n"
                 "product: Cannon Lake PCH cAVS\n"
                 "vendor: Intel Corporation\n"

                 "\n\n    *-network\n"
                 "product: Ethernet Connection (7) I219-LM\n"
                 "vendor: Intel Corporation\n"

                 "\n\n    *-usb\n"
                 "Cannon Lake PCH USB 3.1 xHCI Host Controller\n"
                 "vendor: Intel Corporation\n"

                 "\n\n    *-cdrom\n"
                 "Cannon Lake PCH USB 3.1 xHCI Host Controller\n"
                 "vendor: Intel Corporation\n"
                 ;
    return true;
}
TEST_F(UT_CmdTool, UT_CmdTool_loadLshwInfo)
{
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_getDeviceInfo_loadLshwInfo);
    m_cmdTool->loadLshwInfo("lshw.txt");
    int size = m_cmdTool->m_cmdInfo.size();
    EXPECT_EQ(10, size);
}

bool ut_getDeviceInfo_loadLsblkInfo(void *obj, QString &deviceInfo, const QString &file)
{
    deviceInfo = "NAME ROTA\n"
                 "sda     1\n"
                 "sdb     1\n";
    return true;
}
TEST_F(UT_CmdTool, UT_CmdTool_loadLsblkInfo)
{
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_getDeviceInfo_loadLsblkInfo);
    m_cmdTool->loadLsblkInfo("lsblk_d.txt");
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.find("lsblk_d") != m_cmdTool->m_cmdInfo.end());
}

bool ut_getDeviceInfo_LoadLssgInfo(void *obj, QString &deviceInfo, const QString &file)
{
    deviceInfo = "/dev/sg0";
    return true;
}
TEST_F(UT_CmdTool, UT_CmdTool_loadLssgInfo)
{
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_getDeviceInfo_LoadLssgInfo);
    m_cmdTool->loadLssgInfo("ls_sg.txt");
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.find("smart") != m_cmdTool->m_cmdInfo.end());
}

bool ut_getDeviceInfo_loadSmartCtlInfo(void *obj, QString &deviceInfo, const QString &file)
{
    deviceInfo = "Model Family:     Western Digital Blue\n"
                 "Device Model:     WDC WD10EZEX-08WN4A0\n"
                 "Serial Number:    WD-WCC6Y5NY6N90\n"
                 "User Capacity:    1,000,204,886,016 bytes [1.00 TB]\n"
                 "Power_On_Hours          0x0032   100   100   050    Old_age   Always       -       4762\n"
                 "Power_Cycle_Count       0x0032   100   100   050    Old_age   Always       -       371\n"
                 "Raw_Read_Error_Rate     0x002f   100   100   050    Pre-fail  Always       -       0\n"
                 "Spin_Up_Time            0x0032   100   100   050    Old_age   Always       -       353820672\n"
                 "Start_Stop_Count        0x0032   100   100   050    Old_age   Always       -       353820672\n"
                 "Reallocated_Sector_Ct   0x0032   100   100   050    Old_age   Always       -       353820672\n"
                 "Seek_Error_Rate         0x0032   100   100   050    Old_age   Always       -       353820672\n"
                 "Spin_Retry_Count        0x0032   100   100   050    Old_age   Always       -       353820672\n"
                 "Calibration_Retry_Count 0x0032   100   100   050    Old_age   Always       -       353820672\n"
                 "G-Sense_Error_Rate      0x0032   100   100   050    Old_age   Always       -       353820672\n"
                 "Power-Off_Retract_Count 0x0032   100   100   050    Old_age   Always       -       353820672\n"
                 "Load_Cycle_Count        0x0032   100   100   050    Old_age   Always       -       353820672\n"
                 "Temperature_Celsius     0x0032   100   100   050    Old_age   Always       -       353820672\n"
                 "Reallocated_Event_Count 0x0032   100   100   050    Old_age   Always       -       353820672\n"
                 "Current_Pending_Sector  0x0032   100   100   050    Old_age   Always       -       353820672\n"
                 "UDMA_CRC_Error_Count    0x0032   100   100   050    Old_age   Always       -       353820672\n"
                 "Offline_Uncorrectable   0x0032   100   100   050    Old_age   Always       -       353820672\n"
                 "Multi_Zone_Error_Rate   0x0032   100   100   050    Old_age   Always       -       353820672\n";

    return true;
}
TEST_F(UT_CmdTool, UT_CmdTool_loadSmartCtlInfo)
{
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_getDeviceInfo_loadSmartCtlInfo);
    m_cmdTool->loadSmartCtlInfo("sda", "smartctl_sda.txt");
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.find("smart") != m_cmdTool->m_cmdInfo.end());
}

bool ui_getDeviceInfo_loadDmesgInfo(void *obj, QString &deviceInfo, const QString &file)
{
    deviceInfo = "nouveau 0000:01:00.0: VRAM: 2048MiB\n"
                 "nouveau 0000:01:00.0: VRAM Size 2048MiB\n"
                 "snd_hda_codec_realtek hdaudioC0D1: autoconfig for ALC891: \n"
                 "snd_hda_codec_realtek hdaudioC0D1: autoconfig for ALC891-VD:\n"
                 "snd_hda_codec_realtek hdaudioC0D1: autoconfig for ALC891 VD:";
    return true;
}
TEST_F(UT_CmdTool, UT_CmdTool_loadDmesgInfo)
{
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ui_getDeviceInfo_loadDmesgInfo);
    m_cmdTool->loadDmesgInfo("dmesg.txt");
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.find("dmesg") != m_cmdTool->m_cmdInfo.end());
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.find("audiochip") != m_cmdTool->m_cmdInfo.end());
}

bool UT_CmdTool_loadHciconfigInfo(void *obj, QString &deviceInfo, const QString &file)
{
    deviceInfo = "hci0:	Type: Primary  Bus: UART\n"
                 "	BD Address: E0:D4:62:EB:4C:38  ACL MTU: 1021:7  SCO MTU: 255:4\n"
                 "	UP RUNNING PSCAN ISCAN\n"
                 "	RX bytes:1500880 acl:0 sco:0 events:15271 errors:0\n"
                 "	TX bytes:20222 acl:0 sco:0 commands:1244 errors:0\n"
                 "	Features: 0xbf 0xfe 0x4f 0xfe 0xdb 0xff 0x5b 0x87\n"
                 "	Packet type: DM1 DM3 DM5 DH1 DH3 DH5 HV1 HV2 HV3\n"
                 "	Link policy: RSWITCH SNIFF\n"
                 "	Link mode: SLAVE ACCEPT\n"
                 "	Name: 'uos-PC'\n"
                 "	Class: 0x18010c\n"
                 "	Service Classes: Capturing, Object Transfer\n"
                 "	Device Class: Computer, Laptop\n"
                 "	HCI Version:  (0xa)  Revision: 0x8319\n"
                 "	LMP Version:  (0xa)  Subversion: 0x8319\n"
                 "	Manufacturer: HiSilicon Technologies Co., Ltd. (271)\n";
    return true;
}
DSysInfo::UosEdition ui_uosEditionType()
{
    return DSysInfo::UosHome;
}
TEST_F(UT_CmdTool, UT_CmdTool_loadHciconfigInfo_001)
{
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), UT_CmdTool_loadHciconfigInfo);
    m_cmdTool->loadHciconfigInfo("hciconfig.txt");
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.find("hciconfig") != m_cmdTool->m_cmdInfo.end());
}
TEST_F(UT_CmdTool, UT_CmdTool_loadHciconfigInfo_002)
{
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), UT_CmdTool_loadHciconfigInfo);
    stub.set(ADDR(DSysInfo, uosEditionType), ui_uosEditionType);
    m_cmdTool->loadHciconfigInfo("hciconfig.txt");
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.find("hciconfig") == m_cmdTool->m_cmdInfo.end());
}

TEST_F(UT_CmdTool, UT_CmdTool_loadBluetoothCtlInfo_001)
{
    QMap<QString, QString> mapInfo;
    m_cmdTool->loadBluetoothCtlInfo(mapInfo);
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.find("hciconfig") != m_cmdTool->m_cmdInfo.end());
}
bool UT_CmdTool_loadBluetoothCtlInfo_002(void *obj, QString &deviceInfo, const QString &file)
{
    deviceInfo = "	BD Address: E0:D4:62:EB:4C:38  ACL MTU: 1021:7  SCO MTU: 255:4";
    return false;
}
TEST_F(UT_CmdTool, UT_CmdTool_loadBluetoothCtlInfo_002)
{
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), UT_CmdTool_loadBluetoothCtlInfo_002);
    QMap<QString, QString> mapInfo;
    mapInfo.insert("BD Address", "E0:D4:62:EB:4C:38");
    m_cmdTool->loadBluetoothCtlInfo(mapInfo);
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.find("hciconfig") != m_cmdTool->m_cmdInfo.end());
}
bool UT_CmdTool_loadBluetoothCtlInfo_003(void *obj, QString &deviceInfo, const QString &file)
{
    deviceInfo = "	BD Address: E0:D4:62:EB:4C:38  ACL MTU: 1021:7  SCO MTU: 255:4";
    return true;
}
TEST_F(UT_CmdTool, UT_CmdTool_loadBluetoothCtlInfo_003)
{
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), UT_CmdTool_loadBluetoothCtlInfo_003);
    QMap<QString, QString> mapInfo;
    mapInfo.insert("BD Address", "E0:D4:62:EB:4C:38");
    m_cmdTool->loadBluetoothCtlInfo(mapInfo);
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.find("hciconfig") != m_cmdTool->m_cmdInfo.end());
}

TEST_F(UT_CmdTool, UT_CmdTool_loadPrinterInfo)
{
    // 这个用例必须有打印机才能有结果，且无法造数据
    m_cmdTool->loadPrinterInfo();
}

bool ut_getDeviceInfoFromCmd(void *obj, QString &deviceInfo, const QString &cmd)
{
    deviceInfo = "Hardware Class: monitor\n"
                 "Model: \"VIEWSONIC VA2430-FHD\"\n"
                 "Vendor: VSC \"VIEWSONIC\"\n"
                 "Serial ID: \"VSQ201321302\"\n";
    return true;
}
TEST_F(UT_CmdTool, UT_CmdTool_loadHwinfoInfo_001)
{
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfoFromCmd), ut_getDeviceInfoFromCmd);
    m_cmdTool->loadHwinfoInfo("hwinfo_monitor", "");
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.find("hwinfo_monitor") != m_cmdTool->m_cmdInfo.end());
}
bool ut_getDeviceInfo_loadHwinfoInfo(void *obj, QString &deviceInfo, const QString &file)
{
    deviceInfo = "Hardware Class: disk\n"
                 "Vendor: pci 0x8086 \"Intel Corporation\"\n"
                 "Device: pci 0xa348 \"Cannon Lake PCH cAVS\""
                 "\n\nHardware Class: mouse\n"
                 "Vendor: usb 0x248a \"Maxxter\"\n"
                 "Device: usb 0x8367 \"Wireless Receiver\"\n";
    return true;
}
TEST_F(UT_CmdTool, UT_CmdTool_loadHwinfoInfo_002)
{
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_getDeviceInfo_loadHwinfoInfo);
    m_cmdTool->loadHwinfoInfo("hwinfo", "");
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.find("hwinfo_disk") != m_cmdTool->m_cmdInfo.end());
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.find("hwinfo_mouse") != m_cmdTool->m_cmdInfo.end());
}

//void loadDmidecodeInfo(const QString &key, const QString &debugfile);
bool ut_getDeviceInfo_loadDmidecode2Info(void *obj, QString &deviceInfo, const QString &file)
{
    deviceInfo = "Manufacturer: LENOVO\n"
                 "Product Name: 3133\n"
                 "Version: NOK\n"
                 "Location In Chassis: Default string\n"
                 "Chassis Handle: 0x0003\n"
                 "Type: Motherboard\n"
                 "Contained Object Handles: 0";
    return true;
}
TEST_F(UT_CmdTool, UT_CmdTool_loadDmidecodeInfo_001)
{
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_getDeviceInfo_loadDmidecode2Info);
    m_cmdTool->loadDmidecodeInfo("dmidecode2", "");
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.find("dmidecode2") != m_cmdTool->m_cmdInfo.end());
}
bool ut_getDeviceInfo_loadDmidecodeInfo_002(void *obj, QString &deviceInfo, const QString &file)
{
    deviceInfo = "laptop";
    return true;
}
TEST_F(UT_CmdTool, UT_CmdTool_loadDmidecodeInfo_002)
{
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_getDeviceInfo_loadDmidecodeInfo_002);
    m_cmdTool->loadDmidecodeInfo("dmidecode3", "");
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.find("dmidecode3") == m_cmdTool->m_cmdInfo.end());
}
bool ut_getDeviceInfo_loadDmidecodeInfo_003(void *obj, QString &deviceInfo, const QString &file)
{
    deviceInfo = "Manufacturer: Samsung\n"
                 "Serial Number: 40F86BCE\n"
                 "Asset Tag: 9876543210\n"
                 "Configured Memory Speed: 2666 MT/s\n"
                 "Locator: ChannelA-DIMM1\n"
                 "Part Number: M378A1K43CB2-CTD\n"
                 "\n\nManufacturer: Samsung\n"
                 "Serial Number: 40F86BCE\n"
                 "Asset Tag: 9876543210\n"
                 "Configured Memory Speed: 2666 MT/s\n"
                 "Locator: ChannelA-DIMM1\n"
                 "Part Number: M378A1K43CB2-CTD\n";
    return true;
}
TEST_F(UT_CmdTool, UT_CmdTool_loadDmidecodeInfo_003)
{
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_getDeviceInfo_loadDmidecodeInfo_003);
    m_cmdTool->loadDmidecodeInfo("dmidecode17", "");
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.find("dmidecode17") != m_cmdTool->m_cmdInfo.end());
}
//void loadDmidecode2Info(const QString &key, const QString &debugfile);
TEST_F(UT_CmdTool, UT_CmdTool_loadDmidecode2Info)
{
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_getDeviceInfo_loadDmidecode2Info);
    m_cmdTool->loadDmidecode2Info("dmidecode2", "dmidecode2");
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.find("dmidecode2") != m_cmdTool->m_cmdInfo.end());
}

//void loadBiosInfoFromLspci(QString &chipsetFamliy);
bool ut_getDeviceInfo_loadBiosInfoFromLspci_001()
{
    return false;
}
TEST_F(UT_CmdTool, UT_CmdTool_loadBiosInfoFromLspci_001)
{
    QString chip;
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_getDeviceInfo_loadBiosInfoFromLspci_001);
    m_cmdTool->loadBiosInfoFromLspci(chip);
    EXPECT_STREQ(chip.toStdString().c_str(), "");
}
bool ut_getDeviceInfo_loadBiosInfoFromLspci_002(void *obj, QString &deviceInfo, const QString &file)
{
    deviceInfo = "Subsystem: chip";
    return true;
}
TEST_F(UT_CmdTool, UT_CmdTool_loadBiosInfoFromLspci_002)
{
    QString chip;
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_getDeviceInfo_loadBiosInfoFromLspci_002);
    m_cmdTool->loadBiosInfoFromLspci(chip);
    EXPECT_STREQ(chip.toStdString().c_str(), "chip");
}

//void addUsbMapInfo(const QString &key, const QMap<QString, QString> &mapInfo);
TEST_F(UT_CmdTool, UT_CmdTool_addUsbMapInfo)
{
    QList<QMap<QString, QString>> lstMapInfo;
    QMap<QString, QString> mapInfo;
    mapInfo.insert("SysFS BusID", "1-10:1.0");
    lstMapInfo.append(mapInfo);
    m_cmdTool->m_cmdInfo["hwinfo_usb"] = lstMapInfo;
    m_cmdTool->addUsbMapInfo("hwinfo_usb", mapInfo);

    QMap<QString, QString> mapInfo2;
    mapInfo2.insert("SysFS BusID", "1-11:1.0");
    m_cmdTool->addUsbMapInfo("hwinfo_usb", mapInfo2);
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.find("hwinfo_usb") != m_cmdTool->m_cmdInfo.end());
}

bool ut_getCatDeviceInfo_catInput(void *obj, QString &deviceInfo, const QString &file)
{
    deviceInfo = "I: Bus=0003 Vendor=093a Product=2510 Version=0111\n"
                 "N: Name=\"PixA琀 USB Optical Mouse\"\n"
                 "P: Phys=usb-0000:00:14.0-1/input0\n"
                 "S: Sysfs=/devices/pci0000:00/0000:00:14.0/usb1/1-1/1-1:1.0/0003:093A:2510.0001/input/input2\n"
                 "U: Uniq=\n"
                 "H: Handlers=mouse0\n"
                 "B: KEY=70000 0 0 0 0\n"
                 "B: EV=17\n"
                 "B: REL=103\n"
                 "B: MSC=10\n";
    return true;
}

TEST_F(UT_CmdTool, UT_CmdTool_loadCatInputDeviceInfo)
{
    Stub stub;
    stub.set(ADDR(CmdTool, getCatDeviceInfo), ut_getCatDeviceInfo_catInput);

    m_cmdTool->loadCatInputDeviceInfo("cat_devices", "/proc/bus/input/devices");
    EXPECT_FALSE(DeviceManager::instance()->inputInfo("mouse0").isEmpty());
}

bool ut_getCatDeviceInfo_catVersion(void *obj, QString &deviceInfo, const QString &file)
{
    deviceInfo = "I: Bus=0003 Vendor=093a Product=2510 Version=0111\n"
                 "N: Name=\"PixA琀 USB Optical Mouse\"\n"
                 "P: Phys=usb-0000:00:14.0-1/input0\n"
                 "S: Sysfs=/devices/pci0000:00/0000:00:14.0/usb1/1-1/1-1:1.0/0003:093A:2510.0001/input/input2\n"
                 "U: Uniq=\n"
                 "H: Handlers=mouse0\n"
                 "B: KEY=70000 0 0 0 0\n"
                 "B: EV=17\n"
                 "B: REL=103\n"
                 "B: MSC=10\n";
    return true;
}

TEST_F(UT_CmdTool, UT_CmdTool_loadCatAudioInfo)
{
    Stub stub;
    stub.set(ADDR(CmdTool, getCatDeviceInfo), ut_getCatDeviceInfo_catVersion);

    m_cmdTool->loadCatAudioInfo("cat_version", "/proc/version");
    EXPECT_FALSE(m_cmdTool->m_cmdInfo["cat_version"].isEmpty());
}

bool ut_cmdtool_getInfo(void *obj, const QString &key, QString &info)
{
    info = "Manufacturer: LENOVO\n"
           "Product Name: 3133\n"
           "Version: NOK\n";
    return true;
}

TEST_F(UT_CmdTool, UT_CmdTool_getDeviceInfo)
{
    Stub stub;
    stub.set(ADDR(DBusInterface, getInfo), ut_cmdtool_getInfo);

    QString deviceInfo;
    m_cmdTool->getDeviceInfo(deviceInfo, "dmidecode2");
    EXPECT_STREQ("Manufacturer: LENOVO\nProduct Name: 3133\nVersion: NOK\n", deviceInfo.toStdString().c_str());
}
