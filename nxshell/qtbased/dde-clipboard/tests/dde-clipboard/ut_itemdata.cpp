// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "itemdata.h"

#include <QDebug>
#include <QSignalSpy>

class TstItemData : public testing::Test
{
public:
    void SetUp() override
    {
        QByteArray buf;
        data = new ItemData(buf);
    }

    void TearDown() override
    {
        delete data;
        data = nullptr;
    }

public:
    ItemData *data = nullptr;
};

TEST_F(TstItemData, coverageTest)
{
    data->setDataEnabled(true);
    ASSERT_TRUE(data->dataEnabled());

    data->setDataEnabled(false);
    ASSERT_FALSE(data->dataEnabled());

    QSignalSpy destroySpy(data, &ItemData::destroy);
    data->remove();
    ASSERT_EQ(destroySpy.count(), 1);

    QSignalSpy popSpy(data, &ItemData::reborn);
    data->popTop();
    ASSERT_EQ(popSpy.count(), 1);
}
