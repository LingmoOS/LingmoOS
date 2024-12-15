// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "settings.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

//qt
#include <DApplication>

/***************************************STUB begin*********************************************/
/***************************************STUB end**********************************************/


class UT_Settings: public ::testing::Test
{
public:
    UT_Settings() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new Settings();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    Settings *m_tester;
};

TEST_F(UT_Settings, initTest)
{
}

TEST_F(UT_Settings, test_instance)
{
    EXPECT_TRUE(m_tester->instance() != nullptr);
}

TEST_F(UT_Settings, test_flush)
{
    m_tester->flush();
}

TEST_F(UT_Settings, test_getOption)
{
    m_tester->getOption(kSettingKeyProcessTabIndex);
}

TEST_F(UT_Settings, test_setOption)
{
//    DMainWindow::closeEvent(event);
    m_tester->setOption(kSettingKeyProcessAttributeDialogWidth, 100);
}
