// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_VNOTEITEM_H
#define UT_VNOTEITEM_H

#include "gtest/gtest.h"
#include <QTest>
#include <QObject>

class UT_VnoteItem : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    UT_VnoteItem();
};

#endif // UT_VNOTEITEM_H
