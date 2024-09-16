// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "gui/searchconfig/blacklistview/blacklistmodel.h"

#include "stubext.h"

#include <gtest/gtest.h>

GRANDSEARCH_USE_NAMESPACE

TEST(BlackListModelTest, constructor)
{
    BlackListModel model(10, 10);

    EXPECT_EQ(model.rowCount(), 10);
    EXPECT_EQ(model.columnCount(), 10);
}

TEST(BlackListModelTest, flags)
{
    stub_ext::StubExt stu;
    QVariant data;
    data.setValue(QString("test"));
    stu.set_lamda(&QModelIndex::data, [&](){
        return data;
    });

    stu.set_lamda(VADDR(QStandardItemModel, flags), [](){
        return Qt::NoItemFlags;
    });

    BlackListModel model(10, 10);
    EXPECT_TRUE(model.flags(QModelIndex()).testFlag(Qt::ItemIsSelectable));

    data.clear();
    data.setValue(QString(""));
    EXPECT_FALSE(model.flags(QModelIndex()).testFlag(Qt::ItemIsSelectable));
}

TEST(BlackListModelTest, mimeTypes)
{
    BlackListModel model(10, 10);
    stub_ext::StubExt stu;
    bool ut_call_mimeTypes = false;
    stu.set_lamda(VADDR(QStandardItemModel, mimeTypes), [&]{
        ut_call_mimeTypes = true;
        return QStringList();
    });
    model.mimeTypes();
    EXPECT_TRUE(ut_call_mimeTypes);
}
