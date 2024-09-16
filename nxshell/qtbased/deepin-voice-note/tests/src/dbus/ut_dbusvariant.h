// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_DBUSVARIANT_H
#define UT_DBUSVARIANT_H

#include "gtest/gtest.h"
#include <QTest>
#include <QObject>

class UT_QDBusArgument : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    UT_QDBusArgument();
};

#endif // UT_DBUSVARIANT_H
