// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mimetypedisplaymanager.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/


/*******************************单元测试************************************/
// 测试MimeTypeDisplayManager
class UT_MimeTypeDisplayManager : public ::testing::Test
{
public:
    UT_MimeTypeDisplayManager(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new MimeTypeDisplayManager();
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    MimeTypeDisplayManager *m_tester;
};

TEST_F(UT_MimeTypeDisplayManager, initTest)
{

}

TEST_F(UT_MimeTypeDisplayManager, test_displayName)
{
    EXPECT_EQ(m_tester->displayName("inode/directory"), "Directory");
}

TEST_F(UT_MimeTypeDisplayManager, test_displayNameToEnum)
{
    EXPECT_EQ(m_tester->displayNameToEnum("application/x-desktop"), FileType::DesktopApplication);
    EXPECT_EQ(m_tester->displayNameToEnum("inode/directory"), FileType::Directory);
    EXPECT_EQ(m_tester->displayNameToEnum("application/x-executable"), FileType::Executable);
    EXPECT_EQ(m_tester->displayNameToEnum("video/"), FileType::Videos);
    EXPECT_EQ(m_tester->displayNameToEnum("audio/"), FileType::Audios);
    EXPECT_EQ(m_tester->displayNameToEnum("image/"), FileType::Images);
    EXPECT_EQ(m_tester->displayNameToEnum("text/"), FileType::Documents);
    if (MimeTypeDisplayManager::ArchiveMimeTypes.count() > 0) {
        EXPECT_EQ(m_tester->displayNameToEnum(MimeTypeDisplayManager::ArchiveMimeTypes[0]), FileType::Archives);
    }
    if (MimeTypeDisplayManager::BackupMimeTypes.count() > 0) {
        EXPECT_EQ(m_tester->displayNameToEnum(MimeTypeDisplayManager::BackupMimeTypes[0]), FileType::Backups);
    }
    EXPECT_EQ(m_tester->displayNameToEnum("sadsadasd"), FileType::Unknown);
}

TEST_F(UT_MimeTypeDisplayManager, test_defaultIcon)
{
    EXPECT_EQ(m_tester->defaultIcon("inode/directory"), "folder");
}

TEST_F(UT_MimeTypeDisplayManager, test_displayNames)
{
    EXPECT_EQ(m_tester->displayNames(), m_tester->m_displayNames);
}

TEST_F(UT_MimeTypeDisplayManager, test_supportArchiveMimetypes)
{
    EXPECT_EQ(m_tester->supportArchiveMimetypes() == MimeTypeDisplayManager::ArchiveMimeTypes, true);
}

TEST_F(UT_MimeTypeDisplayManager, test_supportVideoMimeTypes)
{
    EXPECT_EQ(m_tester->supportVideoMimeTypes() == MimeTypeDisplayManager::VideoMimeTypes, true);
}


