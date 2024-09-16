// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_OPSSTATEINTERFACE_H
#define UT_OPSSTATEINTERFACE_H

#include "gtest/gtest.h"
#include <QTest>
#include <QObject>

class OpsStateInterface;
class UT_OpsStateInterface : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    UT_OpsStateInterface();

    virtual void SetUp() override;
    virtual void TearDown() override;

protected:
    OpsStateInterface *m_opsstateinterface {nullptr};
};

#endif // UT_OPSSTATEINTERFACE_H
