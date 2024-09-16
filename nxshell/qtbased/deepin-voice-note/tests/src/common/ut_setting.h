// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_SETTING_H
#define UT_SETTING_H

#include "gtest/gtest.h"
#include <QTest>
#include <QObject>

class UT_Setting : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    UT_Setting();
};

#endif // UT_SETTING_H
