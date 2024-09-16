// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "application.h"
#include "stub.h"

#include <QObject>
#include <QTest>

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

class Ut_application : public QObject, public::testing::Test
{
public:
    Ut_application() {}
};

TEST(UT_Application_setCurrentTaskState,UT_Application_setCurrentTaskState_001)
{
    int argc = 1;
    char *argv[2] = {"one", "two"};
    Application *pApplication = new Application(argc,argv);

    Application::TaskState pTaskState = Application::kTaskStarted;
    pApplication->setCurrentTaskState(pTaskState);
    EXPECT_EQ(pApplication->m_currentTaskStat,pTaskState);

    pApplication->deleteLater();
}

TEST(UT_Application_getCurrentTaskState,UT_Application_getCurrentTaskState_001)
{
    int argc = 1;
    char *argv[2] = {"one", "two"};
    Application *pApplication = new Application(argc,argv);

    EXPECT_EQ(pApplication->getCurrentTaskState(),pApplication->m_currentTaskStat);

    pApplication->deleteLater();
}

TEST(UT_Application_event,UT_Application_event_001)
{
    int argc = 1;
    char *argv[2] = {"one", "two"};
    Application *pApplication = new Application(argc,argv);

    EXPECT_TRUE(kMonitorStartEventType);
    pApplication->deleteLater();
}

TEST(UT_Application_event,UT_Application_event_002)
{
    int argc = 1;
    char *argv[2] = {"one", "two"};
    Application *pApplication = new Application(argc,argv);

    EXPECT_TRUE(kNetifStartEventType);
    pApplication->deleteLater();
}


TEST(UT_Application_event,UT_Application_event_003)
{
    int argc = 1;
    char *argv[2] = {"one", "two"};
    Application *pApplication = new Application(argc,argv);
//    pApplication->raiseWindow();
    EXPECT_TRUE(kNetifStartEventType);
    pApplication->deleteLater();
}

TEST(UT_Application_event,UT_Application_event_004)
{
    QEvent::Type kOtherEventType = static_cast<QEvent::Type>(QEvent::User + 1);

    int argc = 1;
    char *argv[2] = {"one", "two"};
    Application *pApplication = new Application(argc,argv);
//    pApplication->raiseWindow();
    EXPECT_TRUE(kOtherEventType);
    pApplication->deleteLater();
}


class UT_MonitorStartEvent: public ::testing::Test
{
public:
    UT_MonitorStartEvent() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new MonitorStartEvent();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    MonitorStartEvent *m_tester;
};

TEST_F(UT_MonitorStartEvent, initTest)
{
}

