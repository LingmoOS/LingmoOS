// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "successpage.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>
#include <QTest>

class UT_SuccessPage : public ::testing::Test
{
public:
    UT_SuccessPage(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new SuccessPage;
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    SuccessPage *m_tester;
};

TEST_F(UT_SuccessPage, initTest)
{

}

TEST_F(UT_SuccessPage, test_setCompressFullPath)
{
    m_tester->setCompressFullPath("111");
    EXPECT_EQ(m_tester->m_strFullPath, "111");
}

TEST_F(UT_SuccessPage, test_setSuccessDes)
{
    m_tester->setSuccessDes("222");
    EXPECT_EQ(m_tester->m_pSuccessLbl->text(), "222");
}

TEST_F(UT_SuccessPage, test_setSuccessType)
{
    m_tester->setSuccessType(SI_Compress);
    EXPECT_EQ(m_tester->m_successInfoType, SI_Compress);
}

TEST_F(UT_SuccessPage, test_getSuccessType)
{
    m_tester->setSuccessType(SI_Compress);
    EXPECT_EQ(m_tester->getSuccessType(), SI_Compress);
}
