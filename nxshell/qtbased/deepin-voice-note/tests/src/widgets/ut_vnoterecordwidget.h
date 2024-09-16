// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_VNOTERECORDWIDGET_H
#define UT_VNOTERECORDWIDGET_H

#include "gtest/gtest.h"
#include <QTest>
#include <QObject>

class VNoteRecordWidget;
class UT_VNoteRecordWidget : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    UT_VNoteRecordWidget();

    // Test interface
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;

private:
    VNoteRecordWidget *m_vnoterecordwidget {nullptr};
};

#endif // UT_VNOTERECORDWIDGET_H
