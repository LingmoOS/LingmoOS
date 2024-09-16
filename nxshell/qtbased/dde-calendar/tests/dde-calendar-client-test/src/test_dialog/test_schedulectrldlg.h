// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_SCHEDULECTRLDLG_H
#define TEST_SCHEDULECTRLDLG_H

#include <QObject>
#include <gtest/gtest.h>
#include "dialog/schedulectrldlg.h"

class test_schedulectrldlg : public::QObject, public::testing::Test
{
public:
    test_schedulectrldlg();
    ~test_schedulectrldlg();
protected:
    CScheduleCtrlDlg *mScheduleCtrlDlg = nullptr;
};

#endif // TEST_SCHEDULECTRLDLG_H
