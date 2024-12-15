// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

//#include "test_monthwindow.h"

//#include "../third-party_stub/stub.h"
//#include "monthWidget/monthview.h"

//test_monthWindow::test_monthWindow()
//{
//}

//void test_monthWindow::SetUp()
//{
//    m_monthWindow = new CMonthWindow(nullptr);
//}

//void test_monthWindow::TearDown()
//{
//    delete m_monthWindow;
//    m_monthWindow = nullptr;
//}

//TEST_F(test_monthWindow, setTheMe)
//{
//    m_monthWindow->setTheMe(0);
//    m_monthWindow->setTheMe(1);
//    m_monthWindow->setTheMe(2);
//}

//namespace MonthWindow {
//void deleteselectschedule_Stub(void *obj)
//{
//    Q_UNUSED(obj)
//}
//} // namespace MonthWindow

//TEST_F(test_monthWindow, deleteselectSchedule)
//{
//    Stub stub;
//    stub.set(ADDR(CMonthView, deleteSelectSchedule), MonthWindow::deleteselectschedule_Stub);
//    m_monthWindow->deleteselectSchedule();
//}

//TEST_F(test_monthWindow, previousMonth)
//{
//    QDate currentDate = QDate::currentDate();
//    m_monthWindow->setSelectDate(currentDate);
//    m_monthWindow->previousMonth();
//    ASSERT_EQ(m_monthWindow->getSelectDate(), currentDate.addMonths(1));
//}

//TEST_F(test_monthWindow, nextMonth)
//{
//    QDate currentDate = QDate::currentDate();
//    m_monthWindow->setSelectDate(currentDate);
//    m_monthWindow->nextMonth();
//    ASSERT_EQ(m_monthWindow->getSelectDate(), currentDate.addMonths(-1));
//}

//TEST_F(test_monthWindow, slotViewSelectDate)
//{
//    QDate currentDate = QDate::currentDate();
//    m_monthWindow->slotViewSelectDate(currentDate);
//}

//TEST_F(test_monthWindow, resizeEvent)
//{
//    m_monthWindow->setFixedSize(500, 300);
//    m_monthWindow->setFixedSize(600, 400);
//}

//TEST_F(test_monthWindow, slottoday)
//{
//    m_monthWindow->slottoday();
//}
