// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "../deb-installer/manager/DealDependThread.h"
#include "../deb-installer/model/deblistmodel.h"

#include <stub.h>
#include <QProcess>
#include <QIODevice>

class ut_dealDependThread_Test : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() { m_dThread = new DealDependThread(); }
    void TearDown() { delete m_dThread; }

    DealDependThread *m_dThread = nullptr;
    Stub stub;
};

TEST_F(ut_dealDependThread_Test, DealDependThread_UT_setDependsList)
{
    QStringList dependsList;
    dependsList << "package1"
                << "package";
    m_dThread->setDependsList(dependsList, 0);

    EXPECT_EQ(m_dThread->m_dependsList.size(), 2);
    EXPECT_EQ(0, m_dThread->m_index);
}

TEST_F(ut_dealDependThread_Test, DealDependThread_UT_setBrokenDepend)
{
    m_dThread->setBrokenDepend("package");

    ASSERT_STREQ(m_dThread->m_brokenDepend.toLocal8Bit(), "package");
}

void proc_start(const QString &program, const QStringList &arguments, QIODevice::OpenModeFlag mode)
{
    Q_UNUSED(program);
    Q_UNUSED(arguments);
    Q_UNUSED(mode);
    return;
}

TEST_F(ut_dealDependThread_Test, DealDependThread_UT_start)
{
    QStringList dependsList;
    dependsList << "package1"
                << "package";
    m_dThread->setDependsList(dependsList, 1);
    m_dThread->setBrokenDepend("package");
    m_dThread->run();
    EXPECT_EQ(m_dThread->m_brokenDepend.toLocal8Bit(), "package");
    EXPECT_EQ(1, m_dThread->m_index);
}

TEST_F(ut_dealDependThread_Test, DealDependThread_UT_onFinished)
{
    m_dThread->bDependsStatusErr = false;
    m_dThread->slotInstallFinished(-1);
    EXPECT_FALSE(m_dThread->bDependsStatusErr);

    m_dThread->bDependsStatusErr = true;
    m_dThread->slotInstallFinished(2);
    EXPECT_FALSE(m_dThread->bDependsStatusErr);
}

TEST_F(ut_dealDependThread_Test, DealDependThread_UT_finished)
{
    emit m_dThread->proc->finished(0);

    ASSERT_FALSE(m_dThread->bDependsStatusErr);
}

QByteArray readAllStandardOutput_success()
{
    return "Not authorized";
}

TEST_F(ut_dealDependThread_Test, DealDependThread_UT_on_readoutput)
{
    stub.set(ADDR(QProcess, readAllStandardOutput), readAllStandardOutput_success);
    m_dThread->slotReadOutput();
    EXPECT_TRUE(m_dThread->bDependsStatusErr);
}

TEST_F(ut_dealDependThread_Test, slotReadOutput_Empty_Pass)
{
    m_dThread->slotReadOutput();
    EXPECT_FALSE(m_dThread->bDependsStatusErr);
    EXPECT_FALSE(m_dThread->bVerifyStatusErr);
}

QByteArray stub_readAllStandardOutput_Hierarchical_Failed()
{
    return "deepin hook exit code: 65280 ";
}

TEST_F(ut_dealDependThread_Test, slotReadOutput_HierarchicalVerify_Failed)
{
    stub.set(ADDR(QProcess, readAllStandardOutput), stub_readAllStandardOutput_Hierarchical_Failed);
    m_dThread->slotReadOutput();
    EXPECT_TRUE(m_dThread->bVerifyStatusErr);
}

QByteArray stub_readAllStandardOutput_Hierarchical_Pass()
{
    return "deepin hook exit code: 65277 ";
}

TEST_F(ut_dealDependThread_Test, slotReadOutput_HierarchicalVerify_Pass)
{
    stub.set(ADDR(QProcess, readAllStandardOutput), stub_readAllStandardOutput_Hierarchical_Pass);
    m_dThread->slotReadOutput();
    EXPECT_FALSE(m_dThread->bVerifyStatusErr);
}
