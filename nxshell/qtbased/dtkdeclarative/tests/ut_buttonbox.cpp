// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "test_helper.hpp"

class ut_ButtonBox : public ::testing::Test
{
public:
};

TEST_F(ut_ButtonBox, properties)
{
    ControlHelper<> helper("qrc:/qml/ButtonBox.qml");
    ASSERT_TRUE(helper.object);
    QQmlListReference buttons(helper.object, "buttons");
    ASSERT_EQ(buttons.count(), 2);
}
