// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QApt/Package>
#include <QApt/Backend>
#include <stub.h>

#define private public
#include "../deb-installer/model/packageanalyzer.h"

class ut_packageanalyzer_TEST : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() {}
    void TearDown() {}

private:
    PackageAnalyzer *w;
};

TEST_F(ut_packageanalyzer_TEST, PackageAnalyzer_UT_supportArch)
{
    ASSERT_EQ(PackageAnalyzer::instance().supportArch("any"), true);
    ASSERT_EQ(PackageAnalyzer::instance().supportArch("all"), true);
}

TEST_F(ut_packageanalyzer_TEST, PackageAnalyzer_UT_packageInstallStatus)
{
    DebIr ir;
    ir.isValid = true;
    ir.packageName = "deepin-hd;o3h8dhoewl";
    ir.architecture = "amd64";

    ir.version = "1.0";
    auto data = PackageAnalyzer::instance().packageInstallStatus(ir);
    ASSERT_EQ(data.first, PackageAnalyzer::NotInstalled);

    ir.packageName = "libc6";
    data = PackageAnalyzer::instance().packageInstallStatus(ir);
    ASSERT_EQ(data.first, PackageAnalyzer::InstalledLaterVersion);

    ir.version = "999.999";
    data = PackageAnalyzer::instance().packageInstallStatus(ir);
    ASSERT_EQ(data.first, PackageAnalyzer::InstalledEarlierVersion);
}

TEST_F(ut_packageanalyzer_TEST, PackageAnalyzer_UT_packageWithArch)
{
    auto package = PackageAnalyzer::instance().packageWithArch("libc6", "deepin-hd;o3h8dhoewl", "");
    ASSERT_NE(package, nullptr);
}

TEST_F(ut_packageanalyzer_TEST, PackageAnalyzer_UT_virtualPackageIsExist)
{
    auto result = PackageAnalyzer::instance().virtualPackageIsExist("deepin-hd;o3h8dhoewl");
    ASSERT_EQ(result, false);
}

TEST_F(ut_packageanalyzer_TEST, PackageAnalyzer_UT_versionMatched)
{
    bool result;

    result = PackageAnalyzer::instance().versionMatched("1.0", "1.0", QApt::LessOrEqual);
    ASSERT_EQ(result, true);

    result = PackageAnalyzer::instance().versionMatched("1.5", "1.0", QApt::GreaterOrEqual);
    ASSERT_EQ(result, false);

    result = PackageAnalyzer::instance().versionMatched("1.0", "1.5", QApt::GreaterThan);
    ASSERT_EQ(result, true);

    result = PackageAnalyzer::instance().versionMatched("1.0", "1.0", QApt::NotEqual);
    ASSERT_EQ(result, false);

    result = PackageAnalyzer::instance().versionMatched("1.0", "1.1", QApt::NotEqual);
    ASSERT_EQ(result, true);
}
