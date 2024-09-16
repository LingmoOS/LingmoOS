// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_UTILS_H
#define UT_UTILS_H

#include "gtest/gtest.h"
#include <QTest>
#include <QObject>

class Utils;

class UT_Utils : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    UT_Utils();

    virtual void SetUp() override;
    virtual void TearDown() override;

protected:
    Utils *m_utils {nullptr};
};

#endif // UT_UTILS_H
