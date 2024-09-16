// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

#define private public
#define protected public
#include "DeviceCpu.h"

class UT_DeviceCpu : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceCpu = new DeviceCpu;
    }
    void TearDown()
    {
        delete m_deviceCpu;
    }
    DeviceCpu *m_deviceCpu;
};

void setLscpuData(QMap<QString, QString> &mapLscpu)
{
    mapLscpu.insert("model name", "Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz");
    mapLscpu.insert("vendor_id", "GenuineIntel");
    mapLscpu.insert("Thread(s) per core", "1");
    mapLscpu.insert("bogomips", "7200.00");
    mapLscpu.insert("Architecture", "x86_64");
    mapLscpu.insert("cpu family", "6");
    mapLscpu.insert("CPU MHz", "4085.639");
    mapLscpu.insert("model", "158");
    mapLscpu.insert("stepping", "10");
    mapLscpu.insert("L1d cache", "128 KiB");
    mapLscpu.insert("L1i cache", "128 KiB");
    mapLscpu.insert("L2 cache", "1 MiB");
    mapLscpu.insert("L3 cache", "6 MiB");
    mapLscpu.insert("Flags", "MMX SSE SSE2 SSE3 3D Now SSE4 SSSE3 SSE4_1 SSE4_2 AMD64 EM64T");
    mapLscpu.insert("flags", "MMX SSE SSE2 SSE3 3D Now SSE4 SSSE3 SSE4_1 SSE4_2 AMD64 EM64T");
    mapLscpu.insert("Virtualization", "VT-x");
    mapLscpu.insert("processor", "0");
    mapLscpu.insert("core id", "0");
}

void setLshwData(QMap<QString, QString> &mapLshw)
{
    mapLshw.insert("product", "Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz");
    mapLshw.insert("version", "Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz");
    mapLshw.insert("vendor", "Intel Corp.");
}

void setDmidecodeData(QMap<QString, QString> &mapDmidecode)
{
    mapDmidecode.insert("product", "Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz");
    mapDmidecode.insert("Manufacturer", "Intel(R) Corporation");
    mapDmidecode.insert("Max Speed", "8300 MHz");
    mapDmidecode.insert("Family", "Core i3");
}

TEST_F(UT_DeviceCpu, UT_DeviceCpu_setCpuInfo)
{
    QMap<QString, QString> mapLscpu;
    QMap<QString, QString> mapLshw;
    QMap<QString, QString> mapDmidecode;

    setLscpuData(mapLscpu);
    setLshwData(mapLshw);
    setDmidecodeData(mapDmidecode);

    m_deviceCpu->setCpuInfo(mapLscpu, mapLshw, mapDmidecode, 4, 4);

    EXPECT_STREQ("Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz", m_deviceCpu->name().toStdString().c_str());
    EXPECT_EQ(4, m_deviceCpu->m_LogicalCPUNum);
    EXPECT_EQ(4, m_deviceCpu->m_CPUCoreNum);
}

TEST_F(UT_DeviceCpu, UT_DeviceCpu_initFilterKey)
{
    m_deviceCpu->initFilterKey();
    EXPECT_EQ(5, m_deviceCpu->m_FilterKey.size());
}

TEST_F(UT_DeviceCpu, UT_DeviceCpu_getOverviewInfo)
{
    QMap<QString, QString> mapLscpu;
    QMap<QString, QString> mapLshw;
    QMap<QString, QString> mapDmidecode;

    setLscpuData(mapLscpu);
    setLshwData(mapLshw);
    setDmidecodeData(mapDmidecode);

    m_deviceCpu->setCpuInfo(mapLscpu, mapLshw, mapDmidecode, 4, 4);

    QString overview = m_deviceCpu->getOverviewInfo();

    EXPECT_STREQ("Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz (FourCore(s) / FourProcessor)", overview.toStdString().c_str());
}

TEST_F(UT_DeviceCpu, UT_DeviceCpu_loadBaseDeviceInfo)
{
    QMap<QString, QString> mapLscpu;
    setLscpuData(mapLscpu);
    m_deviceCpu->setInfoFromLscpu(mapLscpu);
    m_deviceCpu->loadBaseDeviceInfo();

    QPair<QString, QString> value0 = m_deviceCpu->m_LstBaseInfo.at(0);
    EXPECT_STREQ("Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz", value0.second.toStdString().c_str());
    QPair<QString, QString> value1 = m_deviceCpu->m_LstBaseInfo.at(1);
    EXPECT_STREQ("GenuineIntel", value1.second.toStdString().c_str());
    QPair<QString, QString> value2 = m_deviceCpu->m_LstBaseInfo.at(2);
    EXPECT_STREQ("0", value2.second.toStdString().c_str());
    QPair<QString, QString> value3 = m_deviceCpu->m_LstBaseInfo.at(3);
    EXPECT_STREQ("0", value3.second.toStdString().c_str());
    QPair<QString, QString> value4 = m_deviceCpu->m_LstBaseInfo.at(4);
    EXPECT_STREQ("1", value4.second.toStdString().c_str());
    QPair<QString, QString> value5 = m_deviceCpu->m_LstBaseInfo.at(5);
    EXPECT_STREQ("4085.639", value5.second.toStdString().c_str());
    QPair<QString, QString> value6 = m_deviceCpu->m_LstBaseInfo.at(6);
    EXPECT_STREQ("7200.00", value6.second.toStdString().c_str());
    QPair<QString, QString> value7 = m_deviceCpu->m_LstBaseInfo.at(7);
    EXPECT_STREQ("x86_64", value7.second.toStdString().c_str());
    QPair<QString, QString> value8 = m_deviceCpu->m_LstBaseInfo.at(8);
    EXPECT_STREQ("6", value8.second.toStdString().c_str());
//    QPair<QString, QString> value9 = m_deviceCpu->m_LstBaseInfo.at(9);
//    EXPECT_STREQ("158", value9.second.toStdString().c_str());
}

TEST_F(UT_DeviceCpu, UT_DeviceCpu_loadOtherDeviceInfo)
{
    m_deviceCpu->loadOtherDeviceInfo();
    EXPECT_EQ(0, m_deviceCpu->m_LstOtherInfo.size());
}

TEST_F(UT_DeviceCpu, UT_DeviceCpu_loadTableHeader)
{
    m_deviceCpu->loadTableHeader();
    EXPECT_EQ(4, m_deviceCpu->m_TableHeader.size());
}

TEST_F(UT_DeviceCpu, UT_DeviceCpu_loadTableData)
{
    m_deviceCpu->loadTableData();
    EXPECT_EQ(4, m_deviceCpu->m_TableData.size());
}

TEST_F(UT_DeviceCpu, UT_DeviceCpu_name)
{
    QMap<QString, QString> mapLscpu;
    setLscpuData(mapLscpu);

    m_deviceCpu->setInfoFromLscpu(mapLscpu);
    QString name = m_deviceCpu->name();
    EXPECT_STREQ("Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz", name.toStdString().c_str());
}

TEST_F(UT_DeviceCpu, UT_DeviceCpu_vendor)
{
    QMap<QString, QString> mapLscpu;
    setLscpuData(mapLscpu);

    m_deviceCpu->setInfoFromLscpu(mapLscpu);
    QString vendor = m_deviceCpu->vendor();
    EXPECT_STREQ("GenuineIntel", vendor.toStdString().c_str());
}

TEST_F(UT_DeviceCpu, UT_DeviceCpu_driver)
{
    QString driver = m_deviceCpu->driver();
    EXPECT_STREQ("", driver.toStdString().c_str());
}

TEST_F(UT_DeviceCpu, UT_DeviceCpu_subTitle)
{
    QMap<QString, QString> mapLscpu;
    setLscpuData(mapLscpu);

    m_deviceCpu->setInfoFromLscpu(mapLscpu);
    QString title =  m_deviceCpu->subTitle();
    EXPECT_STREQ("Processor 0", title.toStdString().c_str());
}

TEST_F(UT_DeviceCpu, UT_DeviceCpu_frequencyIsRange_001)
{
    QMap<QString, QString> mapLscpu;
    setLscpuData(mapLscpu);

    m_deviceCpu->setInfoFromLscpu(mapLscpu);
    ASSERT_FALSE(m_deviceCpu->frequencyIsRange());
}

TEST_F(UT_DeviceCpu, UT_DeviceCpu_frequencyIsRange_002)
{
    QMap<QString, QString> mapLscpu;
    setLscpuData(mapLscpu);
    mapLscpu.insert("CPU min MHz", "800.0000");
    mapLscpu.insert("CPU max MHz", "4200.0000");

    m_deviceCpu->setInfoFromLscpu(mapLscpu);
    ASSERT_TRUE(m_deviceCpu->frequencyIsRange());
}

TEST_F(UT_DeviceCpu, UT_DeviceCpu_setInfoFromLscpu_001)
{
    QMap<QString, QString> mapLscpu;
    setLscpuData(mapLscpu);

    m_deviceCpu->setInfoFromLscpu(mapLscpu);
    EXPECT_STREQ("Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz", m_deviceCpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("GenuineIntel", m_deviceCpu->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("1", m_deviceCpu->m_ThreadNum.toStdString().c_str());
    EXPECT_STREQ("7200.00", m_deviceCpu->m_BogoMIPS.toStdString().c_str());
    EXPECT_STREQ("x86_64", m_deviceCpu->m_Architecture.toStdString().c_str());
    EXPECT_STREQ("6", m_deviceCpu->m_Familly.toStdString().c_str());
    EXPECT_STREQ("4085.639", m_deviceCpu->m_CurFrequency.toStdString().c_str());
    EXPECT_STREQ("158", m_deviceCpu->m_Model.toStdString().c_str());
    EXPECT_STREQ("10", m_deviceCpu->m_Step.toStdString().c_str());
    EXPECT_STREQ("128 KiB", m_deviceCpu->m_CacheL1Data.toStdString().c_str());
    EXPECT_STREQ("128 KiB", m_deviceCpu->m_CacheL1Order.toStdString().c_str());
    EXPECT_STREQ("1 MiB", m_deviceCpu->m_CacheL2.toStdString().c_str());
    EXPECT_STREQ("6 MiB", m_deviceCpu->m_CacheL3.toStdString().c_str());
    EXPECT_STREQ("MMX SSE SSE2 SSE3 3D Now SSE4 SSSE3 SSE4_1 SSE4_2 AMD64 EM64T", m_deviceCpu->m_Flags.toStdString().c_str());
    EXPECT_STREQ("VT-x", m_deviceCpu->m_HardwareVirtual.toStdString().c_str());
    EXPECT_STREQ("0", m_deviceCpu->m_PhysicalID.toStdString().c_str());
    EXPECT_STREQ("0", m_deviceCpu->m_CoreID.toStdString().c_str());
    EXPECT_STREQ("4085.639 MHz", m_deviceCpu->m_Frequency.toStdString().c_str());
    EXPECT_STREQ("MMX SSE SSE2 SSE3 3D Now SSE4 SSSE3 SSE4_1 SSE4_2 AMD64 EM64T ", m_deviceCpu->m_Extensions.toStdString().c_str());
    EXPECT_FALSE(m_deviceCpu->m_FrequencyIsRange);
}

TEST_F(UT_DeviceCpu, UT_DeviceCpu_setInfoFromLscpu_002)
{
    QMap<QString, QString> mapLscpu;
    setLscpuData(mapLscpu);
    mapLscpu.insert("CPU min MHz", "800.0000");
    mapLscpu.insert("CPU max MHz", "4200.0000");

    m_deviceCpu->setInfoFromLscpu(mapLscpu);
    EXPECT_STREQ("Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz", m_deviceCpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("GenuineIntel", m_deviceCpu->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("1", m_deviceCpu->m_ThreadNum.toStdString().c_str());
    EXPECT_STREQ("7200.00", m_deviceCpu->m_BogoMIPS.toStdString().c_str());
    EXPECT_STREQ("x86_64", m_deviceCpu->m_Architecture.toStdString().c_str());
    EXPECT_STREQ("6", m_deviceCpu->m_Familly.toStdString().c_str());
    EXPECT_STREQ("4085.639", m_deviceCpu->m_CurFrequency.toStdString().c_str());
    EXPECT_STREQ("158", m_deviceCpu->m_Model.toStdString().c_str());
    EXPECT_STREQ("10", m_deviceCpu->m_Step.toStdString().c_str());
    EXPECT_STREQ("128 KiB", m_deviceCpu->m_CacheL1Data.toStdString().c_str());
    EXPECT_STREQ("128 KiB", m_deviceCpu->m_CacheL1Order.toStdString().c_str());
    EXPECT_STREQ("1 MiB", m_deviceCpu->m_CacheL2.toStdString().c_str());
    EXPECT_STREQ("6 MiB", m_deviceCpu->m_CacheL3.toStdString().c_str());
    EXPECT_STREQ("MMX SSE SSE2 SSE3 3D Now SSE4 SSSE3 SSE4_1 SSE4_2 AMD64 EM64T", m_deviceCpu->m_Flags.toStdString().c_str());
    EXPECT_STREQ("VT-x", m_deviceCpu->m_HardwareVirtual.toStdString().c_str());
    EXPECT_STREQ("0", m_deviceCpu->m_PhysicalID.toStdString().c_str());
    EXPECT_STREQ("0", m_deviceCpu->m_CoreID.toStdString().c_str());
    EXPECT_STREQ("0.8-4.2 GHz", m_deviceCpu->m_Frequency.toStdString().c_str());
    EXPECT_STREQ("MMX SSE SSE2 SSE3 3D Now SSE4 SSSE3 SSE4_1 SSE4_2 AMD64 EM64T ", m_deviceCpu->m_Extensions.toStdString().c_str());
    EXPECT_TRUE(m_deviceCpu->m_FrequencyIsRange);
}

TEST_F(UT_DeviceCpu, UT_DeviceCpu_setInfoFromLscpu_003)
{
    QMap<QString, QString> mapLscpu;
    setLscpuData(mapLscpu);
    mapLscpu.insert("CPU min MHz", "800.0000");
    mapLscpu.insert("CPU max MHz", "800.0000");

    m_deviceCpu->setInfoFromLscpu(mapLscpu);
    EXPECT_STREQ("Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz", m_deviceCpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("GenuineIntel", m_deviceCpu->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("1", m_deviceCpu->m_ThreadNum.toStdString().c_str());
    EXPECT_STREQ("7200.00", m_deviceCpu->m_BogoMIPS.toStdString().c_str());
    EXPECT_STREQ("x86_64", m_deviceCpu->m_Architecture.toStdString().c_str());
    EXPECT_STREQ("6", m_deviceCpu->m_Familly.toStdString().c_str());
    EXPECT_STREQ("4085.639", m_deviceCpu->m_CurFrequency.toStdString().c_str());
    EXPECT_STREQ("158", m_deviceCpu->m_Model.toStdString().c_str());
    EXPECT_STREQ("10", m_deviceCpu->m_Step.toStdString().c_str());
    EXPECT_STREQ("128 KiB", m_deviceCpu->m_CacheL1Data.toStdString().c_str());
    EXPECT_STREQ("128 KiB", m_deviceCpu->m_CacheL1Order.toStdString().c_str());
    EXPECT_STREQ("1 MiB", m_deviceCpu->m_CacheL2.toStdString().c_str());
    EXPECT_STREQ("6 MiB", m_deviceCpu->m_CacheL3.toStdString().c_str());
    EXPECT_STREQ("MMX SSE SSE2 SSE3 3D Now SSE4 SSSE3 SSE4_1 SSE4_2 AMD64 EM64T", m_deviceCpu->m_Flags.toStdString().c_str());
    EXPECT_STREQ("VT-x", m_deviceCpu->m_HardwareVirtual.toStdString().c_str());
    EXPECT_STREQ("0", m_deviceCpu->m_PhysicalID.toStdString().c_str());
    EXPECT_STREQ("0", m_deviceCpu->m_CoreID.toStdString().c_str());
    EXPECT_STREQ("800 MHz", m_deviceCpu->m_Frequency.toStdString().c_str());
    EXPECT_STREQ("MMX SSE SSE2 SSE3 3D Now SSE4 SSSE3 SSE4_1 SSE4_2 AMD64 EM64T ", m_deviceCpu->m_Extensions.toStdString().c_str());
    EXPECT_FALSE(m_deviceCpu->m_FrequencyIsRange);
}

TEST_F(UT_DeviceCpu, UT_DeviceCpu_setInfoFromLshw_001)
{
    QMap<QString, QString> mapLshw;
    setLshwData(mapLshw);
    m_deviceCpu->m_Name = "Loongson";

    m_deviceCpu->setInfoFromLshw(mapLshw);
    EXPECT_STREQ("Loongson", m_deviceCpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("Intel Corp.", m_deviceCpu->m_Vendor.toStdString().c_str());
}

TEST_F(UT_DeviceCpu, UT_DeviceCpu_setInfoFromLshw_002)
{
    QMap<QString, QString> mapLshw;
    setLshwData(mapLshw);
    mapLshw.insert("product", "null");

    m_deviceCpu->setInfoFromLshw(mapLshw);
    EXPECT_STREQ("Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz", m_deviceCpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("Intel Corp.", m_deviceCpu->m_Vendor.toStdString().c_str());

}

TEST_F(UT_DeviceCpu, UT_DeviceCpu_setInfoFromDmidecode_001)
{
    QMap<QString, QString> mapDmidecode;
    setDmidecodeData(mapDmidecode);
    m_deviceCpu->m_Name = "Loongson";

    m_deviceCpu->setInfoFromDmidecode(mapDmidecode);
    EXPECT_STREQ("Loongson", m_deviceCpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("Intel(R) Corporation", m_deviceCpu->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("8300 MHz", m_deviceCpu->m_Frequency.toStdString().c_str());
    EXPECT_STREQ("Core i3", m_deviceCpu->m_Familly.toStdString().c_str());
}

TEST_F(UT_DeviceCpu, UT_DeviceCpu_setInfoFromDmidecode_002)
{
    QMap<QString, QString> mapDmidecode;
    setDmidecodeData(mapDmidecode);

    m_deviceCpu->setInfoFromDmidecode(mapDmidecode);
    EXPECT_STREQ("Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz", m_deviceCpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("Intel(R) Corporation", m_deviceCpu->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("8300 MHz", m_deviceCpu->m_Frequency.toStdString().c_str());
    EXPECT_STREQ("Core i3", m_deviceCpu->m_Familly.toStdString().c_str());
}

TEST_F(UT_DeviceCpu, UT_DeviceCpu_getTrNumber)
{
    m_deviceCpu->getTrNumber();
    EXPECT_EQ(67, m_deviceCpu->m_trNumber.size());
}

TEST_F(UT_DeviceCpu, UT_DeviceCpu_setCurFreq)
{
    m_deviceCpu->setCurFreq("1800 MHz");
    EXPECT_STREQ("1800 MHz", m_deviceCpu->m_CurFrequency.toStdString().c_str());
}

TEST_F(UT_DeviceCpu, UT_DeviceCpu_available)
{
    EXPECT_TRUE(m_deviceCpu->available());
}
