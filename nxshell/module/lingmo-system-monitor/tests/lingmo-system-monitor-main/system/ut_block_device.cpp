// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/block_device.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

//qt
#include <QString>
#include <QProcess>
#include <QFile>
#include <QIODevice>
#include <QTextStream>

using namespace core::system;

/***************************************STUB begin*********************************************/

void stub_ReadDeviceInfo()
{
    core::system::BlockDevice blockDevice;
}

bool stub_modPath_readDeviceSize(void*, QFile::OpenMode)
{
    return false;
}

bool stub_contains(const QStringRef &s, Qt::CaseSensitivity cs)
{
    return true;
}

/***************************************STUB end**********************************************/

class UT_BlockDevice: public ::testing::Test
{
public:
    UT_BlockDevice() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new BlockDevice();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    BlockDevice *m_tester;
};

TEST_F(UT_BlockDevice, initTest)
{
}

TEST_F(UT_BlockDevice, test_copy)
{
    BlockDevice* rhs = m_tester;
    m_tester->operator= (*rhs);
}

TEST_F(UT_BlockDevice, test_setDeviceName)
{
    QString strFileName = "abc";
    m_tester->setDeviceName(strFileName.toLocal8Bit());
    QString retName = QString::fromLatin1(m_tester->deviceName());
//    qInfo()<<"retName:"<<retName;
    EXPECT_EQ("abc", retName);
}

TEST_F(UT_BlockDevice, test_readDeviceInfo)
{
    m_tester->readDeviceInfo();
    EXPECT_TRUE((m_tester->capacity() > 0) || (m_tester->d->_time_Sec>0));
}

//TEST_F(UT_BlockDevice, test_readDeviceInfo_02)
//{
//    Stub stub;
//    stub.set((bool(QString::*)(QStringRef &))ADDR(QString, contains), stub_contains);
//    m_tester->readDeviceInfo();
//    EXPECT_TRUE((m_tester->capacity() > 0) || (m_tester->d->_time_Sec>0));
//}

TEST_F(UT_BlockDevice, test_readDeviceModel)
{
    core::system::BlockDevice blockDevice;
    blockDevice.readDeviceModel();
    if (blockDevice.model().isEmpty())
        qInfo("isEmpty");
//    EXPECT_FALSE(blockDevice.model().isEmpty());
}

TEST_F(UT_BlockDevice, test_readDeviceSize_01)
{
    EXPECT_EQ(m_tester->readDeviceSize(m_tester->d->name), 0);
}

TEST_F(UT_BlockDevice, test_readDeviceSize_02)
{
    typedef bool (*fptr)(QFile*, QIODevice::OpenMode);
    fptr A_foo = (fptr)((bool(QFile::*)(QIODevice::OpenMode))&QFile::open);

    Stub stub;
    stub.set(A_foo, stub_modPath_readDeviceSize);
    m_tester->readDeviceSize("101");
}

TEST_F(UT_BlockDevice, test_calcDiskIoStates)
{
    QFile file(PROC_PATH_DISK);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    QList<QStringList> strList;
    QTextStream in(&file);

    QString line;
    do{
        line = in.readLine();
        if (!line.contains("loop")){
            strList << line.split(" ", QString::SkipEmptyParts);
        }
    }while (!line.isNull());
    file.close();

    if (strList.size()){
        QStringList deviceInfo = strList[0];
        if (deviceInfo.size() > 16 && deviceInfo[2] == m_tester->d->name) {
            m_tester->calcDiskIoStates(deviceInfo);
            EXPECT_TRUE(m_tester->readSpeed() != 0);
        }
     }
}

TEST_F(UT_BlockDevice, test_deviceName)
{
    m_tester->deviceName();
}

TEST_F(UT_BlockDevice, test_model)
{
    m_tester->model();
}

TEST_F(UT_BlockDevice, test_capacity)
{
    m_tester->capacity();
}

TEST_F(UT_BlockDevice, test_blocksRead)
{
    m_tester->blocksRead();
}

TEST_F(UT_BlockDevice, test_bytesRead)
{
    m_tester->bytesRead();
}

TEST_F(UT_BlockDevice, test_sectorsReadPerSecond)
{
    m_tester->sectorsReadPerSecond();
}

TEST_F(UT_BlockDevice, test_readRequestIssuedPerSecond)
{
    m_tester->readRequestIssuedPerSecond();
}

TEST_F(UT_BlockDevice, test_readRequestMergedPerSecond)
{
    m_tester->readDeviceInfo();
    m_tester->readRequestMergedPerSecond();
}

TEST_F(UT_BlockDevice, test_readRequestMergedPercent)
{
    m_tester->readDeviceInfo();
    m_tester->readRequestMergedPercent();
}

TEST_F(UT_BlockDevice, test_percentUtilization)
{
    m_tester->readDeviceInfo();
    m_tester->percentUtilization();
}

TEST_F(UT_BlockDevice, test_transferPerSecond)
{
    m_tester->readDeviceInfo();
    m_tester->transferPerSecond();
}

TEST_F(UT_BlockDevice, test_blocksWritten)
{
    m_tester->readDeviceInfo();
    m_tester->blocksWritten();
}

TEST_F(UT_BlockDevice, test_bytesWritten)
{
    m_tester->readDeviceInfo();
    m_tester->bytesWritten();
}

TEST_F(UT_BlockDevice, test_sectorsWrittenPerSecond)
{
    m_tester->readDeviceInfo();
    m_tester->sectorsWrittenPerSecond();
}

TEST_F(UT_BlockDevice, test_writeRequestIssuedPerSecond)
{
    m_tester->readDeviceInfo();
    m_tester->writeRequestIssuedPerSecond();
}

TEST_F(UT_BlockDevice, test_writeRequestMergedPerSecond)
{
    m_tester->readDeviceInfo();
    m_tester->writeRequestMergedPerSecond();
}
TEST_F(UT_BlockDevice, test_writeRequestMergedPercent)
{
    m_tester->readDeviceInfo();
    m_tester->writeRequestMergedPercent();
}

TEST_F(UT_BlockDevice, test_readIssuer)
{
    m_tester->readDeviceInfo();
    m_tester->readIssuer();
}

TEST_F(UT_BlockDevice, test_writeComplete)
{
    m_tester->readDeviceInfo();
    m_tester->writeComplete();
}
TEST_F(UT_BlockDevice, test_readMerged)
{
    m_tester->readDeviceInfo();
    m_tester->readMerged();
}

TEST_F(UT_BlockDevice, test_writeMerged)
{
    m_tester->readDeviceInfo();
    m_tester->writeMerged();
}

TEST_F(UT_BlockDevice, test_readSpeed)
{
    m_tester->readDeviceInfo();
    m_tester->readSpeed();
}
TEST_F(UT_BlockDevice, test_writeSpeed)
{
    m_tester->readDeviceInfo();
    m_tester->writeSpeed();
}


