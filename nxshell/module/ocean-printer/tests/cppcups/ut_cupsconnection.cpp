// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QDebug>

#include "cupsconnection.h"

class ut_Connection : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    Connection m_cupsConnection;
};

void ut_Connection::SetUp()
{
    /*编译打包环境可能存在cups服务异常的情况*/
    m_cupsConnection.http = NULL;
}

void ut_Connection::TearDown()
{

}

TEST_F(ut_Connection, getPrinters)
{
    if (m_cupsConnection.http)
        ASSERT_NO_THROW(m_cupsConnection.getPrinters());
    else
        qInfo() << "could not connect to cups server";
}

TEST_F(ut_Connection, getPPDs2)
{
    if (m_cupsConnection.http) {
        auto allPPDS = m_cupsConnection.getPPDs2(0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, -1, nullptr, nullptr, nullptr);
        ASSERT_TRUE(allPPDS.size() > 0);
    } else {
        qInfo() << "could not connect to cups server";
    }

}

TEST_F(ut_Connection, adminGetServerSettings)
{
    if (m_cupsConnection.http) {
        /*某些环境可能没有设置，所以不能根据结果进行测试*/
        ASSERT_NO_THROW(m_cupsConnection.adminGetServerSettings());
    } else {
        qInfo() << "could not connect to cups server";
    }
}
