// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include <iostream>
#include "gtest/gtest.h"

#include "unistd.h"

#include "../../service/diskoperation/partedcore.h"

using namespace DiskManager;

class ut_partedcore : public ::testing::Test
{
protected:
    ut_partedcore()
    {
    }

    virtual ~ut_partedcore()
    {
    }
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(ut_partedcore, test)
{
    PartedCore c;
    int i = c.test();

    EXPECT_EQ(i, 1);
}

TEST_F(ut_partedcore, create)
{
    PartitionVec infovec;

    PartitionInfo ut_info;

    ut_info.m_sectorStart = 2048;
    ut_info.m_sectorEnd = 2099200;
    ut_info.m_fileSystemType = 11;
    ut_info.m_fileSystemLabel = " ";
    ut_info.m_alignment = 1;
    ut_info.m_sectorSize = 512;
    ut_info.m_insideExtended = false;
    ut_info.m_busy = false;
    ut_info.m_fileSystemReadOnly = false;
    ut_info.m_devicePath = "/dev/sdc";
    ut_info.m_type = 0;

    infovec.push_back(ut_info);

    PartedCore c;

    bool flag = c.create(infovec);

    EXPECT_TRUE(flag);
}

TEST_F(ut_partedcore, format)
{
    PartitionInfo ut_info;

    ut_info.m_sectorStart = 2048;
    ut_info.m_sectorEnd = 2099200;
    ut_info.m_fileSystemType = 11;
    ut_info.m_fileSystemLabel = " ";
    ut_info.m_alignment = 1;
    ut_info.m_sectorSize = 512;
    ut_info.m_insideExtended = false;
    ut_info.m_busy = false;
    ut_info.m_fileSystemReadOnly = false;
    ut_info.m_devicePath = "/dev/sdc";
    ut_info.m_type = 0;
    ut_info.m_path = "/dev/sdc1";


    PartedCore c;
    c.setCurSelect(ut_info);

    bool flag = c.format("ext4", " ");

    EXPECT_TRUE(flag);

}
TEST_F(ut_partedcore, unmount)
{
    PartitionInfo ut_info;

    ut_info.m_sectorStart = 2048;
    ut_info.m_sectorEnd = 2099200;
    ut_info.m_fileSystemType = 11;
    ut_info.m_fileSystemLabel = " ";
    ut_info.m_alignment = 1;
    ut_info.m_sectorSize = 512;
    ut_info.m_insideExtended = false;
    ut_info.m_busy = true;
    ut_info.m_fileSystemReadOnly = false;
    ut_info.m_devicePath = "/dev/sdb";
    ut_info.m_type = 0;
    ut_info.m_path = "/dev/sdb1";
    ut_info.m_mountPoints.push_back("/mnt");


    PartedCore c;
    c.setCurSelect(ut_info);

    bool flag = c.unmount();

    EXPECT_TRUE(flag);

}

TEST_F(ut_partedcore, deletePartition)
{
    PartitionInfo ut_info;

    ut_info.m_sectorStart = 2048;
    ut_info.m_sectorEnd = 2099200;
    ut_info.m_fileSystemType = 11;
    ut_info.m_fileSystemLabel = " ";
    ut_info.m_alignment = 1;
    ut_info.m_sectorSize = 512;
    ut_info.m_insideExtended = false;
    ut_info.m_busy = false;
    ut_info.m_fileSystemReadOnly = false;
    ut_info.m_devicePath = "/dev/sdc";
    ut_info.m_type = 0;
    ut_info.m_path = "/dev/sdc1";
    ut_info.m_mountPoints.push_back("/mnt");


    PartedCore c;
    c.setCurSelect(ut_info);

    bool flag = c.deletePartition();

    EXPECT_TRUE(flag);

}

TEST_F(ut_partedcore, detectionPartitionTableError)
{

    PartedCore c;

    bool flag = c.detectionPartitionTableError("/dev/sdc");

    EXPECT_FALSE(flag);

}

TEST_F(ut_partedcore, updateUsb)
{

    PartedCore c;

    bool flag = c.updateUsb();

    EXPECT_TRUE(flag);

}

TEST_F(ut_partedcore, updateUsbRemove)
{

    PartedCore c;

    bool flag = c.updateUsbRemove();

    EXPECT_TRUE(flag);

}
