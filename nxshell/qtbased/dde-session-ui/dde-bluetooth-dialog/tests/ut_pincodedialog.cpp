// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "pincodedialog.h"
#undef private

#include <gtest/gtest.h>

class UT_PinCodeDialog : public testing::Test
{

};

TEST_F(UT_PinCodeDialog, coverageTest)
{
    PinCodeDialog dialog("123456", "/com/deepin", QString::number(QDateTime::currentMSecsSinceEpoch()));
    ASSERT_TRUE("123456" == dialog.pinCode());
    Q_EMIT dialog.buttonClicked(1, "");
    Q_EMIT dialog.closed();
    dialog.HandleBlutoothPower("{\"Powered\": false}");
}

