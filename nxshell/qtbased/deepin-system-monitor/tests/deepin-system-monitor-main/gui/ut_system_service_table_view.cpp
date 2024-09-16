// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "system_service_table_view.h"
#include "service/service_manager.h"
#include "settings.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <QKeyEvent>
#include <QTreeView>
#include <QHeaderView>
#include <DDialog>
#include <QtConcurrent>


/***************************************STUB begin*********************************************/
QModelIndexList stub_system_service_eventFilter_selectedIndexes(void* object)
{
    QModelIndexList list;
    list << QModelIndex() << QModelIndex() <<QModelIndex();
    return  list;
}

bool stub_system_service_eventFilter_hasFocus_true()
{
    return true;
}

void stub_startService_exec()
{
    return;
}

void stub_startService_run()
{
    return;
}

void stub_refresh_updateServiceList()
{
    return;
}

Settings* stub_loadSettings_instance()
{
    return nullptr;
}
/***************************************STUB end**********************************************/

class UT_SystemServiceTableView : public ::testing::Test
{
public:
    UT_SystemServiceTableView() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static QWidget wid;
        m_tester = new SystemServiceTableView(&wid);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    SystemServiceTableView *m_tester;
};

TEST_F(UT_SystemServiceTableView, initTest)
{

}


TEST_F(UT_SystemServiceTableView, test_getSourceModel_01)
{
    m_tester->getSourceModel();

    EXPECT_TRUE(m_tester->m_model);
}

TEST_F(UT_SystemServiceTableView, test_eventFilter_01)
{
    typedef int (*fptr)(QTreeView*);
    fptr QTreeView_selectedIndexes = (fptr)(&QTreeView::selectedIndexes);   //obtaining an address
    Stub stub;
    stub.set(QTreeView_selectedIndexes, stub_system_service_eventFilter_selectedIndexes);

    stub.set(ADDR(SystemServiceTableView, hasFocus), stub_system_service_eventFilter_hasFocus_true);
    static QKeyEvent keyEv(QEvent::KeyPress, Qt::Key_M, Qt::AltModifier);
    m_tester->eventFilter(m_tester, &keyEv);
}

TEST_F(UT_SystemServiceTableView, test_startService_01)
{
    m_tester->startService();

    EXPECT_FALSE(m_tester->m_selectedSName.isValid());
}

TEST_F(UT_SystemServiceTableView, test_startService_02)
{
    typedef int (*fptr)(DDialog*);
    fptr DDialog_exec = (fptr)((int(DDialog::*)(void))&DDialog::exec);

    Stub stub;
    stub.set(DDialog_exec, stub_startService_exec);
    m_tester->m_selectedSName = QVariant("111111111111@");
    m_tester->startService();

    EXPECT_TRUE(m_tester->m_selectedSName.isValid());
}

TEST_F(UT_SystemServiceTableView, test_stopService_01)
{
    m_tester->stopService();

    EXPECT_FALSE(m_tester->m_selectedSName.isValid());
}

TEST_F(UT_SystemServiceTableView, test_stopService_02)
{
    typedef int (*fptr)(DDialog*);
    fptr DDialog_exec = (fptr)((int(DDialog::*)(void))&DDialog::exec);

    Stub stub;
    stub.set(DDialog_exec, stub_startService_exec);
    m_tester->m_selectedSName = QVariant("111111111111@");
    m_tester->stopService();

    EXPECT_TRUE(m_tester->m_selectedSName.isValid());
}

TEST_F(UT_SystemServiceTableView, test_restartService_01)
{
    m_tester->restartService();

    EXPECT_FALSE(m_tester->m_selectedSName.isValid());
}

TEST_F(UT_SystemServiceTableView, test_restartService_02)
{

    typedef int (*fptr)(DDialog*);
    fptr DDialog_exec = (fptr)((int(DDialog::*)(void))&DDialog::exec);

    Stub stub;
    stub.set(DDialog_exec, stub_startService_exec);
    m_tester->m_selectedSName = QVariant("111111111111@");

    m_tester->restartService();
    EXPECT_TRUE(m_tester->m_selectedSName.isValid());
}

TEST_F(UT_SystemServiceTableView, test_setServiceStartupMode_01)
{
    m_tester->setServiceStartupMode(true);

    EXPECT_FALSE(m_tester->m_selectedSName.isValid());
}

TEST_F(UT_SystemServiceTableView, test_refresh_01)
{
    Stub stub;
    stub.set(ADDR(ServiceManager, updateServiceList), stub_refresh_updateServiceList);
    m_tester->refresh();

    EXPECT_FALSE(m_tester->m_loading);
}

TEST_F(UT_SystemServiceTableView, test_search_01)
{
    m_tester->search("1");

    EXPECT_TRUE(m_tester->m_proxyModel);
}

TEST_F(UT_SystemServiceTableView, test_resizeEvent_01)
{
    static QResizeEvent ev(QSize(0, 0), QSize(10, 10));
    m_tester->resizeEvent(&ev);

    EXPECT_TRUE(m_tester->m_spinner);
}

TEST_F(UT_SystemServiceTableView, test_displayHeaderContextMenu_01)
{
    m_tester->displayHeaderContextMenu(QPoint(0, 0));
}

TEST_F(UT_SystemServiceTableView, test_displayTableContextMenu_01)
{
    m_tester->displayTableContextMenu(QPoint(0, 0));

    EXPECT_EQ(m_tester->selectedIndexes().size(), 0);
}

TEST_F(UT_SystemServiceTableView, test_displayTableContextMenu_02)
{
    typedef int (*fptr)(QTreeView*);
    fptr QTreeView_selectedIndexes = (fptr)(&QTreeView::selectedIndexes);   //obtaining an address
    Stub stub;
    stub.set(QTreeView_selectedIndexes, stub_system_service_eventFilter_selectedIndexes);

    m_tester->displayTableContextMenu(QPoint(0, 0));

    EXPECT_NE(m_tester->selectedIndexes().size(), 0);
}

TEST_F(UT_SystemServiceTableView, test_loadSettings_01)
{
    EXPECT_TRUE(m_tester->loadSettings());
}

TEST_F(UT_SystemServiceTableView, test_saveSettings_01)
{
    m_tester->saveSettings();
}

TEST_F(UT_SystemServiceTableView, test_onLoadServiceDataList_01)
{
    Stub stub;
    stub.set(ADDR(ServiceManager, updateServiceList), stub_refresh_updateServiceList);
    m_tester->onLoadServiceDataList();
}

TEST_F(UT_SystemServiceTableView, test_sizeHintForColumn_01)
{
    int value = m_tester->sizeHintForColumn(0);

    EXPECT_EQ(value , 20);
}

TEST_F(UT_SystemServiceTableView, test_selectionChanged_01)
{
    QItemSelection selected;
    QItemSelection dselected;
    m_tester->selectionChanged(selected, dselected);
}

TEST_F(UT_SystemServiceTableView, test_initUI_01)
{
    m_tester->initUI(false);

    EXPECT_TRUE(m_tester->m_spinner);
}

TEST_F(UT_SystemServiceTableView, test_initConnections_01)
{
    m_tester->initConnections();
}

TEST_F(UT_SystemServiceTableView, test_adjustInfoLabelVisibility_01)
{
    m_tester->adjustInfoLabelVisibility();
}

TEST_F(UT_SystemServiceTableView, test_refreshServiceStatus_01)
{
    m_tester->refreshServiceStatus("1");
}











