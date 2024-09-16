// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_SAFETY_PROTECTION_WIDGET_H
#define UT_SAFETY_PROTECTION_WIDGET_H

#include "gtest/gtest.h"
#include <QObject>

class SafetyProtectionWidgetTest : public QObject
    , public testing::Test

{
    Q_OBJECT
public:
    SafetyProtectionWidgetTest();

protected:
    virtual void SetUp();
    virtual void TearDown();
};

#endif // UT_SAFETY_PROTECTION_WIDGET_H
