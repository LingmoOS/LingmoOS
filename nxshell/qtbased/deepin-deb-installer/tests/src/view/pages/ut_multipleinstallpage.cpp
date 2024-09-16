// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../deb-installer/view/pages/multipleinstallpage.h"
#include "../deb-installer/model/deblistmodel.h"
#include "../deb-installer/manager/packagesmanager.h"
#include "../deb-installer/model/packagelistview.h"
#include "../deb-installer/view/widgets/ShowInstallInfoTextEdit.h"
#include "../deb-installer/view/widgets/installprocessinfoview.h"
#include "../deb-installer/view/widgets/workerprogress.h"

#include <stub.h>
#include <ut_Head.h>

#include <QDebug>
#include <QStandardItem>

#include <gtest/gtest.h>
using namespace QApt;

bool stud_init()
{
    return true;
}

int stud_getInstallFileSize()
{
    return 10;
}

class UT_MultipleInstallPage : public UT_HEAD
{
public:
    // 添加日志
    static void SetUpTestCase() { qDebug() << "SetUpTestCase" << endl; }
    static void TearDownTestCase() { qDebug() << "TearDownTestCase" << endl; }
    void SetUp()  // TEST跑之前会执行SetUp
    {
        Stub stub;
        stub.set(ADDR(Backend, init), stud_init);
        debListModel = new DebListModel();
        usleep(100 * 1000);
        multiplepage = new MultipleInstallPage(debListModel);
        usleep(100 * 1000);
        qDebug() << "SetUp" << endl;
    }
    void TearDown()  // TEST跑完之后会执行TearDown
    {
        delete multiplepage;
        delete debListModel;
    }
    MultipleInstallPage *multiplepage;
    DebListModel *debListModel;
};

TEST_F(UT_MultipleInstallPage, UT_MultipleInstallPage_total)
{
    multiplepage->setEnableButton(true);
    EXPECT_FALSE(multiplepage->m_installButton->isVisible());
    multiplepage->refreshModel();
    multiplepage->slotHideInfo();
    EXPECT_TRUE(multiplepage->m_upDown);
    EXPECT_FALSE(multiplepage->m_appsListView->hasFocus());
    EXPECT_FALSE(multiplepage->m_installProcessInfoView->isVisible());
    EXPECT_FALSE(multiplepage->m_appsListViewBgFrame->isVisible());
}

TEST_F(UT_MultipleInstallPage, UT_MultipleInstallPage_slotWorkerFinshed)
{
    multiplepage->slotWorkerFinshed();
    EXPECT_FALSE(multiplepage->m_acceptButton->isVisible());
    EXPECT_FALSE(multiplepage->m_backButton->isVisible());
    EXPECT_FALSE(multiplepage->m_processFrame->isVisible());
    EXPECT_FALSE(multiplepage->m_appsListView->m_bIsRightMenuShow);
}

TEST_F(UT_MultipleInstallPage, UT_MultipleInstallPage_slotOutputAvailable)
{
    multiplepage->slotOutputAvailable("");
    EXPECT_EQ("", multiplepage->m_installProcessInfoView->m_editor->toPlainText());
    EXPECT_FALSE(multiplepage->m_installButton->isVisible());
    EXPECT_FALSE(multiplepage->m_infoControlButton->isVisible());
}

TEST_F(UT_MultipleInstallPage, UT_MultipleInstallPage_slotProgressChanged)
{
    multiplepage->slotProgressChanged(100);
    EXPECT_EQ(100, multiplepage->m_progressAnimation->endValue());
    multiplepage->m_installProgress->setValue(0);
    EXPECT_EQ(0, multiplepage->m_progressAnimation->startValue());
}

TEST_F(UT_MultipleInstallPage, UT_MultipleInstallPage_slotAutoScrollInstallList)
{
    Stub stub;
    stub.set(ADDR(DebListModel, getInstallFileSize), stud_getInstallFileSize);
    multiplepage->slotAutoScrollInstallList(-1);
    multiplepage->slotAutoScrollInstallList(2);
    EXPECT_FALSE(multiplepage->m_appsListView->isVisible());
}

TEST_F(UT_MultipleInstallPage, UT_MultipleInstallPage_DealDependResult)
{
    multiplepage->DealDependResult(0, "test");
    multiplepage->DealDependResult(2, "test");
    EXPECT_TRUE(multiplepage->m_installButton->isEnabled());
    multiplepage->DealDependResult(3, "test");
    multiplepage->DealDependResult(4, "test");
    multiplepage->DealDependResult(5, "test");
    multiplepage->DealDependResult(6, "test");
    multiplepage->DealDependResult(1, "test");
    EXPECT_FALSE(multiplepage->m_appsListView->isVisible());
    EXPECT_FALSE(multiplepage->m_tipsLabel->isVisible());
    EXPECT_FALSE(multiplepage->m_installButton->isVisible());
}

TEST_F(UT_MultipleInstallPage, UT_MultipleInstallPage_afterGetAutherFalse)
{
    multiplepage->afterGetAutherFalse();
    EXPECT_TRUE(multiplepage->m_infoControlButton->isEnabled());
    EXPECT_TRUE(multiplepage->m_appsListView->m_bIsRightMenuShow);
    EXPECT_FALSE(multiplepage->m_processFrame->isVisible());
    EXPECT_FALSE(multiplepage->m_installButton->isVisible());
}

TEST_F(UT_MultipleInstallPage, UT_MultipleInstallPage_slotHideDependsInfo)
{
    multiplepage->slotHideDependsInfo();
    EXPECT_FALSE(multiplepage->m_appsListView->hasFocus());
    EXPECT_FALSE(multiplepage->m_showDependsView->isVisible());
    multiplepage->slotShowDependsInfo();
    EXPECT_FALSE(multiplepage->m_showDependsView->isVisible());
    EXPECT_FALSE(multiplepage->m_appsListView->hasFocus());
}

TEST_F(UT_MultipleInstallPage, UT_MultipleInstallPage_slotRequestRemoveItemClicked)
{
    multiplepage->slotRequestRemoveItemClicked(debListModel->index(0));
    EXPECT_TRUE(debListModel->isWorkerPrepare());
    EXPECT_TRUE(debListModel->m_packageOperateStatus.isEmpty());
    EXPECT_FALSE(multiplepage->m_showDependsButton->isVisible());
}

TEST_F(UT_MultipleInstallPage, UT_MultipleInstallPage_slotDependPackages)
{
    Pkg::DependInfo info;
    info.packageName = "libopencl1";
    info.version = "1.0";
    QList<Pkg::DependInfo> list;
    list.append(info);
    QPair<QList<Pkg::DependInfo>, QList<Pkg::DependInfo>> pair;
    pair.first.append(list);
    pair.second.append(list);
    //    dependPackages.insert("deb", pair);
    multiplepage->slotDependPackages(pair, false);
    EXPECT_EQ(1, pair.first.size());
    EXPECT_FALSE(multiplepage->m_showDependsButton->isVisible());
}

TEST_F(UT_MultipleInstallPage, UT_MultipleInstallPage_slotShowInfo)
{
    multiplepage->slotShowInfo();
    EXPECT_EQ(multiplepage->m_upDown, false);
    EXPECT_FALSE(multiplepage->m_appsListView->hasFocus());
    EXPECT_FALSE(multiplepage->m_installProcessInfoView->isVisible());
    EXPECT_FALSE(multiplepage->m_appsListViewBgFrame->isVisible());
}

TEST_F(UT_MultipleInstallPage, UT_MultipleInstallPage_slotHiddenCancelButton)
{
    multiplepage->slotHiddenCancelButton();
    EXPECT_FALSE(multiplepage->m_installButton->hasFocus());
    EXPECT_FALSE(multiplepage->m_appsListView->m_bIsRightMenuShow);
    EXPECT_FALSE(multiplepage->m_showDependsButton->isVisible());
    EXPECT_FALSE(multiplepage->m_installButton->isVisible());
    EXPECT_FALSE(multiplepage->m_backButton->isVisible());
}
