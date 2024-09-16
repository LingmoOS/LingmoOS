// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_SCHEDULERDATABASE_H
#define TEST_SCHEDULERDATABASE_H

#include "schedulerdatabase.h"
#include "gtest/gtest.h"
#include "../third-party_stub/stub.h"
#include <QObject>
#include <QVariant>
#include <QSqlQuery>

static bool schedulerdatabase_next = false;

static bool schedulerdatabase_stub_exec(const QString&)
{
    schedulerdatabase_next = false;
    return true;
};

static bool schedulerdatabase_stub_next()
{
    static int count = 0;
    if (!schedulerdatabase_next) {
        schedulerdatabase_next = true;
        count = 0;
    }
    count++;
    if (count >= 10) {
        schedulerdatabase_next = false;
        count = 0;
    }
    return schedulerdatabase_next;
};

static QVariant schedulerdatabase_stub_value(const QString&)
{
    return QVariant();
};

class test_schedulerdatabase: public::testing::Test
{
public:
    test_schedulerdatabase();

    virtual void SetUp()
    {
        Stub stub;
        stub.set((bool(QSqlQuery::*)(const QString&))ADDR(QSqlQuery, exec), schedulerdatabase_stub_exec);
        stub.set((QVariant(QSqlQuery::*)(const QString&)const)ADDR(QSqlQuery, value), schedulerdatabase_stub_value);
        stub.set(ADDR(QSqlQuery, next), schedulerdatabase_stub_next);
        mBase = new SchedulerDatabase();
    }

    virtual void TearDown()
    {
        delete mBase;
        mBase = nullptr;
    }
protected:
    SchedulerDatabase *mBase = nullptr;
};

#endif // TEST_SCHEDULERDATABASE_H
