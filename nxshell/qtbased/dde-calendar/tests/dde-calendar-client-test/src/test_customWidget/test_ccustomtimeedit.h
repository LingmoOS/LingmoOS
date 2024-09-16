// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CCUSTOMTIMEEDIT_H
#define TEST_CCUSTOMTIMEEDIT_H

#include <gtest/gtest.h>
#include "customWidget/ccustomtimeedit.h"

#include <QObject>

class test_CCustomTimeEdit : public QObject
    , public testing::Test
{
    Q_OBJECT
public:
    explicit test_CCustomTimeEdit(QObject *parent = nullptr)
        : QObject(parent) {

          };

    void SetUp() override;
    void TearDown() override;
signals:

public slots:
public:
    CCustomTimeEdit *m_timeEdit = nullptr;
};

#endif // TEST_CCUSTOMTIMEEDIT_H
