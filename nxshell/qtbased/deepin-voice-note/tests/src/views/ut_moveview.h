// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_MOVEVIEW_H
#define UT_MOVEVIEW_H

#include "gtest/gtest.h"
#include <QObject>

class UT_MoveView : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    UT_MoveView();
};

#endif // UT_MOVEVIEW_H
