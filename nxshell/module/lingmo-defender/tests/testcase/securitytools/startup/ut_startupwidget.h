// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_STARTUPWIDGET_H
#define UT_STARTUPWIDGET_H

#include "gtest/gtest.h"
#include <QObject>

class StartupWidgetTest : public QObject
    , public testing::Test

{
    Q_OBJECT
public:
    StartupWidgetTest();

protected:
    virtual void SetUp();
    virtual void TearDown();
};

#endif // UT_STARTUPWIDGET_H