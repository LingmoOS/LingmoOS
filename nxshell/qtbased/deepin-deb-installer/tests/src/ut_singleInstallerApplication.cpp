// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../deb-installer/singleInstallerApplication.h"
#include "../deb-installer/utils/utils.h"
#include "../deb-installer/view/pages/debinstaller.h"
#include "../deb-installer/utils/hierarchicalverify.h"

#include <stub.h>
#include <ut_Head.h>

#include <QDebug>
#include <QCommandLineParser>
#include <QScopedPointer>

#include <gtest/gtest.h>

void stud_singleaAppProcess(const QCoreApplication &app)
{
    Q_UNUSED(app);
}

bool stub_SingleInstallerApplication_HierarchicalVerify_Invalid()
{
    return false;
}

class SingleInstallerApplication_UT : public UT_HEAD
{
public:
    // 添加日志
    static void SetUpTestCase() { qDebug() << "SetUpTestCase" << endl; }
    static void TearDownTestCase() { qDebug() << "TearDownTestCase" << endl; }
    void SetUp()  // TEST跑之前会执行SetUp
    {
        int a = 1;
        int &argc = a;
        char argv[] = "abc";
        char argv1[] = "123";
        char *pArgv[] = {argv, argv1};
        singleInstaller = new SingleInstallerApplication(argc, pArgv);
        usleep(100 * 1000);
        qDebug() << "SetUp" << endl;
    }
    void TearDown()  // TEST跑完之后会执行TearDown
    {
    }
    SingleInstallerApplication *singleInstaller;
};

TEST_F(SingleInstallerApplication_UT, UT_SingleInstallerApplication_parseCmdLine)
{
    Stub stub;
    stub.set((void(QCommandLineParser::*)(const QCoreApplication &))ADDR(QCommandLineParser, process), stud_singleaAppProcess);
    singleInstaller->m_selectedFiles.append(" ");
    EXPECT_TRUE(singleInstaller->parseCmdLine());
    QStringList debPathList;
    singleInstaller->InstallerDeb(debPathList);
    EXPECT_EQ(0, debPathList.size());
    debPathList.append("test.deb");
    singleInstaller->InstallerDeb(debPathList);
    EXPECT_EQ(1, debPathList.size());
}

TEST_F(SingleInstallerApplication_UT, UT_SingleInstallerApplication_activateWindow)
{
    singleInstaller->m_selectedFiles.append(" ");
    singleInstaller->activateWindow();

    EXPECT_TRUE(singleInstaller->m_qspMainWnd.get()->isVisible());
}

TEST_F(SingleInstallerApplication_UT, UT_SingleInstallerApplication_checkInstallStatus)
{
    singleInstaller->activateWindow();
    EXPECT_EQ(0, singleInstaller->checkInstallStatus("test.db"));
}
TEST_F(SingleInstallerApplication_UT, UT_SingleInstallerApplication_checkDependsStatus)
{
    singleInstaller->activateWindow();
    EXPECT_EQ(2, singleInstaller->checkDependsStatus("test.db"));
}

TEST_F(SingleInstallerApplication_UT, UT_SingleInstallerApplication_checkDigitalSignature)
{
    Stub stub;
    stub.set(ADDR(HierarchicalVerify, isValid), stub_SingleInstallerApplication_HierarchicalVerify_Invalid);

    singleInstaller->activateWindow();
    EXPECT_EQ(0, singleInstaller->checkDigitalSignature("test.db"));
}

TEST_F(SingleInstallerApplication_UT, UT_SingleInstallerApplication_getPackageInfo)
{
    singleInstaller->activateWindow();
    EXPECT_EQ(QString(""), singleInstaller->getPackageInfo("test.db"));
}
TEST_F(SingleInstallerApplication_UT, UT_SingleInstallerApplication_InstallerDebPackge)
{
    singleInstaller->activateWindow();
    EXPECT_EQ(QString("You can only install local deb packages"), singleInstaller->InstallerDebPackge("test.db"));
}
TEST_F(SingleInstallerApplication_UT, UT_SingleInstallerApplication_unInstallDebPackge)
{
    singleInstaller->activateWindow();
    EXPECT_EQ(QString("currentdeb not install, uninstall package faild"), singleInstaller->unInstallDebPackge("test.db"));
}
