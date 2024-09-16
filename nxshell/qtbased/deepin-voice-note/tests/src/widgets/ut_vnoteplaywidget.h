// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_VNOTEPLAYWIDGET_H
#define UT_VNOTEPLAYWIDGET_H

#include "gtest/gtest.h"
#include <QTest>
#include <QObject>

class VNotePlayWidget;
class UT_VNotePlayWidget : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    UT_VNotePlayWidget();
    VNotePlayWidget *m_vnoteplaywidget {nullptr};
    // Test interface
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif // UT_VNOTEPLAYWIDGET_H
