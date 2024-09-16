// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ProgressDialog.h"

#include "stub.h"

#include <gtest/gtest.h>
#include <QTest>

class TestProgressDialog : public ::testing::Test
{
public:
    TestProgressDialog(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ProgressDialog();
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    ProgressDialog *m_tester;
};

TEST_F(TestProgressDialog, initTest)
{

}
