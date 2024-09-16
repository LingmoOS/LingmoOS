// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "fullscreenbackground.h"
#include "mainwidget.h"
#include "updatecontent.h"
#undef private
#include "utils.h"

#include <QTest>

#include <gtest/gtest.h>

class UT_MainWidget : public testing::Test
{
public:
    void SetUp() override
    {
        mainWidget = new MainWidget();
    }

    void TearDown() override
    {
        delete mainWidget;
        mainWidget = nullptr;
    }

public:
    MainWidget *mainWidget = nullptr;
};

TEST_F(UT_MainWidget, coverageTest)
{
    const QString &w = mainWidget->m_blurImageInter->Get(mainWidget->m_wallpaper);
    Q_EMIT mainWidget->m_blurImageInter->BlurDone(mainWidget->m_wallpaper, w, true);

    QTest::keyClick(mainWidget,  Qt::Key_Enter);
    QTest::keyClick(mainWidget,  Qt::Key_Escape);
}
