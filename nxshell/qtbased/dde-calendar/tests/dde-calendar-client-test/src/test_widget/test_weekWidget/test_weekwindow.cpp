// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_weekwindow.h"

#include "../third-party_stub/stub.h"
#include "customWidget/scheduleview.h"

test_weekWindow::test_weekWindow()
{
}

void test_weekWindow::SetUp()
{
    m_weekWindow = new CWeekWindow(nullptr);
}

void test_weekWindow::TearDown()
{
    delete m_weekWindow;
    m_weekWindow = nullptr;
}

TEST_F(test_weekWindow, setTheMe)
{
    m_weekWindow->setTheMe(0);
    m_weekWindow->setTheMe(1);
    m_weekWindow->setTheMe(2);
}

TEST_F(test_weekWindow, updateHeight)
{
    m_weekWindow->updateHeight();
}

TEST_F(test_weekWindow, setTime)
{
    QTime time = QTime::currentTime();
    //定位非全天显示位置
    m_weekWindow->setTime(time);
}

TEST_F(test_weekWindow, setYearData)
{
    m_weekWindow->setYearData();
}

namespace WeekDeleteItem {
void slotDeleteitem_Stub(void *obj)
{
    Q_UNUSED(obj)
}
} // namespace WeekDeleteItem

TEST_F(test_weekWindow, deleteselectSchedule)
{
    Stub stub;
    stub.set(ADDR(CScheduleView, slotDeleteitem), WeekDeleteItem::slotDeleteitem_Stub);
    m_weekWindow->deleteselectSchedule();
}

bool IsDragging_Stub(void *obj)
{
    Q_UNUSED(obj)
    return false;
}

TEST_F(test_weekWindow, slotIsDragging)
{
    Stub stub;
    stub.set(ADDR(CScheduleView, IsDragging), IsDragging_Stub);
    bool isDragging;
    m_weekWindow->slotIsDragging(isDragging);
    ASSERT_EQ(isDragging, false);
}
