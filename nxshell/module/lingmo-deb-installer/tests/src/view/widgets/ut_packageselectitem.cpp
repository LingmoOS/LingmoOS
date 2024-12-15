// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DLabel>
#include <DSpinner>
#include <QCheckBox>
#include <gtest/gtest.h>
#include <stub.h>

#include "../deb-installer/model/packageanalyzer.h"

#define private public
#include "../deb-installer/view/widgets/packageselectitem.h"

QPair<Pkg::PackageInstallStatus, QString> packageInstallStatus_earilier(const DebIr &ir)
{
    Q_UNUSED(ir)
    return {Pkg::InstalledEarlierVersion, "123"};
}

QPair<Pkg::PackageInstallStatus, QString> packageInstallStatus_same(const DebIr &ir)
{
    Q_UNUSED(ir)
    return {Pkg::InstalledSameVersion, "321"};
}

QPair<Pkg::PackageInstallStatus, QString> packageInstallStatus_later(const DebIr &ir)
{
    Q_UNUSED(ir)
    return {Pkg::InstalledLaterVersion, "123321"};
}

QPair<Pkg::PackageInstallStatus, QString> packageInstallStatus_none(const DebIr &ir)
{
    Q_UNUSED(ir)
    return {Pkg::NotInstalled, "1234567"};
}

class ut_packageselectitem_TEST : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() { w = new PackageSelectItem; }
    void TearDown() { delete w; }

private:
    PackageSelectItem *w;
};

TEST_F(ut_packageselectitem_TEST, PackageSelectItem_UT_setDebIRTest_0)
{
    Stub stub;
    stub.set(ADDR(PackageAnalyzer, packageInstallStatus), packageInstallStatus_earilier);

    DebIr ir;
    ir.archMatched = true;
    w->setDebIR(ir);
    ASSERT_EQ(w->checkBox->isChecked(), true);
}

TEST_F(ut_packageselectitem_TEST, PackageSelectItem_UT_setDebIRTest_1)
{
    Stub stub;
    stub.set(ADDR(PackageAnalyzer, packageInstallStatus), packageInstallStatus_same);

    DebIr ir;
    ir.archMatched = true;
    w->setDebIR(ir);
    ASSERT_EQ(w->checkBox->isChecked(), false);
}

TEST_F(ut_packageselectitem_TEST, PackageSelectItem_UT_setDebIRTest_2)
{
    Stub stub;
    stub.set(ADDR(PackageAnalyzer, packageInstallStatus), packageInstallStatus_later);

    DebIr ir;
    ir.archMatched = true;
    w->setDebIR(ir);
    ASSERT_EQ(w->checkBox->isChecked(), false);
}

TEST_F(ut_packageselectitem_TEST, PackageSelectItem_UT_setDebIRTest_4)
{
    Stub stub;
    stub.set(ADDR(PackageAnalyzer, packageInstallStatus), packageInstallStatus_none);

    DebIr ir;
    ir.archMatched = true;
    w->setDebIR(ir);
    ASSERT_EQ(w->checkBox->isChecked(), true);
}

TEST_F(ut_packageselectitem_TEST, PackageSelectItem_UT_setDebIRTest_5)
{
    DebIr ir;
    ir.archMatched = false;
    w->setDebIR(ir);
    ASSERT_EQ(w->checkBox->isChecked(), false);
    ASSERT_EQ(w->checkBox->isEnabled(), false);
}

TEST_F(ut_packageselectitem_TEST, PackageSelectItem_UT_checkEnableTest)
{
    w->setChecked(false);
    ASSERT_EQ(w->checkBox->isChecked(), false);

    w->checkBox->setChecked(true);
    ASSERT_EQ(w->isChecked(), true);

    w->checkBox->setEnabled(false);
    ASSERT_EQ(w->isEnabled(), false);

    w->checkBox->setEnabled(true);
    ASSERT_EQ(w->isEnabled(), true);
}
