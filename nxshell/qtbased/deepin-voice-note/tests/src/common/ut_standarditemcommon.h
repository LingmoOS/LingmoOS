// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_STANDARDITEMCOMMON_H
#define UT_STANDARDITEMCOMMON_H

#include "gtest/gtest.h"
#include <QTest>
#include <QObject>

class StandardItemCommon;

class UT_StandardItemCommon : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    UT_StandardItemCommon();

    virtual void SetUp() override;
    virtual void TearDown() override;

protected:
    StandardItemCommon *m_standarditemcommon {nullptr};
};

#endif // UT_STANDARDITEMCOMMON_H
