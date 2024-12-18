// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktopfile.h"
#include "properties.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/
// 对CompressView的addCompressFiles进行打桩
bool properties_contains_stub(const QString &key)
{
    if (key == "X-Lingmo-AppID" || key == "X-Lingmo-Vendor" || key == "NoDisplay" || key == "Hidden") {
        return true;
    }

    return false;
}

/*******************************单元测试************************************/
// 测试CompressParameter
class UT_DesktopFile : public ::testing::Test
{
public:
    UT_DesktopFile(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new DesktopFile;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    DesktopFile *m_tester;
};

TEST_F(UT_DesktopFile, initTest)
{

}

TEST_F(UT_DesktopFile, test_DesktopFile)
{
    Stub stub;
    QFileStub::stub_QFile_exists(stub, true);
    stub.set(ADDR(Properties, contains), properties_contains_stub);

    DesktopFile desktop1("1.desktop");

    EXPECT_EQ(desktop1.m_fileName, "1.desktop");
    EXPECT_EQ(desktop1.m_lingmoId, "");
    EXPECT_EQ(desktop1.m_lingmoVendor, "");
    EXPECT_EQ(desktop1.m_noDisplay, false);
    EXPECT_EQ(desktop1.m_hioceann, false);
    EXPECT_EQ(desktop1.m_localName, "");
    EXPECT_EQ(desktop1.m_genericName, "");
    EXPECT_EQ(desktop1.m_exec, "");
    EXPECT_EQ(desktop1.m_icon, "");
    EXPECT_EQ(desktop1.m_type, "");
    EXPECT_EQ(desktop1.m_categories.isEmpty(), true);
}

TEST_F(UT_DesktopFile, test_getFileName)
{
    EXPECT_EQ(m_tester->getFileName(), m_tester->m_fileName);
}

TEST_F(UT_DesktopFile, test_getPureFileName)
{
    QString strPath = _SOURCEDIR;
    strPath += "/src/desktop/lingmo-compressor.desktop";
    m_tester->m_fileName = strPath;
    EXPECT_EQ(m_tester->getPureFileName(), "lingmo-compressor");
}

TEST_F(UT_DesktopFile, test_getName)
{
    m_tester->m_name = "lingmo";
    EXPECT_EQ(m_tester->getName(), "lingmo");
}

TEST_F(UT_DesktopFile, test_getLocalName)
{
    m_tester->m_localName = "lingmo";
    EXPECT_EQ(m_tester->getLocalName(), "lingmo");
}

TEST_F(UT_DesktopFile, test_getDisplayName_001)
{
    m_tester->m_lingmoVendor = "lingmo";
    m_tester->m_genericName = "compressor";
    EXPECT_EQ(m_tester->getDisplayName(), "compressor");
}

TEST_F(UT_DesktopFile, test_getDisplayName_002)
{
    m_tester->m_lingmoVendor = "360";
    m_tester->m_localName = "test";
    EXPECT_EQ(m_tester->getDisplayName(), "test");
}

TEST_F(UT_DesktopFile, test_getExec)
{
    EXPECT_EQ(m_tester->getExec(), m_tester->m_exec);
}

TEST_F(UT_DesktopFile, test_getIcon)
{
    EXPECT_EQ(m_tester->getIcon(), m_tester->m_icon);
}

TEST_F(UT_DesktopFile, test_getType)
{
    EXPECT_EQ(m_tester->getType(), m_tester->m_type);
}

TEST_F(UT_DesktopFile, test_getLingmoId)
{
    EXPECT_EQ(m_tester->getLingmoId(), m_tester->m_lingmoId);
}

TEST_F(UT_DesktopFile, test_getLingmoVendor)
{
    EXPECT_EQ(m_tester->getLingmoVendor(), m_tester->m_lingmoVendor);
}

TEST_F(UT_DesktopFile, test_getNoShow)
{
    m_tester->m_noDisplay = true;
    EXPECT_EQ(m_tester->getNoShow(), true);
}

TEST_F(UT_DesktopFile, test_getCategories)
{
    EXPECT_EQ(m_tester->getCategories() == m_tester->m_categories, true);
}

TEST_F(UT_DesktopFile, test_getMimeType)
{
    EXPECT_EQ(m_tester->getMimeType() == m_tester->m_mimeType, true);
}
