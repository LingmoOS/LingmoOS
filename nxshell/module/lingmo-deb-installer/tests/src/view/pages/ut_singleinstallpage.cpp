// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../deb-installer/view/pages/singleinstallpage.h"
#include "../deb-installer/model/deblistmodel.h"
#include "../deb-installer/manager/packagesmanager.h"
#include "../deb-installer/manager/PackageDependsStatus.h"
#include "../deb-installer/view/widgets/workerprogress.h"
#include "../deb-installer/view/widgets/ShowInstallInfoTextEdit.h"

#include <stub.h>
#include <ut_Head.h>

#include <QDebug>

#include <gtest/gtest.h>
using namespace QApt;

void stud_installPackages()
{
    return;
}

bool stud_singleinit()
{
    return true;
}

QString stud_singlepackageName()
{
    return "test";
}

QString stud_singleversion()
{
    return "1.0.0";
}

QString stud_singlelongDescription()
{
    return "this is test";
}

QString stud_singlearchitecture()
{
    return "x86";
}

QString stud_singlefilePath()
{
    return "/home";
}

QString stud_singleshortDescription()
{
    return "this is a short test";
}

PackageDependsStatus stud_singlegetPackageDependsStatus(const int index)
{
    Q_UNUSED(index);
    PackageDependsStatus m_status;
    m_status.status = 1;
    return m_status;
}

int stud_singlepackageInstallStatus(const int index)
{
    Q_UNUSED(index);
    return 1;
}

void stud_singleuninstallPackage(const int idx)
{
    Q_UNUSED(idx);
}

int stud_successtoInt(bool *ok = nullptr)
{
    Q_UNUSED(ok);
    return 2;
}
int stud_failedtoInt(bool *ok = nullptr)
{
    Q_UNUSED(ok);
    return 3;
}
bool stud_recheckPackagePath(QString)
{
    return true;
}

PackageDependsStatus stud_getPackageDependsStatus(const int)
{
    return PackageDependsStatus::_break("package");
}

class UT_SingleInstallpage : public UT_HEAD
{
public:
    // 添加日志
    static void SetUpTestCase() { qDebug() << "SetUpTestCase" << endl; }
    static void TearDownTestCase() { qDebug() << "TearDownTestCase" << endl; }
    void SetUp()  // TEST跑之前会执行SetUp
    {
        qDebug() << "SetUp" << endl;
    }
    void TearDown()  // TEST跑完之后会执行TearDown
    {
        delete page;
        delete model;
    }
    SingleInstallPage *page;
    DebListModel *model;
};

QVariant stu_data()
{
    return Pkg::DependsStatus::DependsOk;
}

QVariant stu_data1()
{
    return Pkg::PackageOperationStatus::Success;
}

TEST_F(UT_SingleInstallpage, UT_SingleInstallpage_total)
{
    Stub stub;
    stub.set(ADDR(Backend, init), stud_singleinit);
    stub.set(ADDR(DebFile, packageName), stud_singlepackageName);
    stub.set(ADDR(DebFile, version), stud_singleversion);
    stub.set(ADDR(DebFile, longDescription), stud_singlelongDescription);
    stub.set(ADDR(DebFile, filePath), stud_singlefilePath);
    stub.set(ADDR(DebFile, architecture), stud_singlearchitecture);
    stub.set(ADDR(DebFile, shortDescription), stud_singleshortDescription);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stud_singlegetPackageDependsStatus);
    stub.set(ADDR(PackagesManager, packageInstallStatus), stud_singlepackageInstallStatus);
    stub.set(decltype (&stud_installPackages)(&DebListModel::slotInstallPackages), stud_installPackages);
    stub.set(decltype (&stud_singleuninstallPackage)(&DebListModel::slotUninstallPackage), stud_singleuninstallPackage);
    stub.set(ADDR(DebListModel, recheckPackagePath), stud_recheckPackagePath);

    model = new DebListModel();
    model->m_packagesManager->m_preparedPackages.append("test");
    model->m_packagesManager->m_preparedPackages.append("test1");
    page = new SingleInstallPage(model);
    page->slotReinstall();
    EXPECT_FALSE(page->m_reinstallButton->hasFocus());
    EXPECT_EQ("Show details", page->m_infoControlButton->m_expandTips);
    EXPECT_FALSE(page->m_installButton->isVisible());
    EXPECT_FALSE(page->m_uninstallButton->isVisible());
    EXPECT_FALSE(page->m_backButton->isVisible());
    EXPECT_FALSE(page->m_reinstallButton->isVisible());
    EXPECT_FALSE(page->m_progressFrame->isVisible());
    EXPECT_EQ("Show details", page->m_infoControlButton->m_expandTips);
    EXPECT_EQ(SingleInstallPage::Reinstall, page->m_operate);
    EXPECT_EQ(DebListModel::WorkerPrepare, page->m_packagesModel->m_workerStatus);

    EXPECT_EQ(page->initLabelWidth(11), 260);
    EXPECT_EQ(page->initLabelWidth(12), 255);
    EXPECT_EQ(page->initLabelWidth(13), 250);
    EXPECT_EQ(page->initLabelWidth(14), 250);
    EXPECT_EQ(page->initLabelWidth(15), 240);
    EXPECT_EQ(page->initLabelWidth(16), 240);
    EXPECT_EQ(page->initLabelWidth(18), 230);
    EXPECT_EQ(page->initLabelWidth(20), 220);
    EXPECT_EQ(page->initLabelWidth(22), 220);

    page->slotInstall();
    EXPECT_FALSE(page->m_installButton->hasFocus());
    EXPECT_EQ("Show details", page->m_infoControlButton->m_expandTips);
    EXPECT_FALSE(page->m_installButton->isVisible());
    EXPECT_FALSE(page->m_progressFrame->isVisible());

    page->slotUninstallCurrentPackage();
    EXPECT_FALSE(page->m_tipsLabel->isVisible());
    EXPECT_FALSE(page->m_uninstallButton->isVisible());
    EXPECT_FALSE(page->m_backButton->isVisible());
    EXPECT_FALSE(page->m_reinstallButton->isVisible());
    EXPECT_FALSE(page->m_progressFrame->isVisible());
    EXPECT_EQ("Show details", page->m_infoControlButton->m_expandTips);
    EXPECT_EQ(SingleInstallPage::Uninstall, page->m_operate);
    EXPECT_EQ(DebListModel::WorkerPrepare, page->m_packagesModel->m_workerStatus);

    page->slotShowInfomation();
    EXPECT_FALSE(page->m_upDown);
    EXPECT_FALSE(page->m_installProcessView->isVisible());
    EXPECT_FALSE(page->m_itemInfoFrame->isVisible());

    page->slotHideInfomation();
    EXPECT_TRUE(page->m_upDown);
    EXPECT_FALSE(page->m_installProcessView->isVisible());
    EXPECT_FALSE(page->m_itemInfoFrame->isVisible());

    page->slotShowInfo();
    EXPECT_FALSE(page->m_uninstallButton->isVisible());
    EXPECT_FALSE(page->m_backButton->isVisible());
    EXPECT_FALSE(page->m_reinstallButton->isVisible());
    EXPECT_FALSE(page->m_installButton->isVisible());
    EXPECT_FALSE(page->m_doneButton->isVisible());
    EXPECT_FALSE(page->m_progressFrame->isVisible());
    EXPECT_FALSE(page->m_showDependsButton->isVisible());
    EXPECT_FALSE(page->m_infoControlButton->isVisible());
    EXPECT_FALSE(page->m_confirmButton->isVisible());
    EXPECT_EQ("", page->m_tipsLabel->text());

    page->slotOutputAvailable("");
    EXPECT_TRUE(page->m_installProcessView->m_editor->toPlainText().isEmpty());
    EXPECT_FALSE(page->m_infoControlButton->isVisible());
    page->m_progress->setValue(0);
    EXPECT_EQ(0, page->m_progress->value());
    EXPECT_TRUE(page->m_workerStarted);

    page->m_upDown = false;
    page->slotWorkerFinished();
    EXPECT_FALSE(page->m_uninstallButton->isVisible());
    EXPECT_FALSE(page->m_backButton->isVisible());
    EXPECT_FALSE(page->m_reinstallButton->isVisible());
    EXPECT_FALSE(page->m_progressFrame->isVisible());
    EXPECT_EQ(0, page->m_progress->value());
    EXPECT_FALSE(page->m_infoControlButton->isVisible());
    EXPECT_FALSE(page->m_upDown);
    Stub stub1;
    stub1.set(ADDR(QModelIndex, data), stu_data1);
    page->m_operate = SingleInstallPage::Unknown;
    page->slotWorkerFinished();

    page->slotWorkerProgressChanged(100);
    EXPECT_EQ(100, page->m_progress->value());

    page->m_progress->setValue(60);
    page->slotWorkerProgressChanged(50);

    page->setEnableButton(false);
    page->setEnableButton(true);
    EXPECT_TRUE(page->m_uninstallButton->isEnabled());
    EXPECT_TRUE(page->m_reinstallButton->isEnabled());
    EXPECT_TRUE(page->m_installButton->isEnabled());

    page->afterGetAutherFalse();
    EXPECT_FALSE(page->m_uninstallButton->isVisible());
    EXPECT_FALSE(page->m_reinstallButton->isVisible());

    page->setAuthConfirm("test");
    EXPECT_FALSE(page->m_pLoadingLabel->isVisible());
    EXPECT_FALSE(page->m_uninstallButton->isVisible());
    EXPECT_FALSE(page->m_backButton->isVisible());
    EXPECT_FALSE(page->m_reinstallButton->isVisible());
    EXPECT_FALSE(page->m_installButton->isVisible());
    EXPECT_FALSE(page->m_confirmButton->isVisible());
    EXPECT_FALSE(page->m_infoControlButton->isVisible());

    page->setAuthBefore();
    EXPECT_FALSE(page->m_uninstallButton->isVisible());
    EXPECT_FALSE(page->m_backButton->isVisible());
    EXPECT_FALSE(page->m_reinstallButton->isVisible());

    page->setCancelAuthOrAuthDependsErr();
    stub.set(ADDR(QModelIndex, data), stu_data);
    page->setCancelAuthOrAuthDependsErr();
    EXPECT_FALSE(page->m_pLoadingLabel->isVisible());
    EXPECT_FALSE(page->m_pDSpinner->isVisible());

    page->DealDependResult(1, "test");
    EXPECT_EQ(1, page->dependAuthStatu);

    page->m_operate = SingleInstallPage::Install;
    page->setEnableButton(true);
    page->setAuthBefore();
    page->afterGetAutherFalse();
    page->m_operate = SingleInstallPage::Reinstall;
    page->setAuthBefore();
    page->afterGetAutherFalse();
    EXPECT_FALSE(page->m_uninstallButton->isEnabled());
    EXPECT_FALSE(page->m_reinstallButton->isEnabled());
    EXPECT_FALSE(page->m_installButton->isEnabled());
}

TEST_F(UT_SingleInstallpage, UT_SingleInstallpage_onWorkFinishedFailed)
{
    Stub stub;
    stub.set(ADDR(Backend, init), stud_singleinit);
    stub.set(ADDR(DebFile, packageName), stud_singlepackageName);
    stub.set(ADDR(DebFile, version), stud_singleversion);
    stub.set(ADDR(DebFile, longDescription), stud_singlelongDescription);
    stub.set(ADDR(DebFile, filePath), stud_singlefilePath);
    stub.set(ADDR(DebFile, architecture), stud_singlearchitecture);
    stub.set(ADDR(DebFile, shortDescription), stud_singleshortDescription);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stud_singlegetPackageDependsStatus);
    stub.set(ADDR(PackagesManager, packageInstallStatus), stud_singlepackageInstallStatus);
    stub.set((decltype(&stud_installPackages))ADDR(DebListModel, slotInstallPackages), stud_installPackages);
    stub.set((decltype(&stud_singleuninstallPackage))ADDR(DebListModel, slotUninstallPackage), stud_singleuninstallPackage);
    stub.set(ADDR(DebListModel, recheckPackagePath), stud_recheckPackagePath);

    model = new DebListModel();
    usleep(100 * 1000);
    model->m_packagesManager->m_preparedPackages.append("test");
    model->m_packagesManager->m_preparedPackages.append("test1");
    page = new SingleInstallPage(model);
    usleep(100 * 1000);
    stub.set(ADDR(QVariant, toInt), stud_failedtoInt);
    page->m_operate = SingleInstallPage::Uninstall;
    page->slotWorkerFinished();
    page->m_operate = SingleInstallPage::Install;
    page->slotWorkerFinished();
    EXPECT_FALSE(page->m_uninstallButton->isVisible());
    EXPECT_FALSE(page->m_backButton->isVisible());
    EXPECT_FALSE(page->m_reinstallButton->isVisible());
    EXPECT_FALSE(page->m_progressFrame->isVisible());
    EXPECT_EQ(0, page->m_progress->value());
    EXPECT_FALSE(page->m_infoControlButton->isVisible());
    EXPECT_TRUE(page->m_upDown);
}

TEST_F(UT_SingleInstallpage, UT_SingleInstallpage_onWorkFinishedSuccees)
{
    Stub stub;
    stub.set(ADDR(Backend, init), stud_singleinit);
    stub.set(ADDR(DebFile, packageName), stud_singlepackageName);
    stub.set(ADDR(DebFile, version), stud_singleversion);
    stub.set(ADDR(DebFile, longDescription), stud_singlelongDescription);
    stub.set(ADDR(DebFile, filePath), stud_singlefilePath);
    stub.set(ADDR(DebFile, architecture), stud_singlearchitecture);
    stub.set(ADDR(DebFile, shortDescription), stud_singleshortDescription);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stud_singlegetPackageDependsStatus);
    stub.set(ADDR(PackagesManager, packageInstallStatus), stud_singlepackageInstallStatus);
    stub.set((decltype(&stud_installPackages))ADDR(DebListModel, slotInstallPackages), stud_installPackages);
    stub.set((decltype(&stud_singleuninstallPackage))ADDR(DebListModel, slotUninstallPackage), stud_singleuninstallPackage);
    stub.set(ADDR(QModelIndex, data), stu_data);
    stub.set(ADDR(DebListModel, recheckPackagePath), stud_recheckPackagePath);
    model = new DebListModel();
    usleep(100 * 1000);
    model->m_packagesManager->m_preparedPackages.append("test");
    model->m_packagesManager->m_preparedPackages.append("test1");
    page = new SingleInstallPage(model);
    usleep(100 * 1000);
    page->showPackageInfo();

    stub.set(ADDR(QVariant, toInt), stud_successtoInt);
    page->slotWorkerFinished();
    page->m_operate = SingleInstallPage::Install;
    page->slotWorkerFinished();
    page->m_operate = SingleInstallPage::Uninstall;
    page->setAuthBefore();
    EXPECT_FALSE(page->m_uninstallButton->isVisible());
    EXPECT_FALSE(page->m_backButton->isVisible());
    EXPECT_FALSE(page->m_reinstallButton->isVisible());
    EXPECT_FALSE(page->m_progressFrame->isVisible());
    EXPECT_EQ(0, page->m_progress->value());
    EXPECT_FALSE(page->m_infoControlButton->isVisible());
    EXPECT_TRUE(page->m_upDown);
    page->DealDependResult(3, "test");
    EXPECT_EQ(3, page->dependAuthStatu);
    page->DealDependResult(2, "test");
    EXPECT_EQ(2, page->dependAuthStatu);
    page->DealDependResult(5, "test");
    EXPECT_EQ(5, page->dependAuthStatu);
    page->DealDependResult(4, "test");
    EXPECT_EQ(4, page->dependAuthStatu);
    page->DealDependResult(6, "test");
    EXPECT_EQ(6, page->dependAuthStatu);
    page->DealDependResult(0, "test");
    EXPECT_EQ(0, page->dependAuthStatu);
}

TEST_F(UT_SingleInstallpage, UT_SingleInstallpage_initTabOrder)
{
    Stub stub;
    stub.set(ADDR(Backend, init), stud_singleinit);
    stub.set(ADDR(DebFile, packageName), stud_singlepackageName);
    stub.set(ADDR(DebFile, version), stud_singleversion);
    stub.set(ADDR(DebFile, longDescription), stud_singlelongDescription);
    stub.set(ADDR(DebFile, filePath), stud_singlefilePath);
    stub.set(ADDR(DebFile, architecture), stud_singlearchitecture);
    stub.set(ADDR(DebFile, shortDescription), stud_singleshortDescription);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stud_singlegetPackageDependsStatus);
    stub.set(ADDR(PackagesManager, packageInstallStatus), stud_singlepackageInstallStatus);
    stub.set((decltype(&stud_installPackages))ADDR(DebListModel, slotInstallPackages), stud_installPackages);
    stub.set((decltype(&stud_singleuninstallPackage))ADDR(DebListModel, slotUninstallPackage), stud_singleuninstallPackage);
    stub.set(ADDR(QModelIndex, data), stu_data);
    stub.set(ADDR(DebListModel, recheckPackagePath), stud_recheckPackagePath);
    model = new DebListModel();
    usleep(100 * 1000);
    model->m_packagesManager->m_preparedPackages.append("test");
    model->m_packagesManager->m_preparedPackages.append("test1");
    page = new SingleInstallPage(model);
    usleep(100 * 1000);

    page->m_installButton->setVisible(true);
    page->m_backButton->setVisible(true);
    page->m_uninstallButton->setVisible(true);
    page->initTabOrder();
    EXPECT_FALSE(page->m_doneButton->isVisible());
    EXPECT_FALSE(page->m_uninstallButton->isVisible());
    EXPECT_FALSE(page->m_confirmButton->isVisible());
    EXPECT_FALSE(page->m_installButton->isVisible());
    page->slotHideDependsInfo();
    EXPECT_TRUE(page->m_upDown);
    page->slotShowDependsInfo();
    EXPECT_TRUE(page->m_upDown);
    EXPECT_FALSE(page->m_installProcessView->isVisible());
    EXPECT_FALSE(page->m_itemInfoFrame->isVisible());
}

TEST_F(UT_SingleInstallpage, UT_SingleInstallpage_paintEvent)
{
    Stub stub;
    stub.set(ADDR(Backend, init), stud_singleinit);
    stub.set(ADDR(DebFile, packageName), stud_singlepackageName);
    stub.set(ADDR(DebFile, version), stud_singleversion);
    stub.set(ADDR(DebFile, longDescription), stud_singlelongDescription);
    stub.set(ADDR(DebFile, filePath), stud_singlefilePath);
    stub.set(ADDR(DebFile, architecture), stud_singlearchitecture);
    stub.set(ADDR(DebFile, shortDescription), stud_singleshortDescription);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stud_singlegetPackageDependsStatus);
    stub.set(ADDR(PackagesManager, packageInstallStatus), stud_singlepackageInstallStatus);
    stub.set((decltype(&stud_installPackages))ADDR(DebListModel, slotInstallPackages), stud_installPackages);
    stub.set((decltype(&stud_singleuninstallPackage))ADDR(DebListModel, slotUninstallPackage), stud_singleuninstallPackage);
    stub.set(ADDR(QModelIndex, data), stu_data);
    stub.set(ADDR(DebListModel, recheckPackagePath), stud_recheckPackagePath);
    model = new DebListModel();
    usleep(100 * 1000);
    model->m_packagesManager->m_preparedPackages.append("test");
    model->m_packagesManager->m_preparedPackages.append("test1");
    page = new SingleInstallPage(model);
    usleep(100 * 1000);
    QPaintEvent paint(QRect(page->rect()));
    page->paintEvent(&paint);
}

TEST_F(UT_SingleInstallpage, UT_SingleInstallpage_slotDependPackages)
{
    Stub stub;
    stub.set(ADDR(Backend, init), stud_singleinit);
    stub.set(ADDR(DebFile, packageName), stud_singlepackageName);
    stub.set(ADDR(DebFile, version), stud_singleversion);
    stub.set(ADDR(DebFile, longDescription), stud_singlelongDescription);
    stub.set(ADDR(DebFile, filePath), stud_singlefilePath);
    stub.set(ADDR(DebFile, architecture), stud_singlearchitecture);
    stub.set(ADDR(DebFile, shortDescription), stud_singleshortDescription);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stud_singlegetPackageDependsStatus);
    stub.set(ADDR(PackagesManager, packageInstallStatus), stud_singlepackageInstallStatus);
    stub.set((decltype(&stud_installPackages))ADDR(DebListModel, slotInstallPackages), stud_installPackages);
    stub.set((decltype(&stud_singleuninstallPackage))ADDR(DebListModel, slotUninstallPackage), stud_singleuninstallPackage);
    stub.set(ADDR(QModelIndex, data), stu_data);
    stub.set(ADDR(DebListModel, recheckPackagePath), stud_recheckPackagePath);
    model = new DebListModel();
    usleep(100 * 1000);
    model->m_packagesManager->m_preparedPackages.append("test");
    model->m_packagesManager->m_preparedPackages.append("test1");
    page = new SingleInstallPage(model);
    usleep(100 * 1000);
    //    QMap<QByteArray, QPair<QList<Pkg::DependInfo>, QList<Pkg::DependInfo>>> dependPackages;
    Pkg::DependInfo info;
    info.packageName = "libopencl1";
    info.version = "1.0";
    QList<Pkg::DependInfo> list;
    list.append(info);
    QPair<QList<Pkg::DependInfo>, QList<Pkg::DependInfo>> pair;
    pair.first.append(list);
    pair.second.append(list);
    //    dependPackages.insert("deb", pair);
    page->slotDependPackages(pair, false);
    EXPECT_EQ(1, pair.first.size());
    EXPECT_FALSE(page->m_showDependsButton->isVisible());
}
