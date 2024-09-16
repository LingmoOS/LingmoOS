// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../deb-installer/view/pages/debinstaller.h"
#include "../deb-installer/model/deblistmodel.h"
#include "../deb-installer/view/pages/multipleinstallpage.h"
#include "../deb-installer/view/pages/singleinstallpage.h"
#include "../deb-installer/view/widgets/filechoosewidget.h"
#include "../deb-installer/view/widgets/choosefilebutton.h"
#include "../deb-installer/manager/packagesmanager.h"
#include "../deb-installer/view/widgets/infocontrolbutton.h"

#include <stub.h>
#include <ut_Head.h>

#include <DTitlebar>

#include <QDebug>
#include <QAction>
#include <QFileInfo>
#include <QtConcurrent>
#include <QFuture>
#include <QUrl>

#include <gtest/gtest.h>

using namespace QApt;

void stud_removePackage(const int idx)
{
    Q_UNUSED(idx);
}

void stud_installDebs() {}

bool stud_checkSuffix(QString filePath)
{
    Q_UNUSED(filePath);
    return true;
}

void stud_appendPackage(QStringList package)
{
    Q_UNUSED(package);
}

void stud_refreshModel() {}

void stud_uninstallCurrentPackage() {}

QString stud_packageName()
{
    return QString("test.deb");
};

QString stud_version()
{
    return QString("1.0.0");
};

QString stud_longDescription()
{
    return QString("This is test");
};

const QList<QString> stud_preparedPackages()
{
    return QList<QString>{"test.deb"};
}

void stud_afterGetAutherFalse() {}

void stud_setEnableButton(bool bEnable)
{
    Q_UNUSED(bEnable);
}

void stud_DealDependResult(int iAuthRes, QString dependName)
{
    Q_UNUSED(iAuthRes);
    Q_UNUSED(dependName);
}

bool stud_isFile()
{
    return true;
}

bool stud_run()
{
    return true;
}

bool stud_reloadCache()
{
    return true;
}

void stud_reset() {}

void stud_appendNoThread(QStringList packages, int allPackageSize)
{
    Q_UNUSED(packages);
    Q_UNUSED(allPackageSize);
}

class UT_Debinstaller : public UT_HEAD
{
public:
    // 添加日志
    static void SetUpTestCase() { qDebug() << "SetUpTestCase" << endl; }
    static void TearDownTestCase() { qDebug() << "TearDownTestCase" << endl; }
    void SetUp()  // TEST跑之前会执行SetUp
    {
        Stub stub;
        stub.set(ADDR(Backend, init), stud_run);
        stub.set(ADDR(PackagesManager, appendNoThread), stud_appendNoThread);
        deb = new DebInstaller();
        debListModel = qobject_cast<DebListModel *>(deb->m_fileListModel);

        usleep(1000 * 1000);
        qDebug() << "SetUp" << endl;
    }
    void TearDown()  // TEST跑完之后会执行TearDown
    {
        delete deb;
    }
    DebInstaller *deb;
    DebListModel *debListModel;
};
void stub_enableCloseAndExit()
{
    return;
}

TEST_F(UT_Debinstaller, UT_Debinstaller_total)
{
    deb->slotShowInvalidePackageMessage();
    deb->slotShowPkgExistMessage();
    deb->slotShowNotLocalPackageMessage();
    deb->slotShowPkgRemovedMessage("00");
    EXPECT_EQ(deb->backToSinglePage(), nullptr);
}

TEST_F(UT_Debinstaller, UT_Debinstaller_refreshSingle)
{
    deb->refreshSingle();
    EXPECT_EQ(2, deb->m_dragflag);
    EXPECT_EQ(2, deb->m_Filterflag);
    EXPECT_TRUE(!deb->m_lastPage.isNull());
}

TEST_F(UT_Debinstaller, UT_Debinstaller_slotEnableCloseButton)
{
    deb->slotEnableCloseButton(false);
    EXPECT_TRUE(deb->titlebar()->quitMenuIsDisabled());
    deb->slotEnableCloseButton(true);
    EXPECT_FALSE(deb->titlebar()->quitMenuIsDisabled());
    deb->refreshMulti();
    EXPECT_EQ(1, deb->m_dragflag);
}

TEST_F(UT_Debinstaller, UT_Debinstaller_slotSetAuthingStatus)
{
    bool auth = false;
    deb->slotSetAuthingStatus(auth);
    EXPECT_FALSE(auth);
}

TEST_F(UT_Debinstaller, UT_Debinstaller_single2Multi)
{
    deb->single2Multi();
    EXPECT_TRUE(debListModel->m_packageOperateStatus.isEmpty());
    EXPECT_EQ(1, deb->m_Filterflag);
    EXPECT_EQ(1, deb->m_dragflag);
}

TEST_F(UT_Debinstaller, UT_Debinstaller_slotChangeDragFlag)
{
    deb->slotChangeDragFlag();
    EXPECT_EQ(0, deb->m_dragflag);
}

TEST_F(UT_Debinstaller, UT_Debinstaller_slotDealDependResult)
{
    deb->slotDealDependResult(DebListModel::AuthDependsSuccess, "test");
    EXPECT_TRUE(debListModel->m_packageOperateStatus.isEmpty());
    deb->slotDealDependResult(DebListModel::AuthBefore, "test");
}

TEST_F(UT_Debinstaller, UT_Debinstaller_slotSetEnableButton)
{
    deb->m_dragflag = 2;
    deb->slotSetEnableButton(true);
    deb->slotSetEnableButton(false);
    deb->slotShowHiddenButton();
    EXPECT_FALSE(deb->m_packageAppending);
    EXPECT_EQ(2, deb->m_dragflag);
    deb->m_dragflag = 1;
    deb->slotSetEnableButton(true);
    deb->slotSetEnableButton(false);
    deb->slotShowHiddenButton();
    EXPECT_FALSE(deb->m_packageAppending);
    EXPECT_EQ(1, deb->m_dragflag);
    EXPECT_TRUE(debListModel->m_packageOperateStatus.isEmpty());
    deb->m_packageAppending = true;
    deb->slotSetEnableButton(true);
    EXPECT_TRUE(deb->m_packageAppending);
}

TEST_F(UT_Debinstaller, UT_Debinstaller_appendPackageStart)
{
    deb->appendPackageStart();
    EXPECT_TRUE(deb->m_packageAppending);
    deb->appendFinished();
    EXPECT_FALSE(deb->m_packageAppending);
}

TEST_F(UT_Debinstaller, UT_Debinstaller_slotPackagesSelected)
{
    deb->m_fileListModel->setWorkerStatus(DebListModel::WorkerProcessing);
    deb->slotPackagesSelected(QStringList() << "test.deb"
                                            << "test1.deb");
    EXPECT_EQ(DebListModel::WorkerProcessing, deb->m_fileListModel->getWorkerStatus());
}

TEST_F(UT_Debinstaller, UT_Debinstaller_slotPackagesSelected_01)
{
    deb->m_fileListModel->setWorkerStatus(DebListModel::WorkerFinished);
    deb->slotPackagesSelected(QStringList() << "test.deb");
    EXPECT_EQ(DebListModel::WorkerFinished, deb->m_fileListModel->getWorkerStatus());
}

TEST_F(UT_Debinstaller, UT_Debinstaller_slotShowUninstallConfirmPage)
{
    deb->slotShowUninstallConfirmPage();
    EXPECT_EQ(DebListModel::WorkerUnInstall, deb->m_fileListModel->getWorkerStatus());
    EXPECT_EQ(3, deb->m_Filterflag);
}

TEST_F(UT_Debinstaller, UT_Debinstaller_slotUninstallAccepted)
{
    deb->m_dragflag = 1;
    deb->slotUninstallAccepted();
    EXPECT_TRUE(deb->m_fileChooseWidget->acceptDrops());
    EXPECT_EQ(-1, deb->m_Filterflag) << deb->m_Filterflag;
}

TEST_F(UT_Debinstaller, UT_Debinstaller_slotUninstallCancel)
{
    deb->m_dragflag = 1;
    deb->slotUninstallCancel();
    EXPECT_EQ(DebListModel::WorkerPrepare, deb->m_fileListModel->getWorkerStatus());
    EXPECT_EQ(1, deb->m_Filterflag);
}

TEST_F(UT_Debinstaller, UT_Debinstaller_slotReset)
{
    Stub stub;
    stub.set(ADDR(DebInstaller, checkSuffix), stud_checkSuffix);
    stub.set(ADDR(DebListModel, slotAppendPackage), stud_appendPackage);
    stub.set(ADDR(DebListModel, preparedPackages), stud_preparedPackages);
    stub.set(ADDR(DebListModel, removePackage), stud_removePackage);
    stub.set(ADDR(DebListModel, reset), stud_reset);
    stub.set(ADDR(DebListModel, initPrepareStatus), stud_reset);
    stub.set(ADDR(DebListModel, installDebs), stud_installDebs);
    stub.set(ADDR(MultipleInstallPage, refreshModel), stud_refreshModel);
    stub.set(ADDR(MultipleInstallPage, setEnableButton), stud_setEnableButton);
    stub.set(ADDR(MultipleInstallPage, DealDependResult), stud_DealDependResult);
    stub.set(ADDR(MultipleInstallPage, afterGetAutherFalse), stud_afterGetAutherFalse);
    stub.set(ADDR(SingleInstallPage, slotUninstallCurrentPackage), stud_uninstallCurrentPackage);
    stub.set(ADDR(SingleInstallPage, afterGetAutherFalse), stud_afterGetAutherFalse);
    stub.set(ADDR(SingleInstallPage, setEnableButton), stud_setEnableButton);
    stub.set(ADDR(SingleInstallPage, DealDependResult), stud_DealDependResult);
    stub.set(ADDR(DebFile, packageName), stud_packageName);
    stub.set(ADDR(DebFile, version), stud_version);
    stub.set(ADDR(DebFile, longDescription), stud_longDescription);
    stub.set(ADDR(Backend, reloadCache), stud_reloadCache);
    deb->slotReset();
    EXPECT_EQ(-1, deb->m_dragflag);
    EXPECT_EQ(-1, deb->m_Filterflag);
    EXPECT_EQ(DebListModel::WorkerPrepare, deb->m_fileListModel->m_workerStatus);
    EXPECT_TRUE(deb->m_fileChooseWidget->acceptDrops());
    EXPECT_FALSE(deb->m_fileChooseWidget->m_chooseFileBtn->hasFocus());
}

TEST_F(UT_Debinstaller, UT_Debinstaller_checkSuffix)
{
    Stub stub;
    stub.set(ADDR(QFileInfo, isFile), stud_isFile);
    EXPECT_EQ(deb->checkSuffix("test.deb"), true);
    EXPECT_EQ(deb->checkSuffix("test"), false);
}

TEST_F(UT_Debinstaller, UT_Debinstaller_keyPressEvent)
{
    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QCoreApplication::sendEvent(deb, &keyPressEvent);
}

bool ut_hasUrls()
{
    return true;
}

TEST_F(UT_Debinstaller, UT_Debinstaller_dragEnterEvent)
{
    QMimeData *mimeData = new QMimeData;
    Stub stub;
    stub.set(ADDR(QMimeData, hasUrls), ut_hasUrls);
    QDragEnterEvent enterEvent(QPoint(0, 0), Qt::MoveAction, mimeData, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(deb, &enterEvent);
    EXPECT_TRUE(deb->m_fileChooseWidget->acceptDrops());
    delete mimeData;
}
TEST_F(UT_Debinstaller, UT_Debinstaller_dragMoveEvent)
{
    QMimeData *mimeData = new QMimeData;
    QDragMoveEvent enterEvent(QPoint(0, 0), Qt::MoveAction, mimeData, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(deb, &enterEvent);
    delete mimeData;
}

TEST_F(UT_Debinstaller, UT_Debinstaller_dropEvent)
{
    QByteArray csvData = "test";
    QMimeData *mimeData = new QMimeData;
    mimeData->setData("text/csv", csvData);
    QDropEvent dropEvent(QPoint(0, 0), Qt::MoveAction, mimeData, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(deb, &dropEvent);
    delete mimeData;
}

TEST_F(UT_Debinstaller, UT_Debinstaller_closeEvent)
{
    QByteArray csvData = "test";
    QMimeData *mimeData = new QMimeData;
    mimeData->setData("text/csv", csvData);
    QCloseEvent closeEvent;
    QCoreApplication::sendEvent(deb, &closeEvent);
    delete mimeData;
}
