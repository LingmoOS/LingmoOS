// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "notification-center/notifycenterwidget.h"
#include "iconbutton.h"
#undef private
#include "mockpersistence.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace std;

class UT_NotifyCenterWidget : public testing::Test
{
public:
    void SetUp() override
    {
        PersistenceHelper helper;

        EXPECT_CALL(persistence, getAllNotify()).
                    WillRepeatedly(testing::Invoke(&helper, &PersistenceHelper::getAllNotify));

        EXPECT_CALL(persistence, getAll()).
                    WillRepeatedly(testing::Invoke(&helper, &PersistenceHelper::getAll));

        EXPECT_CALL(persistence, addOne(testing::_)).
                    WillRepeatedly(testing::Invoke(&helper, &PersistenceHelper::addOne));

        EXPECT_CALL(persistence, addAll(testing::_)).
                    WillRepeatedly(testing::Invoke(&helper, &PersistenceHelper::addAll));

        EXPECT_CALL(persistence, removeOne(testing::_)).
                    WillRepeatedly(testing::Invoke(&helper, &PersistenceHelper::removeOne));

        EXPECT_CALL(persistence, removeApp(testing::_)).
                    WillRepeatedly(testing::Invoke(&helper, &PersistenceHelper::removeApp));

        EXPECT_CALL(persistence, recordCount()).
                    WillRepeatedly(testing::Invoke(&helper, &PersistenceHelper::getRecordCount));

        EXPECT_CALL(persistence, getById(testing::_)).
                    WillRepeatedly(testing::Invoke(&helper, &PersistenceHelper::getById));

        EXPECT_CALL(persistence, getFrom(testing::_, testing::_)).
                    WillRepeatedly(testing::Invoke(&helper, &PersistenceHelper::getFrom));

        obj = new NotifyCenterWidget(&persistence);
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
    }

public:
    MockPersistence persistence;
    NotifyCenterWidget *obj = nullptr;
};

TEST_F(UT_NotifyCenterWidget, coverageTest)
{
    QRect displayRect(0, 0, 1920, 1080);
    QRect dockRect(0, 0, 1920, 48);
    obj->updateGeometry(displayRect, dockRect, OSD::Bottom, 0);
    EXPECT_EQ(QRect(1520, 10, 400, 1002), obj->geometry());
    obj->updateGeometry(displayRect, dockRect, OSD::Top, 0);
    EXPECT_EQ(QRect(1510, 68, 400, 1002), obj->geometry());
    obj->updateGeometry(displayRect, dockRect, OSD::Bottom, 1);
    EXPECT_EQ(QRect(1510, 10, 400, 1012), obj->geometry());
    obj->updateGeometry(displayRect, dockRect, OSD::Top, 1);
    EXPECT_EQ(QRect(1510, 58, 400, 1012), obj->geometry());

    dockRect = QRect(0, 0, 48, 1080);
    obj->updateGeometry(displayRect, dockRect, OSD::Right, 0);
    EXPECT_EQ(QRect(1452, 10, 400, 1060), obj->geometry());
    obj->updateGeometry(displayRect, dockRect, OSD::Left, 0);
    EXPECT_EQ(QRect(1510, 10, 400, 1060), obj->geometry());
    obj->updateGeometry(displayRect, dockRect, OSD::Right, 1);
    EXPECT_EQ(QRect(1462, 10, 400, 1060), obj->geometry());
    obj->updateGeometry(displayRect, dockRect, OSD::Left, 1);
    EXPECT_EQ(QRect(1510, 10, 400, 1060), obj->geometry());

    obj->showWidget();
    obj->onlyHideWidget();
    obj->onlyShowWidget();
    Q_EMIT obj->m_clearButton->clicked();
    Q_EMIT obj->m_notifyWidget->focusOnButton();
}
