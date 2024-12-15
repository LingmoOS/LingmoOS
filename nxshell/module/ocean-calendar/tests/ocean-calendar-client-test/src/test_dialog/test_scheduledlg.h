// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_SCHEDULEDLG_H
#define TEST_SCHEDULEDLG_H

#include <QObject>
#include <gtest/gtest.h>
#include "dialog/scheduledlg.h"
class test_scheduledlg : public::QObject, public::testing::Test
{
public:
    test_scheduledlg();
    ~test_scheduledlg();
protected:
    CScheduleDlg *mScheduleDlg = nullptr;
};

#endif // TEST_SCHEDULEDLG_H
