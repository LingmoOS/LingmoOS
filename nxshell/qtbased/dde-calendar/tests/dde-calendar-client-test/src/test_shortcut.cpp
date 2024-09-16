// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_shortcut.h"

#include "shortcut.h"
#include "gtest/gtest.h"

test_Shortcut::test_Shortcut(QObject *parent)
    : QObject(parent)
{
}

TEST(shorcutTest, shorcut)
{
    Shortcut shortcut;
    QString str = shortcut.toStr();
}
