// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QDir>
#include "filesystem/dpathbuf.h"

DCORE_USE_NAMESPACE


class ut_DPathBuf : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    DPathBuf *pathBuf = nullptr;

};

void ut_DPathBuf::SetUp()
{
    pathBuf = new DPathBuf("/tmp/etc");
    QDir dir("/tmp/etc/");
    if (!dir.exists())
        dir.mkdir("/tmp/etc/");
}

void ut_DPathBuf::TearDown()
{
    if (pathBuf) {
        delete pathBuf;
        pathBuf = nullptr;
    }
    QDir dir("/tmp/etc/");
    if (dir.exists())
        dir.remove("/tmp/etc/");
}

TEST_F(ut_DPathBuf, testDPathBufOperatorSlashQString)
{
    *pathBuf = *pathBuf / QString("test");
    auto str = pathBuf->toString();
    ASSERT_TRUE(str == "/tmp/etc/test");
}

TEST_F(ut_DPathBuf, testDPathBufOperatorSlashEqualQString)
{
    *pathBuf /= QString("test");
    auto str = pathBuf->toString();
    ASSERT_TRUE(str == "/tmp/etc/test");
}

TEST_F(ut_DPathBuf, testDPathBufOperatorSlashChar)
{
    *pathBuf = *pathBuf / "test";
    auto str = pathBuf->toString();
    ASSERT_TRUE(str == "/tmp/etc/test");
}

TEST_F(ut_DPathBuf, testDPathBufOperatorSlashEqualChar)
{
    *pathBuf /= "test";
    auto str = pathBuf->toString();
    ASSERT_TRUE(str == "/tmp/etc/test");
}

TEST_F(ut_DPathBuf, testDPathBufJoin)
{
    *pathBuf = pathBuf->join(QString("test"));
    auto str = pathBuf->toString();
    ASSERT_TRUE(str == "/tmp/etc/test");
}

TEST_F(ut_DPathBuf, testToString)
{
    auto str = pathBuf->toString();
    ASSERT_TRUE(str == "/tmp/etc");
}
