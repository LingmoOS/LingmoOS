// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "archiveformat.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>

#include <QTextCodec>
#include <QFileInfo>
/*******************************函数打桩************************************/

/*******************************函数打桩************************************/


// 测试ArchiveFormat
class UT_ArchiveFormat : public ::testing::Test
{
public:
    UT_ArchiveFormat() {}

public:
    virtual void SetUp()
    {
        QString strPath = _SOURCEDIR;
        strPath += "/3rdparty/libzipplugin/kerfuffle_libzip.json";
        CustomMimeType mimeType;
        mimeType.m_bUnKnown = true;
        mimeType.m_strTypeName = "application/zip";
        KPluginMetaData metadata(strPath);
        m_tester = ArchiveFormat::fromMetadata(mimeType, metadata);
    }

    virtual void TearDown()
    {

    }

protected:
    ArchiveFormat m_tester;
};

TEST_F(UT_ArchiveFormat, initTest)
{

}

TEST_F(UT_ArchiveFormat, test_encryptionType)
{
    m_tester.m_encryptionType = Unencrypted;
    EXPECT_EQ(m_tester.encryptionType(), Unencrypted);
}

TEST_F(UT_ArchiveFormat, test_minCompressionLevel)
{
    m_tester.m_minCompressionLevel = 1;
    EXPECT_EQ(m_tester.minCompressionLevel(), 1);
}

TEST_F(UT_ArchiveFormat, test_maxCompressionLevel)
{
    m_tester.m_maxCompressionLevel = 9;
    EXPECT_EQ(m_tester.maxCompressionLevel(), 9);
}

TEST_F(UT_ArchiveFormat, test_defaultCompressionLevel)
{
    m_tester.m_defaultCompressionLevel = 6;
    EXPECT_EQ(m_tester.defaultCompressionLevel(), 6);
}

TEST_F(UT_ArchiveFormat, test_supportsWriteComment)
{
    m_tester.m_supportsWriteComment = true;
    EXPECT_EQ(m_tester.supportsWriteComment(), true);
}

TEST_F(UT_ArchiveFormat, test_supportsTesting)
{
    m_tester.m_supportsTesting = true;
    EXPECT_EQ(m_tester.supportsTesting(), true);
}

TEST_F(UT_ArchiveFormat, test_supportsMultiVolume)
{
    m_tester.m_supportsMultiVolume = true;
    EXPECT_EQ(m_tester.supportsMultiVolume(), true);
}

TEST_F(UT_ArchiveFormat, test_compressionMethods)
{
    m_tester.m_compressionMethods = QVariantMap();
    EXPECT_EQ(m_tester.compressionMethods(), QVariantMap());
}

TEST_F(UT_ArchiveFormat, test_defaultCompressionMethod)
{
    m_tester.m_defaultCompressionMethod = "Default";
    EXPECT_EQ(m_tester.defaultCompressionMethod(), "Default");
}

TEST_F(UT_ArchiveFormat, test_encryptionMethods)
{
    m_tester.m_encryptionMethods = QStringList() << "AES256";
    EXPECT_EQ(m_tester.encryptionMethods(), QStringList() << "AES256");
}

TEST_F(UT_ArchiveFormat, test_defaultEncryptionMethod)
{
    m_tester.m_defaultEncryptionMethod = "AES256";
    EXPECT_EQ(m_tester.defaultEncryptionMethod(), "AES256");
}


