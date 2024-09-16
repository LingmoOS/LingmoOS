// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "GetDriverPathWidget.h"
#include "ut_Head.h"
#include "stub.h"

#include <QDir>

#include <gtest/gtest.h>


class UT_GetDriverPathWidget : public UT_HEAD
{
public:
    void SetUp()
    {
        m_GetDriverPathWidget = new GetDriverPathWidget;
    }
    void TearDown()
    {
        delete m_GetDriverPathWidget;
    }

    GetDriverPathWidget *m_GetDriverPathWidget;
};

TEST_F(UT_GetDriverPathWidget, UT_GetDriverPathWidget_path)
{
    QString ret = m_GetDriverPathWidget->path();
    EXPECT_STREQ(QDir::homePath().toStdString().c_str(), ret.toStdString().c_str());
}

TEST_F(UT_GetDriverPathWidget, UT_GetDriverPathWidget_includeSubdir_001)
{
    m_GetDriverPathWidget->mp_includeCheckBox->setCheckState(Qt::Checked);
    EXPECT_TRUE(m_GetDriverPathWidget->includeSubdir());
}

TEST_F(UT_GetDriverPathWidget, UT_GetDriverPathWidget_includeSubdir_002)
{
    m_GetDriverPathWidget->mp_includeCheckBox->setCheckState(Qt::Unchecked);
    EXPECT_FALSE(m_GetDriverPathWidget->includeSubdir());
}

TEST_F(UT_GetDriverPathWidget, UT_GetDriverPathWidget_updateTipLabelText)
{
    m_GetDriverPathWidget->updateTipLabelText("/home");
    EXPECT_STREQ("/home", m_GetDriverPathWidget->mp_tipLabel->text().toStdString().c_str());
    EXPECT_STREQ("/home", m_GetDriverPathWidget->mp_tipLabel->toolTip().toStdString().c_str());
}

TEST_F(UT_GetDriverPathWidget, UT_GetDriverPathWidget_clearTipLabelText)
{
    m_GetDriverPathWidget->clearTipLabelText();
    EXPECT_STREQ("", m_GetDriverPathWidget->mp_tipLabel->text().toStdString().c_str());
    EXPECT_STREQ("", m_GetDriverPathWidget->mp_tipLabel->toolTip().toStdString().c_str());
}
