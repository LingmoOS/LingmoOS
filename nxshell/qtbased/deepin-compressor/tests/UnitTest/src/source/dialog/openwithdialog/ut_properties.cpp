// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "properties.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>

#include <QCheckBox>
#include <QDir>
#include <QResizeEvent>
#include <QStandardPaths>

/*******************************函数打桩************************************/
/*******************************单元测试************************************/
// 测试Properties
class UT_Properties : public ::testing::Test
{
public:
    UT_Properties(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QString strPath = _SOURCEDIR;
        strPath += "/src/desktopdeepin-compressor.desktop";
        m_tester = new Properties(strPath, "Desktop Entry");
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    Properties *m_tester;
};

TEST_F(UT_Properties, initTest)
{

}

TEST_F(UT_Properties, test_Properties)
{
    Properties temp(*m_tester);
}

TEST_F(UT_Properties, test_getKeys)
{
    EXPECT_EQ(m_tester->getKeys().isEmpty(), true);
}
