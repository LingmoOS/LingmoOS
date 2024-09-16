// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "hseparator.h"
#undef private

#include <QTest>

#include <gtest/gtest.h>

class Tst_Hseparator : public testing::Test
{};

TEST_F(Tst_Hseparator, hSeparator_test)
{
    HSeparator seperator;
    QPaintEvent event(QRect(10, 10, 10, 10));
    QApplication::sendEvent(&seperator, &event);
}
