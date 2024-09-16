// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_configsettings.h"

test_configsettings::test_configsettings()
{
}

test_configsettings::~test_configsettings()
{
}

void test_configsettings::SetUp()
{
    conf = CConfigSettings::getInstance();
}

void test_configsettings::TearDown()
{

}

void stub_Handle()
{

}

void setValue_stub(void *obj, const QString &key, const QVariant &value)
{
    Q_UNUSED(obj)
        Q_UNUSED(key)
            Q_UNUSED(value)}

//void CConfigSettings::sync()
TEST_F(test_configsettings, sync)
{
    Stub stub;
    stub.set(ADDR(CConfigSettings, sync), stub_Handle);
    conf->sync();
}

TEST_F(test_configsettings, setOption)
{
    Stub stub;
    stub.set(ADDR(QSettings, setValue), setValue_stub);
    conf->setOption("base.state", 0);
}
