// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#define private public
#include "mainwindow.h"
#undef private

#include <QTest>
#include <QLabel>

class TstMainWindow : public testing::Test
{
public:
    void SetUp() override
    {
        window = new MainWindow();
    }

    void TearDown() override
    {
        delete window;
        window = nullptr;
    }

public:
    MainWindow *window = nullptr;
};

TEST_F(TstMainWindow, coverage_Test)
{
    MainWindow *w = new MainWindow;

    w->Show();
    QTest::qWait(AnimationTime * 3 / 2 + 10);

    w->Hide();
    QTest::qWait(AnimationTime * 3 / 2 + 10);

    w->geometryChanged();
    QTest::qWait(1);

    delete w;
    w = nullptr;
}

TEST_F(TstMainWindow, animation_Test)
{
    window->showAni();
    QTest::qWait(AnimationTime * 3 / 2 + 10);
    ASSERT_TRUE(window->isVisible());

    window->hideAni();
    QTest::qWait(AnimationTime * 3 / 2 + 10);
    ASSERT_FALSE(window->isVisible());

    bool visible = window->isVisible();
    window->Toggle();
    QTest::qWait(AnimationTime * 3 / 2 + 10);
    ASSERT_TRUE(!visible);
}
