// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "gui/searchconfig/checkboxwidget/checkboxwidget.h"
#include "gui/searchconfig/checkboxwidget/checkboxitem.h"
#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;
GRANDSEARCH_USE_NAMESPACE

TEST(CheckBoxWidgetTest, addCheckBox)
{
    CheckBoxWidget w;
    EXPECT_EQ(0, w.addCheckBox("1"));
    EXPECT_EQ(1, w.addCheckBox("2"));
    EXPECT_EQ(2, w.addCheckBox("3"));
}

TEST(CheckBoxWidgetTest, setCheckBoxChecked)
{
    CheckBoxWidget w;
    w.addCheckBox("test");
    EXPECT_TRUE(w.setCheckBoxChecked("test", true));
}

TEST(CheckBoxWidgetTest, onCheckBoxChecked)
{
    CheckBoxWidget w;
    w.addCheckBox("test");

    stub_ext::StubExt st;
    st.set_lamda(&QObject::sender, [&]() {
        return w.m_checkBoxList.first();
    });

    EXPECT_NO_FATAL_FAILURE(w.onCheckBoxChecked(true));
}
