// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "dbus/unit_info.h"

//gtest
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/

class UT_UnitInfo : public ::testing::Test
{
public:
    UT_UnitInfo() : m_tester(nullptr), m_tester1(nullptr), m_tester2(nullptr), m_tester3(nullptr), m_tester4(nullptr)
    , m_tester5(nullptr), m_tester6(nullptr), m_tester7(nullptr), m_tester8(nullptr), m_tester9(nullptr)
    , m_tester10(nullptr), m_tester11(nullptr){}

public:
    virtual void SetUp()
    {
        const QString name("ABC");
        const QString desp("ABC");
        const QString loadState("loadStateTest");
        const QString activeState("activeStateTest");
        const QString testStr("subStateTest");
        const QString testStr1("followedByTest");
        const QString testStr2("unitObjectPathTest");
        const quint32 testInt = 456;
        const QString testStr3("jobTypeTest");
        const QString testStr4("jobObjectPathTest");
        m_tester = new UnitInfo();
        m_tester1 = new UnitInfo(name, "", "", "", "", "", "", 0, "", "");
        m_tester2 = new UnitInfo("", desp, "", "", "", "", "", 0, "", "");
        m_tester3 = new UnitInfo("name", "description", "loadState", "activeState",
                                 "subState", "followedBy", "unitObjectPath", 123, "jobType", "jobObjectPath");
        m_tester4 = new UnitInfo("name", "description", loadState, "activeState",
                                 "subState", "followedBy", "unitObjectPath", 123, "jobType", "jobObjectPath");
        m_tester5 = new UnitInfo("name", "description", "loadState", activeState,
                                 "subState", "followedBy", "unitObjectPath", 123, "jobType", "jobObjectPath");
        m_tester6 = new UnitInfo("name", "description", "loadState", "activeState",
                                   testStr, "followedBy", "unitObjectPath", 123, "jobType", "jobObjectPath");
        m_tester7 = new UnitInfo("name", "description", "loadState", "activeState",
                                   "subState", testStr1, "unitObjectPath", 123, "jobType", "jobObjectPath");
        m_tester8 = new UnitInfo("name", "description", "loadState", "activeState",
                                 "subState", "followedBy", testStr2, 123, "jobType", "jobObjectPath");
        m_tester9 = new UnitInfo("name", "description", "loadState", "activeState",
                                 "subState", "followedBy", "unitObjectPath", testInt, "jobType", "jobObjectPath");
        m_tester10 = new UnitInfo("name", "description", "loadState", "activeState",
                                  "subState", "followedBy", "unitObjectPath", 123, testStr3, "jobObjectPath");
        m_tester11 = new UnitInfo("name", "description", "loadState", "activeState",
                                   "subState", "followedBy", "unitObjectPath", 123, "jobType", testStr4);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
        if (m_tester1) {
            delete m_tester1;
            m_tester1 = nullptr;
        }
        if (m_tester2) {
            delete m_tester2;
            m_tester2 = nullptr;
        }
        if (m_tester3) {
            delete m_tester3;
            m_tester3 = nullptr;
        }
        if (m_tester4) {
            delete m_tester4;
            m_tester4 = nullptr;
        }
        if (m_tester5) {
            delete m_tester5;
            m_tester5 = nullptr;
        }
        if (m_tester6) {
            delete m_tester6;
            m_tester6 = nullptr;
        }
        if (m_tester7) {
            delete m_tester7;
            m_tester7 = nullptr;
        }
        if (m_tester8) {
            delete m_tester8;
            m_tester8 = nullptr;
        }
        if (m_tester9) {
            delete m_tester9;
            m_tester9 = nullptr;
        }
        if (m_tester10) {
            delete m_tester10;
            m_tester10 = nullptr;
        }
        if (m_tester11) {
            delete m_tester11;
            m_tester11 = nullptr;
        }
    }

protected:
    UnitInfo *m_tester;
    UnitInfo *m_tester1;
    UnitInfo *m_tester2;
    UnitInfo *m_tester3;
    UnitInfo *m_tester4;
    UnitInfo *m_tester5;
    UnitInfo *m_tester6;
    UnitInfo *m_tester7;
    UnitInfo *m_tester8;
    UnitInfo *m_tester9;
    UnitInfo *m_tester10;
    UnitInfo *m_tester11;
};

TEST_F(UT_UnitInfo, initTest)
{

}

TEST_F(UT_UnitInfo, test_getName_01)
{
    const QString name("ABC");

    QString retName = m_tester1->getName();

    EXPECT_EQ(name, retName);
}

TEST_F(UT_UnitInfo, test_setName_01)
{
    const QString name("ABC");

    m_tester->setName(name);

    QString retName = m_tester->getName();

    EXPECT_EQ(name, retName);
}

TEST_F(UT_UnitInfo, test_getDescription_01)
{
    const QString desp("ABC");

    QString retDesp = m_tester2->getDescription();

    EXPECT_EQ(desp, retDesp);
}

TEST_F(UT_UnitInfo, test_setDescription_01)
{
    const QString desp("ABC");

    m_tester->setDescription(desp);

    QString retDesp = m_tester->getDescription();

    EXPECT_EQ(desp, retDesp);
}

TEST_F(UT_UnitInfo, test_getLoadState_01)
{
    const QString loadState("loadStateTest");

    QString retLoadState = m_tester4->getLoadState();

    EXPECT_EQ(loadState, retLoadState);
}

TEST_F(UT_UnitInfo, test_setLoadState_01)
{
    const QString loadState("loadStateTest");

    m_tester3->setLoadState(loadState);
    QString retLoadState = m_tester3->getLoadState();

    EXPECT_EQ(loadState, retLoadState);
}

TEST_F(UT_UnitInfo, test_getActiveState_01)
{
    const QString activeState("activeStateTest");

    QString retActiveState = m_tester5->getActiveState();

    EXPECT_EQ(activeState, retActiveState);
}

TEST_F(UT_UnitInfo, test_setActiveState_01)
{
    const QString activeState("activeStateTest");

    m_tester3->setActiveState(activeState);
    QString ret = m_tester3->getActiveState();

    EXPECT_EQ(activeState, ret);
}

TEST_F(UT_UnitInfo, test_getSubState_01)
{
    const QString testStr("subStateTest");

    QString ret = m_tester6->getSubState();

    EXPECT_EQ(testStr, ret);
}

TEST_F(UT_UnitInfo, test_setSubState_01)
{
    const QString testStr("activeStateTest");

    m_tester3->setSubState(testStr);
    QString ret = m_tester3->getSubState();

    EXPECT_EQ(testStr, ret);
}

TEST_F(UT_UnitInfo, test_getFollowedBy_01)
{
    const QString testStr("followedByTest");

    QString ret = m_tester7->getFollowedBy();

    EXPECT_EQ(testStr, ret);
}

TEST_F(UT_UnitInfo, test_setFollowedBy_01)
{
    const QString testStr("followedByTest");

    m_tester3->setFollowedBy(testStr);
    QString ret = m_tester3->getFollowedBy();

    EXPECT_EQ(testStr, ret);
}

TEST_F(UT_UnitInfo, test_getUnitObjectPath_01)
{
    const QString testStr("unitObjectPathTest");

    QString ret = m_tester8->getUnitObjectPath();

    EXPECT_EQ(testStr, ret);
}

TEST_F(UT_UnitInfo, test_setUnitObjectPath_01)
{
    const QString testStr("unitObjectPathTest");

    m_tester3->setUnitObjectPath(testStr);
    QString ret = m_tester3->getUnitObjectPath();

    EXPECT_EQ(testStr, ret);
}

TEST_F(UT_UnitInfo, test_getJobId_01)
{
    const quint32 testInt = 456;

    quint32 ret = m_tester9->getJobId();

    EXPECT_EQ(testInt, ret);
}

TEST_F(UT_UnitInfo, test_setJobId_01)
{
    const quint32 testInt = 456;

    m_tester3->setJobId(testInt);
    quint32 ret = m_tester3->getJobId();

    EXPECT_EQ(testInt, ret);
}

TEST_F(UT_UnitInfo, test_getJobType_01)
{
    const QString testStr("jobTypeTest");

    QString ret = m_tester10->getJobType();

    EXPECT_EQ(testStr, ret);
}

TEST_F(UT_UnitInfo, test_setJobType_01)
{
    const QString testStr("jobTypeTest");

    m_tester3->setJobType(testStr);
    QString ret = m_tester3->getJobType();

    EXPECT_EQ(testStr, ret);
}

TEST_F(UT_UnitInfo, test_getJobObjectPath_01)
{
    const QString testStr("jobObjectPathTest");

    QString ret = m_tester11->getJobObjectPath();

    EXPECT_EQ(testStr, ret);
}

TEST_F(UT_UnitInfo, test_setJobObjectPath_01)
{
    const QString testStr("jobObjectPathTest");

    m_tester3->setJobObjectPath(testStr);
    QString ret = m_tester3->getJobObjectPath();

    EXPECT_EQ(testStr, ret);
}

TEST_F(UT_UnitInfo, test_operator_01)
{
    const UnitInfo rhs;
    m_tester->operator=(rhs);
}

TEST_F(UT_UnitInfo, test_operator_02)
{
    const UnitInfo other;
    m_tester->operator==(other);
}
