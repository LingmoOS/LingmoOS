// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "kill_process_confirm_dialog.h"
#include "model/cpu_info_model.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <QMessageBox>

/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/

class UT_KillProcessConfirmDialog : public ::testing::Test
{
public:
    UT_KillProcessConfirmDialog() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new KillProcessConfirmDialog();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    KillProcessConfirmDialog *m_tester;
};

TEST_F(UT_KillProcessConfirmDialog, initTest)
{

}

TEST_F(UT_KillProcessConfirmDialog, test_result_01)
{
    m_tester->m_result = false;
    m_tester->result();
    EXPECT_FALSE(m_tester->m_result);
}

TEST_F(UT_KillProcessConfirmDialog, test_setResult_01)
{
    m_tester->setResult(true);

    EXPECT_TRUE(m_tester->m_result);
}

TEST_F(UT_KillProcessConfirmDialog, test_onButtonClicked_01)
{
    m_tester->onButtonClicked(0, "1");

    EXPECT_EQ(m_tester->m_result, QMessageBox::Cancel);
}

TEST_F(UT_KillProcessConfirmDialog, test_onButtonClicked_02)
{
    m_tester->onButtonClicked(1, "1");

    EXPECT_EQ(m_tester->m_result, QMessageBox::Ok);
}






