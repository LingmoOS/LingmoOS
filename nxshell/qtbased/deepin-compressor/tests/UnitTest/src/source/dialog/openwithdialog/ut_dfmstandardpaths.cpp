// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmstandardpaths.h"
#include "durl.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/

/*******************************单元测试************************************/
// 测试CompressParameter
class UT_DFMStandardPaths : public ::testing::Test
{
public:
    UT_DFMStandardPaths(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new DFMStandardPaths;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    DFMStandardPaths *m_tester;
};

TEST_F(UT_DFMStandardPaths, initTest)
{

}

TEST_F(UT_DFMStandardPaths, test_location)
{
    EXPECT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::TrashPath), QDir::homePath() + "/.local/share/Trash");
    EXPECT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::TrashFilesPath), QDir::homePath() + "/.local/share/Trash/files");
    EXPECT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::TrashInfosPath), QDir::homePath() + "/.local/share/Trash/info");
#ifdef APPSHAREDIR
#endif
#ifdef PLUGINDIR
#endif
#ifdef QMAKE_TARGET
#endif
    EXPECT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::ThumbnailPath), QDir::homePath() + "/.cache/thumbnails");
    EXPECT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::ThumbnailFailPath), m_tester->location(DFMStandardPaths::StandardLocation::ThumbnailPath) + "/fail");
    EXPECT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::ThumbnailLargePath), m_tester->location(DFMStandardPaths::StandardLocation::ThumbnailPath) + "/large");
    EXPECT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::ThumbnailNormalPath), m_tester->location(DFMStandardPaths::StandardLocation::ThumbnailPath) + "/normal");
    EXPECT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::ThumbnailSmallPath), m_tester->location(DFMStandardPaths::StandardLocation::ThumbnailPath) + "/small");
#ifdef APPSHAREDIR
#endif
    EXPECT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::RecentPath), "recent:///");
    EXPECT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::HomePath), QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    EXPECT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::DesktopPath), QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first());
    EXPECT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::VideosPath), QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).first());
    EXPECT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::MusicPath), QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first());
    EXPECT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::PicturesPath), QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first());
    EXPECT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::DocumentsPath), QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first());
    EXPECT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::DownloadsPath), QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).first());
    EXPECT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::CachePath), m_tester->getCachePath());
    EXPECT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::DiskPath), QDir::rootPath());
#ifdef NETWORK_ROOT
    EXPECT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::NetworkRootPath), NETWORK_ROOT);
#endif
#ifdef USERSHARE_ROOT
    EXPECT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::UserShareRootPath), USERSHARE_ROOT);
#endif
#ifdef COMPUTER_ROOT
    EXPECT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::ComputerRootPath), COMPUTER_ROOT);
#endif
    EXPECT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::Root), "/");
}

TEST_F(UT_DFMStandardPaths, test_getCachePath)
{
    EXPECT_EQ(m_tester->getCachePath().isEmpty(), false);
}

