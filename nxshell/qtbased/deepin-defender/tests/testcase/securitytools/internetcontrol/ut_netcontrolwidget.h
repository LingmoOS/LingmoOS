// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "gtest/gtest.h"
#include <QObject>

class NetControlWidgetTest : public QObject
    , public testing::Test

{
    Q_OBJECT
public:
    NetControlWidgetTest();

protected:
    virtual void SetUp();
    virtual void TearDown();
};
