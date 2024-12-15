// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_SECURITYLOGDLG_H
#define UT_SECURITYLOGDLG_H

#include "gtest/gtest.h"
#include <QObject>

class MockSecurityLogDataInterface;

class SecurityLogDlgTest : public testing::Test
    , public QObject
{
public:
    SecurityLogDlgTest();
    static void SetUpTestCase();
    static void TearDownTestCase();

public:
    MockSecurityLogDataInterface *m_interface;
};

#endif // UT_SECURITYLOGDLG_H
