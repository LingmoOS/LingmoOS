// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_VNOTEFOLDEROPER_H
#define UT_VNOTEFOLDEROPER_H

#include "gtest/gtest.h"
#include <QTest>
#include <QObject>

class VNoteFolderOper;
class UT_VNoteFolderOper : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    UT_VNoteFolderOper();

    virtual void SetUp() override;
    virtual void TearDown() override;

protected:
    VNoteFolderOper *m_vnotefolderoper {nullptr};
};

#endif // UT_VNOTEFOLDEROPER_H
