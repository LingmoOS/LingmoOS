// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notification/actionbutton.h"

#include <gtest/gtest.h>

class UT_ActionButton : public testing::Test
{
public:
    void SetUp() override
    {
        obj = new ActionButton();
        actionList << "_open" << "查看"
                   << "_close" << "忽略";
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
    }

public:
    ActionButton *obj = nullptr;
    QStringList actionList;
};

TEST_F(UT_ActionButton, coverageTest)
{

}

TEST_F(UT_ActionButton, addButtonsTest)
{
    obj->addButtons(actionList);
    int count = obj->buttonList().size();

    EXPECT_FALSE(obj->isEmpty());
    EXPECT_EQ(actionList.size() / 2, count);
}

TEST_F(UT_ActionButton, clearButtonsTest)
{
    obj->clear();
    EXPECT_TRUE(obj->isEmpty());
}

TEST_F(UT_ActionButton, buttonsSizeTest)
{
    EXPECT_TRUE(QSize(70, 60) == obj->buttonSize());

    QSize size(100, 90);
    obj->setButtonSize(size);
    EXPECT_TRUE(size == obj->buttonSize());
}

TEST_F(UT_ActionButton, contentSizeTest)
{
    QSize size = obj->contentSize("查看", false);
    EXPECT_TRUE(QSize(70, 40) == size);
}
