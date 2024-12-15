// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "../deb-installer/manager/PackageDependsStatus.h"

#include "../deb-installer/model/deblistmodel.h"
#include <stub.h>

class ut_packageDependsStatus_Test : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() { big = new PackageDependsStatus(Pkg::DependsStatus::DependsBreak, "packageName"); }
    void TearDown()
    {
        delete m_pds;
        delete big;
    }

    PackageDependsStatus *m_pds = nullptr;
    PackageDependsStatus *big = nullptr;
};

TEST_F(ut_packageDependsStatus_Test, PackageDependsStatus_UT_ok)
{
    ASSERT_EQ(m_pds->ok().status, Pkg::DependsStatus::DependsOk);
}

TEST_F(ut_packageDependsStatus_Test, PackageDependsStatus_UT_available)
{
    ASSERT_EQ(m_pds->available("package").status, Pkg::DependsStatus::DependsAvailable);
}

TEST_F(ut_packageDependsStatus_Test, PackageDependsStatus_UT_isBreak)
{
    m_pds = new PackageDependsStatus(Pkg::DependsStatus::DependsBreak, "packageName");
    ASSERT_TRUE(m_pds->isBreak());
}

TEST_F(ut_packageDependsStatus_Test, PackageDependsStatus_UT_isAvailable)
{
    m_pds = new PackageDependsStatus(Pkg::DependsStatus::DependsAvailable, "packageName");

    ASSERT_TRUE(m_pds->isAvailable());
}

TEST_F(ut_packageDependsStatus_Test, PackageDependsStatus_UT_isAuthCancel)
{
    m_pds = new PackageDependsStatus(Pkg::DependsStatus::DependsAuthCancel, "packageName");
    ASSERT_TRUE(m_pds->isAuthCancel());
}

TEST_F(ut_packageDependsStatus_Test, PackageDependsStatus_UT_min)
{
    m_pds = new PackageDependsStatus(Pkg::DependsStatus::DependsAvailable, "packageName");
    ASSERT_TRUE(m_pds->min(*big).isAvailable());
}

TEST_F(ut_packageDependsStatus_Test, PackageDependsStatus_UT_minEq)
{
    m_pds = new PackageDependsStatus(Pkg::DependsStatus::DependsAvailable, "packageName");
    ASSERT_TRUE(m_pds->minEq(*big).isAvailable());
}

TEST_F(ut_packageDependsStatus_Test, PackageDependsStatus_UT_max)
{
    m_pds = new PackageDependsStatus(Pkg::DependsStatus::DependsAvailable, "packageName");
    ASSERT_TRUE(m_pds->max(*big).isBreak());
}

TEST_F(ut_packageDependsStatus_Test, PackageDependsStatus_UT_maxEq)
{
    m_pds = new PackageDependsStatus(Pkg::DependsStatus::DependsAvailable, "packageName");
    ASSERT_TRUE(m_pds->maxEq(*big).isBreak());
}
