// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_OLDDATALOADWORKS_H
#define UT_OLDDATALOADWORKS_H

#include "gtest/gtest.h"
#include <QObject>

class UT_OldDataLoadTask : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    UT_OldDataLoadTask();
};

#endif // UT_OLDDATALOADWORKS_H
