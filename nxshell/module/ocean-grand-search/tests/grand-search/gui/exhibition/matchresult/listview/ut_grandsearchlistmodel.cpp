// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gui/exhibition/matchresult/listview/grandsearchlistmodel.h"
#include "global/matcheditem.h"
#include "utils/utils.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QEvent>
#include <QWidget>

using namespace testing;
using namespace GrandSearch;

TEST(GrandSearchListModelTest, constructor)
{
    GrandSearchListModel *m = new GrandSearchListModel(2, 2);

    ASSERT_TRUE(m);

    delete m;
}

TEST(GrandSearchListModelTest, setData)
{
    GrandSearchListModel m(2, 2);

    typedef bool (*fptr)(QStandardItemModel*,const QModelIndex &, const QVariant &, int);
    fptr ut_setData = (fptr)(&QStandardItemModel::setData);

    stub_ext::StubExt stu;

    bool ut_call_setData = false;
    stu.set_lamda(ut_setData, [&](){
        ut_call_setData = true;
        return true;
    });

    m.setData(QModelIndex(), 0);
    EXPECT_TRUE(ut_call_setData);
}

TEST(GrandSearchListModelTest, flags)
{
    GrandSearchListModel m(2, 2);

    auto flags = m.flags(QModelIndex());
    EXPECT_TRUE(flags.testFlag(Qt::ItemIsDropEnabled));

    auto index = m.index(0, 0);
    flags = m.flags(index);
    EXPECT_TRUE(flags.testFlag(Qt::ItemIsDragEnabled));
}
