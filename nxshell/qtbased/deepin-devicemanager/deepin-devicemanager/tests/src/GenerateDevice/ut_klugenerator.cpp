// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "KLUGenerator.h"
#include "DeviceManager.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

QList<QMap<QString, QString> > kluLstMap;
class UT_KLUGenerator : public UT_HEAD
{
public:
    void SetUp()
    {
        m_KLUGenerator = new KLUGenerator;
        QMap<QString, QString> mapInfo1;
        mapInfo1.insert("Manufacturer","Samsung");
        mapInfo1.insert("product","M378A1K43CB2-CTD");
        mapInfo1.insert("width","64 bits");
        mapInfo1.insert("OS","Linux version 4.19.0-amd64-desktop (uos@x86-compile-PC) (gcc version 8.3.0 (Uos 8.3.0.5-1+dde)) #4016 SMP Wed Jun 23 13:35:29 CST 2021");
        mapInfo1.insert("physical","8");
        mapInfo1.insert("core","8");
        mapInfo1.insert("logical","8");
        mapInfo1.insert("size","8GiB");
        mapInfo1.insert("Capacity","931 GB");
        mapInfo1.insert("SysFS BusID","0:0:0:0");
        mapInfo1.insert("Model","camera");
        mapInfo1.insert("Device Files","/sys/device/0");
        mapInfo1.insert("Device","Bluetooth");
        kluLstMap.append(mapInfo1);
        QMap<QString, QString> mapInfo2;
        mapInfo2.insert("Manufacturer","Samsung");
        mapInfo2.insert("product","M378A1K43CB2-CTD");
        mapInfo2.insert("width","64 bits");
        mapInfo2.insert("OS","Linux version 4.19.0-amd64-desktop (uos@x86-compile-PC) (gcc version 8.3.0 (Uos 8.3.0.5-1+dde)) #4016 SMP Wed Jun 23 13:35:29 CST 2021");
        mapInfo2.insert("size","8GiB");
        mapInfo2.insert("Capacity","931 GB");
        mapInfo2.insert("SysFS BusID","0:0:0:0");
        mapInfo2.insert("Model","camera");
        mapInfo2.insert("Device Files","/sys/device/0");
        mapInfo2.insert("Device","Bluetooth");
        kluLstMap.append(mapInfo2);
    }
    void TearDown()
    {
        delete m_KLUGenerator;
        kluLstMap.clear();
    }
    KLUGenerator *m_KLUGenerator = nullptr;
};

const QList<QMap<QString, QString> > & ut_DeviceGenerator_klu_cmdInfo(){
    return kluLstMap;
}

//virtual void generatorComputerDevice() override;
TEST_F(UT_KLUGenerator,UT_DeviceGenerator_generatorComputerDevice){
    Stub stub;
    stub.set(ADDR(DeviceManager,cmdInfo),ut_DeviceGenerator_klu_cmdInfo);
    m_KLUGenerator->generatorComputerDevice();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceComputer.size());
}

//virtual void generatorGpuDevice() override;
TEST_F(UT_KLUGenerator,UT_DeviceGenerator_generatorGpuDevice){
    Stub stub;
    stub.set(ADDR(DeviceManager,cmdInfo),ut_DeviceGenerator_klu_cmdInfo);
    m_KLUGenerator->generatorGpuDevice();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceGPU.size());
}

//virtual void generatorMonitorDevice() override;
TEST_F(UT_KLUGenerator,UT_DeviceGenerator_generatorMonitorDevice){
    Stub stub;
    stub.set(ADDR(DeviceManager,cmdInfo),ut_DeviceGenerator_klu_cmdInfo);
    m_KLUGenerator->generatorMonitorDevice();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceMonitor.size());
}

//virtual void generatorAudioDevice() override;
TEST_F(UT_KLUGenerator,UT_DeviceGenerator_generatorAudioDevice){
    Stub stub;
    stub.set(ADDR(DeviceManager,cmdInfo),ut_DeviceGenerator_klu_cmdInfo);
    m_KLUGenerator->generatorAudioDevice();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceAudio.size());
}

//virtual void generatorPowerDevice() override;
TEST_F(UT_KLUGenerator,UT_DeviceGenerator_generatorPowerDevice){
    Stub stub;
    stub.set(ADDR(DeviceManager,cmdInfo),ut_DeviceGenerator_klu_cmdInfo);
    m_KLUGenerator->generatorPowerDevice();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDevicePower.size());
}

//virtual void getKeyboardInfoFromHwinfo() override;
TEST_F(UT_KLUGenerator,UT_DeviceGenerator_getKeyboardInfoFromHwinfo){
    Stub stub;
    stub.set(ADDR(DeviceManager,cmdInfo),ut_DeviceGenerator_klu_cmdInfo);
    m_KLUGenerator->getKeyboardInfoFromHwinfo();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceKeyboard.size());
}

//virtual void getOthersInfoFromHwinfo() override;
TEST_F(UT_KLUGenerator,UT_DeviceGenerator_getOthersInfoFromHwinfo){
    Stub stub;
    stub.set(ADDR(DeviceManager,cmdInfo),ut_DeviceGenerator_klu_cmdInfo);
    m_KLUGenerator->getOthersInfoFromHwinfo();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceOthers.size());
}

//virtual void getDiskInfoFromHwinfo();
TEST_F(UT_KLUGenerator,UT_DeviceGenerator_getDiskInfoFromHwinfo){
    Stub stub;
    stub.set(ADDR(DeviceManager,cmdInfo),ut_DeviceGenerator_klu_cmdInfo);
    m_KLUGenerator->getDiskInfoFromHwinfo();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceStorage.size());
}

//virtual void getDiskInfoFromLshw();
TEST_F(UT_KLUGenerator,UT_DeviceGenerator_getDiskInfoFromLshw){
    Stub stub;
    stub.set(ADDR(DeviceManager,cmdInfo),ut_DeviceGenerator_klu_cmdInfo);
    m_KLUGenerator->getDiskInfoFromHwinfo();
    m_KLUGenerator->getDiskInfoFromLshw();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceStorage.size());
}

//virtual void getDiskInfoFromLsblk();
TEST_F(UT_KLUGenerator,UT_DeviceGenerator_getDiskInfoFromLsblk){
    Stub stub;
    stub.set(ADDR(DeviceManager,cmdInfo),ut_DeviceGenerator_klu_cmdInfo);
    m_KLUGenerator->getDiskInfoFromHwinfo();
    m_KLUGenerator->getDiskInfoFromLshw();
    m_KLUGenerator->getDiskInfoFromLsblk();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceStorage.size());
}

//virtual void getDiskInfoFromSmartCtl();
TEST_F(UT_KLUGenerator,UT_DeviceGenerator_getDiskInfoFromSmartCtl){
    Stub stub;
    stub.set(ADDR(DeviceManager,cmdInfo),ut_DeviceGenerator_klu_cmdInfo);
    m_KLUGenerator->getDiskInfoFromLshw();
    m_KLUGenerator->getDiskInfoFromLsblk();
    m_KLUGenerator->getDiskInfoFromSmartCtl();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceStorage.size());
}
//virtual void getAudioInfoFromCatAudio();
TEST_F(UT_KLUGenerator,UT_DeviceGenerator_getAudioInfoFromCatAudio){
    Stub stub;
    stub.set(ADDR(DeviceManager,cmdInfo),ut_DeviceGenerator_klu_cmdInfo);
    m_KLUGenerator->getAudioInfoFromCatAudio();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceAudio.size());
}
