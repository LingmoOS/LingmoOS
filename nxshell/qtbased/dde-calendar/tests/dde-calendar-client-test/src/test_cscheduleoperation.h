// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CSCHEDULEOPERATION_H
#define TEST_CSCHEDULEOPERATION_H

#include "../third-party_stub/stub.h"
#include "scheduleTask/cscheduleoperation.h"
#include "gtest/gtest.h"

#include <QObject>

class test_cscheduleoperation : public QObject
    , public ::testing::Test
{
public:
    test_cscheduleoperation();
    ~test_cscheduleoperation();
    void SetUp() override;
    void TearDown() override;

public:
    CScheduleOperation operation;

public:
    Stub stub;
};

#endif // TEST_CSCHEDULEOPERATION_H
