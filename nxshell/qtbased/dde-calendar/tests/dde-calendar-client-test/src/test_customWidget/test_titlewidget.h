// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_TITLEWIDGET_H
#define TEST_TITLEWIDGET_H

#include <QObject>
#include <gtest/gtest.h>
#include "customWidget/ctitlewidget.h"

class test_titleWidget : public QObject,public testing::Test
{
    Q_OBJECT
public:
    explicit test_titleWidget();
    void SetUp() override;
    void TearDown() override;
signals:

public slots:
public:
    CTitleWidget *m_titleWidget = nullptr;
};

#endif // TEST_TITLEWIDGET_H
