// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_UPGRADEDBUTIL_H
#define UT_UPGRADEDBUTIL_H

#include "gtest/gtest.h"
#include <QTest>
#include <QObject>

class UpgradeDbUtil;
class UT_UpgradeDbUtil : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    UT_UpgradeDbUtil();

    virtual void SetUp() override;
    virtual void TearDown() override;

protected:
    UpgradeDbUtil *m_upgradedbutil {nullptr};
};

#endif // UT_UPGRADEDBUTIL_H
