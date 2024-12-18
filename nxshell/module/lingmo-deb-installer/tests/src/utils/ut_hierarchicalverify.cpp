// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../deb-installer/utils/hierarchicalverify.h"

#include <QSignalSpy>
#include <QDBusInterface>

#include <stub.h>

#include <gtest/gtest.h>

class ut_HierarchicalVerify_TEST : public ::testing::Test
{
protected:
    void SetUp();
    void TearDown();
};

void ut_HierarchicalVerify_TEST::SetUp()
{
    HierarchicalVerify::instance()->interfaceInvalid = false;
}

void ut_HierarchicalVerify_TEST::TearDown()
{
    HierarchicalVerify::instance()->invalidPackages.clear();
}

// Stub functions.
bool stub_checkHierarchicalInterface_true()
{
    return true;
}

bool stub_dbus_isValid_false()
{
    return false;
}

static bool stub_switchValid = true;
bool stub_checkHierarchicalInterface_switch()
{
    return stub_switchValid;
}

TEST_F(ut_HierarchicalVerify_TEST, isValid_WithInterface_True)
{
    Stub stub;
    stub.set(ADDR(HierarchicalVerify, checkHierarchicalInterface), stub_checkHierarchicalInterface_true);

    ASSERT_TRUE(HierarchicalVerify::instance()->checkValidImpl());
}

TEST_F(ut_HierarchicalVerify_TEST, isValid_NoInterface_False)
{
    HierarchicalVerify::instance()->interfaceInvalid = false;
    ASSERT_FALSE(HierarchicalVerify::instance()->isValid());
}

TEST_F(ut_HierarchicalVerify_TEST, checkTransactionError_TestRegExp_True)
{
    auto hVerify = HierarchicalVerify::instance();
    ASSERT_TRUE(hVerify->checkTransactionError("pkg", "lingmohook65280"));
    ASSERT_TRUE(hVerify->checkTransactionError("pkg", "\r\nlingmodeehook+++65280\n"));
    ASSERT_TRUE(hVerify->checkTransactionError("pkg2", "lingmohookhook65280"));
    ASSERT_TRUE(hVerify->checkTransactionError("pkg2", "Error:lingmo hook exit code 65280"));

    // 1071 调整错误码为 256
    ASSERT_TRUE(hVerify->checkTransactionError("pkg2", "lingmohookhook256"));
    ASSERT_TRUE(hVerify->checkTransactionError("pkg2", "Error:lingmo hook exit code 256"));
    ASSERT_TRUE(hVerify->checkTransactionError("pkg2",
                                               "执行钩子 if test -x /usr/sbin/lingmo-pkg-install-hook;then /usr/sbin/lingmo-pkg "
                                               "install-hook -e hc-verifysign;fi 出错，退出状态为 256"));

    QSet<QString> pkgSet{"pkg", "pkg2"};
    ASSERT_EQ(hVerify->invalidPackages, pkgSet);
}

TEST_F(ut_HierarchicalVerify_TEST, checkTransactionError_TestRegExp_False)
{
    auto hVerify = HierarchicalVerify::instance();
    ASSERT_FALSE(hVerify->checkTransactionError("pkg", ""));
    ASSERT_FALSE(hVerify->checkTransactionError("pkg", "deepihoo65280"));
    ASSERT_FALSE(hVerify->checkTransactionError("pkg", "\r\nlingmo\ndeehook+++65280\n"));
    ASSERT_FALSE(hVerify->checkTransactionError("pkg2", "lingmoh-ookh-ook65280"));
    ASSERT_FALSE(hVerify->checkTransactionError("pkg2", "Error:lingmo hook \n exit code 65280"));

    // 1071 调整错误码为 256
    ASSERT_FALSE(hVerify->checkTransactionError("pkg2", "lingmohookhook25"));
    ASSERT_FALSE(hVerify->checkTransactionError("pkg2", "Error:lingmo hook exit code 255"));
    ASSERT_FALSE(hVerify->checkTransactionError("pkg2",
                                                "执行钩子 if test -x /usr/sbin/lingmo-pkg-install-hook;then /usr/sbin/lingmo-pkg "
                                                "install-hook -e hc-verifysign;fi 出错，退出状态为 \n测试代码"));
    ASSERT_FALSE(hVerify->checkTransactionError("pkg2",
                                                "执行钩子 if test -x /usr/sbin/lingmo-pkg-install-hook;then /usr/sbin/lingmo-pkg "
                                                "install-hook -e hc-verifysign;fi 出错，退出状态为 xxx"));

    ASSERT_TRUE(hVerify->invalidPackages.isEmpty());
}

TEST_F(ut_HierarchicalVerify_TEST, verifyResult_Store_True)
{
    auto hVerify = HierarchicalVerify::instance();
    hVerify->invalidPackages.insert("deb");

    // Passed when not contains invalid package name.
    ASSERT_TRUE(hVerify->pkgVerifyPassed(""));
    ASSERT_FALSE(hVerify->pkgVerifyPassed("deb"));

    hVerify->clearVerifyResult();
    ASSERT_TRUE(hVerify->pkgVerifyPassed("deb"));
}
