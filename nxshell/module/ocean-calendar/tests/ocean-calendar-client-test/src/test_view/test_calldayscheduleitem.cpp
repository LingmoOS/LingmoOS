// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_calldayscheduleitem.h"

#include "../testscheduledata.h"

#include <QPainter>

//test_CAlldayscheduleitem::test_CAlldayscheduleitem()
//{
//}

//void test_CAlldayscheduleitem::SetUp()
//{
//    m_rectF.setRect(0, 0, 200, 50);
//    m_allItem = new CAllDayScheduleItem(m_rectF);
//}

//void test_CAlldayscheduleitem::TearDown()
//{
//    delete m_allItem;
//    m_allItem = nullptr;
//}

////paintBackground
//TEST_F(test_CAlldayscheduleitem, paintBackground)
//{
//    m_allItem->setData(TestDataInfo::getScheduleItemDInfo().first());
//    QPixmap pixmap(m_rectF.toRect().size());
//    pixmap.fill(Qt::transparent);
//    QPainter painter(&pixmap);
//    m_allItem->setPressSchedule(TestDataInfo::getScheduleItemDInfo().first());
//    m_allItem->m_vSelectflag = true;
//    m_allItem->paintBackground(&painter, pixmap.rect(), true);
//    m_allItem->m_vHoverflag = true;
//    m_allItem->m_vSelectflag = false;
//    m_allItem->paintBackground(&painter, pixmap.rect(), true);
//}

////hasSelectSchedule
//TEST_F(test_CAlldayscheduleitem, hasSelectSchedule)
//{
//    m_allItem->setData(TestDataInfo::getScheduleItemDInfo().first());

//    bool hasSelectInfo = m_allItem->hasSelectSchedule(TestDataInfo::getScheduleItemDInfo().first());
//    ASSERT_TRUE(hasSelectInfo);
//}
