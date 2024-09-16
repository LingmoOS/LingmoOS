// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PanguVGenerator.h"
#include "PanguGenerator.h"
#include "MipsGenerator.h"
#include "DeviceManager.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

QList<QMap<QString, QString> > panGuLstMap;
class UT_PanguVGenerator : public UT_HEAD
{
public:
    void SetUp()
    {
        m_PanguVGenerator = new PanguVGenerator;
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
        panGuLstMap.append(mapInfo1);
        QMap<QString, QString> mapInfo2;
        mapInfo2.insert("Manufacturer","Samsung");
        mapInfo2.insert("product","M378A1K43CB2-CTD");
        mapInfo2.insert("width","64 bits");
        mapInfo2.insert("OS","Linux version 4.19.0-amd64-desktop (uos@x86-compile-PC) (gcc version 8.3.0 (Uos 8.3.0.5-1+dde)) #4016 SMP Wed Jun 23 13:35:29 CST 2021");
        mapInfo2.insert("size","8GiB");
        mapInfo2.insert("Capacity","931 GB");
        mapInfo2.insert("SysFS BusID","0:0:0:0");
        mapInfo2.insert("Model","camera");
        panGuLstMap.append(mapInfo2);
    }
    void TearDown()
    {
        delete m_PanguVGenerator;
        panGuLstMap.clear();
    }
    PanguVGenerator *m_PanguVGenerator = nullptr;
};

class UT_PanguGenerator : public UT_HEAD
{
public:
    void SetUp()
    {
        m_PanguGenerator = new PanguGenerator;
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
        panGuLstMap.append(mapInfo1);
        QMap<QString, QString> mapInfo2;
        mapInfo2.insert("Manufacturer","Samsung");
        mapInfo2.insert("product","M378A1K43CB2-CTD");
        mapInfo2.insert("width","64 bits");
        mapInfo2.insert("OS","Linux version 4.19.0-amd64-desktop (uos@x86-compile-PC) (gcc version 8.3.0 (Uos 8.3.0.5-1+dde)) #4016 SMP Wed Jun 23 13:35:29 CST 2021");
        mapInfo2.insert("size","8GiB");
        mapInfo2.insert("Capacity","931 GB");
        mapInfo2.insert("SysFS BusID","0:0:0:0");
        mapInfo2.insert("Model","camera");
        panGuLstMap.append(mapInfo2);
    }
    void TearDown()
    {
        delete m_PanguGenerator;
        panGuLstMap.clear();
    }
    PanguGenerator *m_PanguGenerator = nullptr;
};

class UT_MipsGenerator : public UT_HEAD
{
public:
    void SetUp()
    {
        m_MipsGenerator = new MipsGenerator;
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
        panGuLstMap.append(mapInfo1);
        QMap<QString, QString> mapInfo2;
        mapInfo2.insert("Manufacturer","Samsung");
        mapInfo2.insert("product","M378A1K43CB2-CTD");
        mapInfo2.insert("width","64 bits");
        mapInfo2.insert("OS","Linux version 4.19.0-amd64-desktop (uos@x86-compile-PC) (gcc version 8.3.0 (Uos 8.3.0.5-1+dde)) #4016 SMP Wed Jun 23 13:35:29 CST 2021");
        mapInfo2.insert("size","8GiB");
        mapInfo2.insert("Capacity","931 GB");
        mapInfo2.insert("SysFS BusID","0:0:0:0");
        mapInfo2.insert("Model","camera");
        panGuLstMap.append(mapInfo2);
    }
    void TearDown()
    {
        delete m_MipsGenerator;
        panGuLstMap.clear();
    }
    MipsGenerator *m_MipsGenerator = nullptr;
};

const QList<QMap<QString, QString> > & ut_PanguGenerator_cmdInfo(){
    return panGuLstMap;
}
// MipsGenerator virtual void generatorComputerDevice() override;
TEST_F(UT_MipsGenerator,UT_MipsGenerator_generatorComputerDevice){
    Stub stub;
    stub.set(ADDR(DeviceManager,cmdInfo),ut_PanguGenerator_cmdInfo);
    m_MipsGenerator->generatorComputerDevice();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceComputer.size());
}

// PanguGenerator virtual void generatorComputerDevice() override;
TEST_F(UT_PanguGenerator,UT_PanguGenerator_generatorComputerDevice){
    Stub stub;
    stub.set(ADDR(DeviceManager,cmdInfo),ut_PanguGenerator_cmdInfo);
    m_PanguGenerator->generatorComputerDevice();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceComputer.size());
}

// PanguVGenerator virtual void generatorMonitorDevice() override;
TEST_F(UT_PanguVGenerator,UT_PanguVGenerator_generatorComputerDevice){
    Stub stub;
    stub.set(ADDR(DeviceManager,cmdInfo),ut_PanguGenerator_cmdInfo);
    m_PanguVGenerator->generatorMonitorDevice();
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceMonitor.size());
}


