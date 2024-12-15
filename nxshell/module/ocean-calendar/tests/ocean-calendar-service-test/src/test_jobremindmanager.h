// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_JOBREMINDMANAGER_H
#define TEST_JOBREMINDMANAGER_H

#include "jobremindmanager.h"
#include "gtest/gtest.h"
#include <QObject>

class test_jobremindmanager : public QObject, public::testing::Test
{
public:
    test_jobremindmanager();
    ~test_jobremindmanager();
protected:
    JobRemindManager *jobRemindManager = nullptr;
};

#endif // TEST_JOBREMINDMANAGER_H
