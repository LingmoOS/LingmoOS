// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_MIDDLEVIEWDELEGATE_H
#define UT_MIDDLEVIEWDELEGATE_H
#include "gtest/gtest.h"

#include <QObject>

class UT_MiddleViewDelegate : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    explicit UT_MiddleViewDelegate(QObject *parent = nullptr);

signals:

public slots:
};

#endif // UT_MIDDLEVIEWDELEGATE_H
