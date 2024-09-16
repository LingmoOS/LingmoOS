// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_dsetting.h"

#include "../../src/dsettings.h"

Ut_DSetting::Ut_DSetting()
{

}

TEST_F(Ut_DSetting, getwidget)
{
    DSettingsAlt *m_dsetting = new DSettingsAlt;
    m_dsetting->m_settings->setValue("theme", "");
    m_dsetting->m_settings->setValue("mode", "");
    m_dsetting->m_settings->setValue("history", "");
    m_dsetting->m_settings->setValue("windowX", "");
    m_dsetting->m_settings->setValue("windowY", "");
    m_dsetting->m_settings->setValue("windowWidth", "");
    m_dsetting->m_settings->setValue("windowHeight", "");
    m_dsetting->initConfig();
    EXPECT_EQ(m_dsetting->getOption("windowHeight").toInt(), 0);
    EXPECT_EQ(m_dsetting->getOption("mode").toInt(), 0);
    EXPECT_EQ(m_dsetting->getOption("theme").toInt(), 0);
    DSettingsAlt::deleteInstance();
}
