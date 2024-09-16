// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gui/searchconfig/switchwidget/switchwidget.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QPaintEvent>

using namespace testing;
using namespace GrandSearch;

TEST(SwitchWidgetTest, constructor_title)
{
    QString title("title");

    SwitchWidget *w = new SwitchWidget(title);

    EXPECT_TRUE(w);
    EXPECT_TRUE(w->m_leftWidget);
    EXPECT_TRUE(w->m_switchBtn);

    delete w;
}

TEST(SwitchWidgetTest, constructor_nullptr)
{
    SwitchWidget *w = new SwitchWidget;

    EXPECT_TRUE(w);
    EXPECT_TRUE(w->m_leftWidget);
    EXPECT_TRUE(w->m_switchBtn);

    delete w;
}

TEST(SwitchWidgetTest, setChecked)
{
    SwitchWidget w;

    w.setChecked(true);
    EXPECT_TRUE(w.m_switchBtn->isChecked());

    w.setChecked(false);
    EXPECT_FALSE(w.m_switchBtn->isChecked());
}

TEST(SwitchWidgetTest, checked)
{
    SwitchWidget w;

    w.m_switchBtn->setChecked(true);
    EXPECT_TRUE(w.checked());

    w.m_switchBtn->setChecked(false);
    EXPECT_FALSE(w.checked());
}

TEST(SwitchWidgetTest, setTitle)
{
    SwitchWidget w;

    QString title("testTitle");
    w.setTitle(title);

    QLabel *label = qobject_cast<QLabel *>(w.m_leftWidget);
    ASSERT_TRUE(label);

    EXPECT_EQ(label->text(), title);
}

TEST(SwitchWidgetTest, title)
{
    SwitchWidget w;

    QString title("testTitle");

    QLabel *label = qobject_cast<QLabel *>(w.m_leftWidget);
    ASSERT_TRUE(label);

    label->setText(title);
    EXPECT_EQ(w.title(), title);

    w.m_leftWidget->setParent(nullptr);
    delete w.m_leftWidget;
    w.m_leftWidget = nullptr;

    EXPECT_TRUE(w.title().isEmpty());
}

TEST(SwitchWidgetTest, setIcon)
{
    SwitchWidget w;

    stub_ext::StubExt stu;

    bool ut_call_setPixmap = false;
    SwitchWidget *switchWidget = new SwitchWidget;
    stu.set_lamda(&QLabel::setPixmap, [&]() {
        ut_call_setPixmap = true;
    });

    QIcon icon;
    QSize size(10, 10);

    w.setIcon(icon, size);
    EXPECT_TRUE(ut_call_setPixmap);
    delete switchWidget;
}

TEST(SwitchWidgetTest, setEnableBackground)
{
    SwitchWidget w;

    w.m_hasBack = true;
    w.setEnableBackground(true);
    EXPECT_TRUE(w.m_hasBack);

    w.m_hasBack = false;
    w.setEnableBackground(true);
    EXPECT_TRUE(w.m_hasBack);
}

TEST(SwitchWidgetTest, enableBackground)
{
    SwitchWidget w;

    w.m_hasBack = true;
    EXPECT_TRUE(w.enableBackground());

    w.m_hasBack = false;
    EXPECT_FALSE(w.enableBackground());
}

TEST(SwitchWidgetTest, paintEvent)
{
    SwitchWidget w;

    QPaintEvent event(QRect(QPoint(10, 10), QSize(20, 20)));

    w.m_hasBack = true;

    EXPECT_NO_FATAL_FAILURE(w.paintEvent(&event));
}
