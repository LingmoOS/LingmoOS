// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DFileInfo>

#include <gtest/gtest.h>
#include <stubext.h>

#include "dfileinfo_p.h"
#include "dfilehelper.h"

DIO_USE_NAMESPACE
DCORE_USE_NAMESPACE

class TestDFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        m_fileinfo = new DFileInfo(testUrl);
    }
    void TearDown() override
    {
        delete m_fileinfo;
        m_stub.clear();
    }

    stub_ext::StubExt m_stub;
    QUrl testUrl { "/tmp" };
    DFileInfo *m_fileinfo { nullptr };
};

TEST_F(TestDFileInfo, operatorEqual)
{
    DFileInfo fileinfo1(*m_fileinfo);
    DFileInfo fileinfo2(*m_fileinfo);
    fileinfo1 = fileinfo2;
    EXPECT_EQ(fileinfo1.url(), fileinfo2.url());
}

TEST_F(TestDFileInfo, url)
{
    EXPECT_EQ(testUrl, m_fileinfo->url().value());
}

TEST_F(TestDFileInfo, queryattributes)
{
    m_fileinfo->d->queryAttributes = "test";
    EXPECT_EQ(m_fileinfo->queryAttributes(), "test");
}

TEST_F(TestDFileInfo, queryinfoflag)
{
    FileQueryInfoFlags flag = FileQueryInfoFlags::TypeNone;
    m_fileinfo->d->queryInfoFlags = flag;
    EXPECT_EQ(m_fileinfo->queryInfoFlag(), flag);
}

TEST_F(TestDFileInfo, exits)
{
    {
        m_stub.set_lamda(ADDR(DFileInfoPrivate, checkQuerier), []() {
            __DBG_STUB_INVOKE__
            return true;
        });
        m_stub.set_lamda(g_file_info_get_file_type, [](GFileInfo *) {
            __DBG_STUB_INVOKE__
            return G_FILE_TYPE_REGULAR;
        });
        EXPECT_TRUE(m_fileinfo->exists().value());
    }
    {
        m_stub.set_lamda(ADDR(DFileInfoPrivate, checkQuerier), []() {
            __DBG_STUB_INVOKE__
            return true;
        });
        m_stub.set_lamda(g_file_info_get_file_type, [](GFileInfo *) {
            __DBG_STUB_INVOKE__
            return G_FILE_TYPE_UNKNOWN;
        });
        EXPECT_FALSE(m_fileinfo->exists().value());
    }
}

TEST_F(TestDFileInfo, permission)
{
    {
        m_stub.set_lamda(ADDR(DFileInfoPrivate, checkQuerier), []() {
            __DBG_STUB_INVOKE__
            return false;
        });
        EXPECT_EQ(m_fileinfo->permissions(), Permissions(Permission::NoPermission));
    }
    {
        m_stub.set_lamda(ADDR(DFileInfoPrivate, checkQuerier), []() {
            __DBG_STUB_INVOKE__
            return true;
        });
        m_stub.set_lamda(ADDR(DFileHelper, attributeFromInfo), [](AttributeID, GFileInfo *, QUrl) {
            __DBG_STUB_INVOKE__
            return QVariant();
        });
        EXPECT_EQ(m_fileinfo->permissions(), Permissions(Permission::NoPermission));
    }
}

TEST_F(TestDFileInfo, lasterror)
{
    m_fileinfo->d->error.setErrorCode(IOErrorCode::FileNotFound);
    EXPECT_EQ(m_fileinfo->lastError().getErrorCode(), IOErrorCode::FileNotFound);
}
