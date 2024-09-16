// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_IMAGEVIEWERDIALOG_H
#define UT_IMAGEVIEWERDIALOG_H

#include "gtest/gtest.h"
#include <QTest>
#include <QObject>

class UT_Imageviewerdialog : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    UT_Imageviewerdialog();
};

#endif // UT_IMAGEVIEWERDIALOG_H
