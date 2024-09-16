// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mimesappsmanager.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/

/*******************************单元测试************************************/
// 测试MimeAppsWorker
class UT_MimeAppsWorker : public ::testing::Test
{
public:
    UT_MimeAppsWorker(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new MimeAppsWorker;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    MimeAppsWorker *m_tester;
};

TEST_F(UT_MimeAppsWorker, initTest)
{

}

TEST_F(UT_MimeAppsWorker, test_handleDirectoryChanged)
{
    m_tester->handleDirectoryChanged("");
}

TEST_F(UT_MimeAppsWorker, test_updateCache)
{
    m_tester->updateCache();
}

TEST_F(UT_MimeAppsWorker, test_writeData)
{
    m_tester->writeData("", "");
}

TEST_F(UT_MimeAppsWorker, test_readData)
{
    QString strPath = _SOURCEDIR;
    strPath += "/src/desktopdeepin-compressor.desktop";
    m_tester->readData(strPath);
}


// 测试MimesAppsManager
class UT_MimesAppsManager : public ::testing::Test
{
public:
    UT_MimesAppsManager(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new MimesAppsManager;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    MimesAppsManager *m_tester;
};

TEST_F(UT_MimesAppsManager, initTest)
{

}

TEST_F(UT_MimesAppsManager, test_getMimeType)
{
    EXPECT_EQ(m_tester->getMimeType("1.zip").name(), "application/zip");
}

TEST_F(UT_MimesAppsManager, test_getMimeTypeByFileName)
{
    EXPECT_EQ(m_tester->getMimeTypeByFileName("1.zip"), "application/zip");
}

QString getDefaultAppByMimeType_stub(const QMimeType &)
{
    return "";
}

TEST_F(UT_MimesAppsManager, test_getDefaultAppByFileName)
{
    m_tester->getDefaultAppByFileName("1.zip");
}

TEST_F(UT_MimesAppsManager, test_getDefaultAppByMimeType)
{
    EXPECT_EQ(m_tester->getDefaultAppByMimeType("1.zip"), "");
}

TEST_F(UT_MimesAppsManager, test_getDefaultAppByMimeType1)
{
    EXPECT_EQ(m_tester->getDefaultAppByMimeType("1.zip").isEmpty(), true);
}

TEST_F(UT_MimesAppsManager, test_getDefaultAppDisplayNameByMimeType)
{
    EXPECT_EQ(m_tester->getDefaultAppByMimeType("1.zip"), "");
}

TEST_F(UT_MimesAppsManager, test_getDefaultAppDisplayNameByGio)
{
    EXPECT_EQ(m_tester->getDefaultAppDisplayNameByGio("1.zip"), "");
}

TEST_F(UT_MimesAppsManager, test_getDefaultAppDesktopFileByMimeType_001)
{
    EXPECT_EQ(m_tester->getDefaultAppDesktopFileByMimeType("application/sss"), "");
}

TEST_F(UT_MimesAppsManager, test_getDefaultAppDesktopFileByMimeType_002)
{
    m_tester->getDefaultAppDesktopFileByMimeType("application/zip");
}

TEST_F(UT_MimesAppsManager, test_getRecommendedAppsByQio)
{
    m_tester->getRecommendedAppsByQio(m_tester->getMimeType("1.zip"));
}

TEST_F(UT_MimesAppsManager, test_getRecommendedAppsByGio)
{
    m_tester->getRecommendedAppsByGio("application/zip");
}

TEST_F(UT_MimesAppsManager, test_getApplicationsFolders)
{
    EXPECT_EQ(m_tester->getApplicationsFolders().isEmpty(), false);
}

TEST_F(UT_MimesAppsManager, test_getMimeAppsCacheFile)
{
    EXPECT_EQ(m_tester->getMimeAppsCacheFile().isEmpty(), false);
}

TEST_F(UT_MimesAppsManager, test_getMimeInfoCacheFilePath)
{
    EXPECT_EQ(m_tester->getMimeInfoCacheFilePath().isEmpty(), false);
}

TEST_F(UT_MimesAppsManager, test_getMimeInfoCacheFileRootPath)
{
    EXPECT_EQ(m_tester->getMimeInfoCacheFileRootPath().isEmpty(), false);
}

TEST_F(UT_MimesAppsManager, test_getDesktopFilesCacheFile)
{
    EXPECT_EQ(m_tester->getDesktopFilesCacheFile().isEmpty(), false);
}

TEST_F(UT_MimesAppsManager, test_getDesktopIconsCacheFile)
{
    EXPECT_EQ(m_tester->getDesktopIconsCacheFile().isEmpty(), false);
}

TEST_F(UT_MimesAppsManager, test_getDesktopFiles)
{
    EXPECT_EQ(m_tester->getDesktopFiles().isEmpty(), false);
}

TEST_F(UT_MimesAppsManager, test_getDDEMimeTypeFile)
{
    EXPECT_EQ(m_tester->getDDEMimeTypeFile().isEmpty(), false);
}

TEST_F(UT_MimesAppsManager, test_getDesktopObjs)
{
    EXPECT_EQ(m_tester->getDesktopObjs().isEmpty(), false);
}

TEST_F(UT_MimesAppsManager, test_initMimeTypeApps)
{
    m_tester->initMimeTypeApps();
}

TEST_F(UT_MimesAppsManager, test_loadDDEMimeTypes)
{
    m_tester->loadDDEMimeTypes();
}
