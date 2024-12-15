// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "gui/searchconfig/blacklistview/blacklistview.h"
#include "gui/searchconfig/blacklistview/blacklistmodel.h"
#include "business/config/searchconfig.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <DGuiApplicationHelper>

#include <QMimeData>
#include <QFileInfo>
#include <QModelIndex>
#include <QPoint>
#include <QIcon>

using namespace testing;
GRANDSEARCH_USE_NAMESPACE

TEST(BlackListViewTest, constructor)
{
    BlackListView w;

    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(BlackListView, init), [&](){
        return;
    });
    EXPECT_TRUE(w.m_model);
    EXPECT_TRUE(w.m_delegate);
    EXPECT_EQ(w.m_paths, QStringList());
    EXPECT_EQ(w.m_validRows, 0);
}

TEST(BlackListViewTest, removeRows)
{
    BlackListView w;
    stub_ext::StubExt stu;
    bool ut_call_removeRows = true;
    stu.set_lamda(VADDR(QStandardItemModel, removeRows), [&](){
        return ut_call_removeRows;
    });
    w.m_validRows = 7;
    EXPECT_TRUE(w.removeRows(0, 1));

    ut_call_removeRows = false;
    EXPECT_FALSE(w.removeRows(0, 1));
}

TEST(BlackListViewTest, insertRows)
{
    BlackListView w;
    w.m_model->insertRows(0, 1, QModelIndex());
    w.m_model->setData(w.m_model->index(0, 0, QModelIndex()), "test", DATAROLE);

    stub_ext::StubExt stu;
    QVariant datas;
    stu.set_lamda(VADDR(QStandardItem, data), [&](){
        return datas;
    });

    bool ut_call_setData = false;
    stu.set_lamda(ADDR(BlackListView, setData), [&](){
        ut_call_setData = true;
    });

    w.insertRow(QString("/"));
    EXPECT_TRUE(ut_call_setData);

    datas.setValue(1);
    ut_call_setData = false;
    w.insertRow(QString("//"));
    EXPECT_FALSE(ut_call_setData);
}

TEST(BlackListViewTest, appendRow)
{
    BlackListView w;

    stub_ext::StubExt stu;
    bool ut_call_setData = false;
    stu.set_lamda(ADDR(BlackListView, setData), [&](){
        ut_call_setData = true;
    });

    w.appendRow(QString("/"));
    EXPECT_TRUE(ut_call_setData);
}

TEST(BlackListViewTest, updateEmptyRows)
{
    BlackListView w;
    w.m_model->removeRows(0, 5, QModelIndex());

    stub_ext::StubExt stu;
    bool ut_call_appendRow = false;
    stu.set_lamda(ADDR(BlackListView, appendRow), [&](){
        ut_call_appendRow = true;
    });

    w.updateEmptyRows();
    EXPECT_TRUE(ut_call_appendRow);
}

TEST(BlackListViewTest, addModel)
{
    BlackListView w;
    w.m_validRows = 0;

    stub_ext::StubExt stu;
    bool ut_call_insertRow = false;
    stu.set_lamda(ADDR(BlackListView, insertRow), [&](){
        ut_call_insertRow = true;
    });

    bool ut_call_appendRow = false;
    stu.set_lamda(ADDR(BlackListView, appendRow), [&](){
        ut_call_appendRow = true;
    });

    w.addModel(QString("test"));
    EXPECT_TRUE(ut_call_insertRow);
    EXPECT_FALSE(ut_call_appendRow);

    w.m_validRows = 100;
    w.m_model->removeRows(0, 7, QModelIndex());
    ut_call_insertRow = false;
    w.addModel(QString("test"));
    EXPECT_TRUE(ut_call_appendRow);
    EXPECT_FALSE(ut_call_insertRow);
}

TEST(BlackListViewTest, addRow)
{
    BlackListView w;

    stub_ext::StubExt stu;

    bool ut_call_isValid = false;
    stu.set_lamda(ADDR(QModelIndex, isValid), [&](){
        return ut_call_isValid;
    });

    bool ut_call_addModel = false;
    stu.set_lamda(ADDR(BlackListView, addModel), [&](){
        ut_call_addModel = true;
    });

    bool ut_call_blackModelChanged = false;
    stu.set_lamda(ADDR(BlackListView, blackModelChanged), [&](){
        ut_call_blackModelChanged = true;
    });

    bool ut_call_moveRowToLast = false;
    stu.set_lamda(ADDR(BlackListView, moveRowToLast), [&](){
        ut_call_moveRowToLast = true;
    });

    w.addRow(QString("test"));
    EXPECT_TRUE(ut_call_addModel);
    EXPECT_TRUE(ut_call_blackModelChanged);
    EXPECT_FALSE(ut_call_moveRowToLast);

    ut_call_isValid = true;
    ut_call_addModel = false;
    ut_call_blackModelChanged = false;
    w.addRow(QString("test"));
    EXPECT_TRUE(ut_call_moveRowToLast);
    EXPECT_FALSE(ut_call_addModel);
    EXPECT_FALSE(ut_call_blackModelChanged);
}

TEST(BlackListView, moveRowLast)
{
    BlackListView w;
    stub_ext::StubExt stu;
    bool ut_call_isValid = true;
    stu.set_lamda(&QModelIndex::isValid, [&](){
        return ut_call_isValid;
    });

    bool ut_call_addModel = false;
    stu.set_lamda(ADDR(BlackListView, addModel), [&](){
        ut_call_addModel = true;
    });

    bool ut_all_removeRows = false;
    stu.set_lamda(ADDR(BlackListView, removeRows), [&](){
       return ut_all_removeRows;
    });

    w.moveRowToLast(QModelIndex());
    EXPECT_TRUE(ut_call_addModel);

    ut_call_addModel = false;
    ut_call_isValid = false;
    w.moveRowToLast(QModelIndex());
    EXPECT_FALSE(ut_call_addModel);
}

TEST(BlackListViewTest, dropEvent)
{
    QDropEvent *e = new QDropEvent(QPointF(), Qt::CopyAction, nullptr,
                                   Qt::LeftButton, Qt::NoModifier);

    stub_ext::StubExt stu;

    stu.set_lamda(ADDR(QMimeData, urls), [&](){
        QList<QUrl> urls;
        urls << QUrl("/test");
        return urls;
    });

    bool ut_call_isDir = true;
    stu.set_lamda(ADDR(QFileInfo, isDir), [&](){
        return ut_call_isDir;
    });

    bool ut_call_exists = true;
    stu.set_lamda((bool(QFileInfo::*)()const)&QFileInfo::exists, [&](){
        return ut_call_exists;
    });

    bool ut_call_addRow = false;
    stu.set_lamda(ADDR(BlackListView, addRow), [&](){
        ut_call_addRow = true;
    });

    BlackListView w;
    w.dropEvent(e);
    EXPECT_TRUE(ut_call_addRow);

    ut_call_exists = false;
    ut_call_isDir = false;
    ut_call_addRow = false;

    w.dropEvent(e);
    EXPECT_FALSE(ut_call_addRow);

    delete e;
}

TEST(BlackListViewTest, dragEnterEvent)
{
    QDragEnterEvent *e = new QDragEnterEvent(QPoint(), Qt::CopyAction, nullptr,
                                             Qt::LeftButton, Qt::NoModifier);

    stub_ext::StubExt stu;

    stu.set_lamda(ADDR(QMimeData, urls), [&](){
        QList<QUrl> urls;
        urls << QUrl("test");
        return urls;
    });

    bool ut_call_isSymLink = true;
    stu.set_lamda(ADDR(QFileInfo, isSymLink), [&](){
        return ut_call_isSymLink;
    });

    stu.set_lamda(ADDR(QFileInfo, isDir), [](){
        return true;
    });

    stu.set_lamda((bool(QFileInfo::*)()const)&QFileInfo::exists, [](){
        return true;
    });

    bool ut_call_setDropAction = false;
    stu.set_lamda(ADDR(QDragEnterEvent, setDropAction), [&](){
        ut_call_setDropAction = true;
    });

    bool ut_call_accept = false;
    auto orgFunc = (void(QDragEnterEvent::*)())&QDragEnterEvent::accept;
    stu.set_lamda(orgFunc, [&](){
        ut_call_accept = true;
    });
    BlackListView w;
    w.dragEnterEvent(e);
    EXPECT_TRUE(ut_call_setDropAction);
    EXPECT_FALSE(ut_call_accept);

    ut_call_setDropAction = false;
    ut_call_isSymLink = false;
    w.dragEnterEvent(e);
    EXPECT_TRUE(ut_call_accept);
    EXPECT_TRUE(ut_call_setDropAction);

    delete e;
}

TEST(BlackListViewTest, blackModelChanged)
{
    BlackListView w;
    w.m_model->setData(w.m_model->index(0, 0, QModelIndex()), "test", DATAROLE);

    stub_ext::StubExt stu;
    bool ut_call_setConfig = false;
    stu.set_lamda(ADDR(SearchConfig, setConfig), [&](){
        ut_call_setConfig = true;
    });

    w.blackModelChanged();
    EXPECT_TRUE(ut_call_setConfig);
}

TEST(BlackListViewTest, match)
{
    BlackListView w;
    QModelIndex indexTest;
    EXPECT_EQ(w.match(QString("///")), QModelIndex());

    w.m_model->setData(w.m_model->index(0, 0, QModelIndex()), "test", DATAROLE);

    indexTest = w.m_model->index(0, 0, QModelIndex());
    EXPECT_EQ(w.match(QString("test")), indexTest);
}

TEST(BlackListViewTest, setData)
{
    BlackListView w;

    stub_ext::StubExt stu;
    bool ut_call_isValid = false;
    stu.set_lamda(ADDR(QModelIndex, isValid), [&](){
        return ut_call_isValid;
    });

    bool ut_call_setData = false;
    stu.set_lamda(VADDR(QStandardItemModel, setData), [&](){
        ut_call_setData = true;
        return true;
    });

    w.setData(QModelIndex(), QString(""));
    EXPECT_FALSE(ut_call_setData);
    EXPECT_EQ(w.m_validRows, 0);

    ut_call_isValid = true;
    w.setData(QModelIndex(), QString(""));
    EXPECT_TRUE(ut_call_setData);
    EXPECT_EQ(w.m_validRows, 0);

    ut_call_setData = false;
    w.setData(QModelIndex(), QString("test"));
    EXPECT_TRUE(ut_call_setData);
    EXPECT_EQ(w.m_validRows, 1);
}

TEST(BlackListViewTest, init)
{
    BlackListView w;
    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(SearchConfig, getConfig), [](){
        QStringList paths;
        paths << QString("test") << QString("test2");
        QVariant data;
        data.setValue(paths);
        return data;
    });

    bool ut_call_updateEmptyRows = false;
    stu.set_lamda(ADDR(BlackListView, updateEmptyRows), [&](){
        ut_call_updateEmptyRows = true;
    });

    w.init();
    EXPECT_TRUE(ut_call_updateEmptyRows);
}

TEST(BlackListWrapperTest, constructor)
{
    BlackListWrapper w;

    EXPECT_TRUE(w.m_listView);
    EXPECT_TRUE(w.m_mainLayout);
}

TEST(BlackListWrapperTest, selectionModel)
{
    BlackListWrapper w;

    stub_ext::StubExt stu;
    bool ut_call_selectionModel = false;
    stu.set_lamda(ADDR(QAbstractItemView, selectionModel), [&](){
        ut_call_selectionModel = true;
        return nullptr;
    });

    w.selectionModel();
    EXPECT_TRUE(ut_call_selectionModel);
}

TEST(BlackListWrapperTest, removeRows)
{
    stub_ext::StubExt stu;
    bool ut_call_removeRows = false;
    stu.set_lamda(ADDR(BlackListView, removeRows), [&](){
        ut_call_removeRows = true;
        return true;
    });

    BlackListWrapper w;

    w.removeRows(1, 1);
    EXPECT_TRUE(ut_call_removeRows);
}

TEST(BlackListWrapperTest, addRow)
{
    stub_ext::StubExt stu;
    bool ut_call_addRow = false;
    stu.set_lamda(ADDR(BlackListView, addRow), [&](){
        ut_call_addRow = true;
    });

    BlackListWrapper w;
    w.addRow(QString("test"));
    EXPECT_TRUE(ut_call_addRow);
}

TEST(BlackListWrapperTest, paintEvent)
{
    BlackListWrapper w;
    QPaintEvent event(QRect(QPoint(10, 10), QSize(20, 20)));

    stub_ext::StubExt stu;
    DGuiApplicationHelper::ColorType ut_call_themeType = DGuiApplicationHelper::LightType;
    stu.set_lamda(&DGuiApplicationHelper::themeType, [&](){
        return ut_call_themeType;
    });

    EXPECT_NO_FATAL_FAILURE(w.paintEvent(&event));

    ut_call_themeType = DGuiApplicationHelper::DarkType;
    EXPECT_NO_FATAL_FAILURE(w.paintEvent(&event));
}
