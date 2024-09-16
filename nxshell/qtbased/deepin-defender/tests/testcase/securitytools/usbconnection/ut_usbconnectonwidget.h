// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "gtest/gtest.h"
#include <QObject>

class UsbConnectionWidgetTest : public QObject
    , public testing::Test

{
    Q_OBJECT
public:
    UsbConnectionWidgetTest();

protected:
    virtual void SetUp();
    virtual void TearDown();
};
