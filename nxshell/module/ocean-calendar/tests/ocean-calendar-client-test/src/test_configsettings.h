// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CONFIGSETTINGS_H
#define TEST_CONFIGSETTINGS_H

#include "configsettings.h"
#include "../third-party_stub/stub.h"
#include "gtest/gtest.h"
#include <QObject>

class test_configsettings : public QObject, public::testing::Test
{
public:
    test_configsettings();
    ~test_configsettings();
    void SetUp() override;
    void TearDown() override;

protected:
    CConfigSettings *conf = nullptr;
};

#endif // TEST_CONFIGSETTINGS_H
