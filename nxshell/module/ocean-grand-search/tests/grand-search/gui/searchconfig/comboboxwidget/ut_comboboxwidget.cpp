// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gui/searchconfig/comboboxwidget/comboboxwidget.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QPaintEvent>

using namespace testing;
using namespace GrandSearch;

TEST(ComboboxWidgetTest, constructor_title)
{
    ComboboxWidget *comboboxWidget = new ComboboxWidget("test");
    EXPECT_TRUE(comboboxWidget);
    EXPECT_TRUE(comboboxWidget->m_mainLayout);
    EXPECT_TRUE(comboboxWidget->m_leftWidget);
    EXPECT_TRUE(comboboxWidget->m_comboBox);

    delete comboboxWidget;
}

TEST(ComboboxWidgetTest, constructor_parent)
{
    ComboboxWidget *comboboxWidget = new ComboboxWidget;

    EXPECT_TRUE(comboboxWidget);
    EXPECT_TRUE(comboboxWidget->m_mainLayout);
    EXPECT_TRUE(comboboxWidget->m_leftWidget);
    EXPECT_TRUE(comboboxWidget->m_comboBox);

    delete comboboxWidget;
}

TEST(ComboboxWidgetTest, setChecked)
{
    stub_ext::StubExt stu;

    bool result = false;
    stu.set_lamda(&Dtk::Widget::DComboBox::setCurrentText, [&](){
        result = true;
    });

    ComboboxWidget comboboxWidget;
    comboboxWidget.setChecked("test");
    EXPECT_TRUE(result);
}

TEST(ComboboxWidgetTest, checked)
{
    stub_ext::StubExt stu;

    stu.set_lamda(&Dtk::Widget::DComboBox::currentText, [](){
        return QString("a");
    });

    ComboboxWidget comboboxWidget;
    QString result = comboboxWidget.checked();

    EXPECT_EQ(result, QString("a"));
}

TEST(ComboboxWidgetTest, setTitle)
{
    ComboboxWidget c;

    c.setTitle("test");
    QLabel *label = qobject_cast<QLabel *>(c.m_leftWidget);
    ASSERT_TRUE(label);

    EXPECT_EQ(label->text(), "test");
    delete label;
}

TEST(ComboboxWidget, title)
{
    ComboboxWidget c;
    QString title("test");

    QLabel *label = qobject_cast<QLabel *>(c.m_leftWidget);
    ASSERT_TRUE(label);
    c.setTitle(title);
    QString result = c.title();
    EXPECT_EQ(result, title);

    c.m_leftWidget->setParent(nullptr);
    delete c.m_leftWidget;
    c.m_leftWidget = nullptr;

    EXPECT_TRUE(c.title().isEmpty());
}

TEST(ComboboxWidget, setEnableBackground)
{
    ComboboxWidget c;

    c.m_hasBack = true;
    c.setEnableBackground(true);
    EXPECT_TRUE(c.m_hasBack);

    c.m_hasBack = false;
    c.setEnableBackground(true);
    EXPECT_TRUE(c.m_hasBack);
}

TEST(ComboboxWidget, enableBackground)
{
    ComboboxWidget c;
    c.m_hasBack = true;
    bool result = c.enableBackground();
    EXPECT_TRUE(result);
}

TEST(ComboboxWidget, getComboBox)
{
    ComboboxWidget c;
    c.m_comboBox = new Dtk::Widget::DComboBox;
    Dtk::Widget::DComboBox *d = c.getComboBox();
    EXPECT_TRUE(d);

    delete d;
}

TEST(ComboboxWidget, paintEvent)
{
    ComboboxWidget w;

    QPaintEvent event(QRect(QPoint(10, 10), QSize(20, 20)));

    w.m_hasBack = true;

    EXPECT_NO_FATAL_FAILURE(w.paintEvent(&event));
}
