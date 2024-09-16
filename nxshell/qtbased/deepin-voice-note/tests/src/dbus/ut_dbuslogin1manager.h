// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_DBUSLOGIN1MANAGER_H
#define UT_DBUSLOGIN1MANAGER_H

#include "gtest/gtest.h"
#include <QTest>
#include <QObject>
class DBusLogin1Manager;

class UT_DBusLogin1Manager : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    UT_DBusLogin1Manager();
    virtual void SetUp() override;
    virtual void TearDown() override;
    DBusLogin1Manager *m_loginManager {nullptr};
};

#endif // UT_DBUSLOGIN1MANAGER_H
