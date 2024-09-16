// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_VNTASKWORKER_H
#define UT_VNTASKWORKER_H

#include "gtest/gtest.h"
#include <QTest>
#include <QObject>

class VNTaskWorker;

class UT_VNTaskWorker : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    UT_VNTaskWorker();

    virtual void SetUp() override;
    virtual void TearDown() override;

protected:
    VNTaskWorker *m_vntaskworker {nullptr};
};

#endif // UT_VNTASKWORKER_H
