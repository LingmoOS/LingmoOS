// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

//#include "test_monthdayview.h"
//#include "touchgestureoperation.h"
//#include "../third-party_stub/stub.h"

//#include <QMouseEvent>
//#include <QPainter>

//static int monthdayview_int = 0;

//static int monthdayview_stub_int()
//{
//    return monthdayview_int;
//}

//test_monthdayview::test_monthdayview()
//{
//    mMonthDayView = new CMonthDayView();
//    mMonthWidget = new CMonthWidget();
//    mMonthRect = new CMonthRect();
//    mMonthRect->setDate(QDate());
//    CMonthRect::setSelectRect(mMonthRect);
//}

//test_monthdayview::~test_monthdayview()
//{
//    delete mMonthDayView;
//    mMonthDayView = nullptr;
//    delete mMonthWidget;
//    mMonthWidget = nullptr;
//    delete mMonthRect;
//    mMonthRect = nullptr;
//}

////void CMonthDayView::setSelectDate(const QDate &date)
//TEST_F(test_monthdayview, setSelectdate)
//{
//    mMonthDayView->setSelectDate(QDate::currentDate());
//}

////void CMonthDayView::setTheMe(int type)
//TEST_F(test_monthdayview, setTheMe)
//{
//    mMonthDayView->setTheMe(1);
//    mMonthDayView->setTheMe(2);
//}

////void CMonthDayView::setSearchflag(bool flag)
//TEST_F(test_monthdayview, setSearchflag)
//{
//    mMonthDayView->setSearchflag(false);
//}

//TEST_F(test_monthdayview, event_01)
//{
//    Stub stub;
//    stub.set(ADDR(touchGestureOperation, getTouchState), monthdayview_stub_int);
//    QMouseEvent e(QEvent::MouseButtonPress, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
//    mMonthDayView->event(&e);
//}

////void CMonthWidget::setDate(const QDate date[12])
//TEST_F(test_monthdayview, setDate)
//{
//    QDate m_days[12];
//    QDate currentDate = QDate::currentDate();
//    for (int i = 0; i < 12; i++) {
//        m_days[i] = currentDate.addDays(i);
//    }
//    mMonthWidget->setDate(m_days);
//}

////void CMonthWidget::updateSize()
//TEST_F(test_monthdayview, updateSize)
//{
//    mMonthWidget->updateSize();
//}

////void CMonthRect::setDate(const QDate &date)
//TEST_F(test_monthdayview, setRectDate)
//{
//    mMonthRect->setDate(QDate::currentDate());
//    mMonthRect->getDate();
//}

////void CMonthRect::setRect(const QRectF &rect)
//TEST_F(test_monthdayview, setRect)
//{
//    mMonthRect->setRect(QRectF());
//    mMonthRect->rect();
//}

////void CMonthRect::setDevicePixelRatio(const qreal pixel)
//TEST_F(test_monthdayview, setDevicePixelRatio)
//{
//    mMonthRect->setDevicePixelRatio(1.2);
//}

////void CMonthRect::setTheMe(int type)
//TEST_F(test_monthdayview, setRTheMe)
//{
//    mMonthRect->setTheMe(1);
//    mMonthRect->setTheMe(2);
//}

//TEST_F(test_monthdayview, paintEvent_01)
//{
//    QPaintEvent paint(QRect(0, 0, 1, 1));
//    mMonthWidget->paintEvent(&paint);
//}

//TEST_F(test_monthdayview, mouseReleaseEvent_01)
//{
//    QMouseEvent e(QEvent::MouseButtonPress, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
//    mMonthWidget->mouseReleaseEvent(&e);
//}

//TEST_F(test_monthdayview, keyPressEvent_01)
//{
//    QKeyEvent e(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
//    mMonthWidget->keyPressEvent(&e);
//}

//TEST_F(test_monthdayview, keyPressEvent_02)
//{
//    QKeyEvent e(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
//    mMonthWidget->keyPressEvent(&e);
//}

//TEST_F(test_monthdayview, keyPressEvent_03)
//{
//    QKeyEvent e(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
//    mMonthWidget->keyPressEvent(&e);
//}

//TEST_F(test_monthdayview, focusInEvent_01)
//{
//    QFocusEvent e(QEvent::FocusIn, Qt::TabFocusReason);
//    mMonthWidget->focusInEvent(&e);
//    EXPECT_TRUE(mMonthWidget->m_isFocus);
//}

//TEST_F(test_monthdayview, focusInEvent_02)
//{
//    QFocusEvent e(QEvent::FocusIn, Qt::OtherFocusReason);
//    mMonthWidget->focusInEvent(&e);
//    EXPECT_FALSE(mMonthWidget->m_isFocus);
//}

//TEST_F(test_monthdayview, focusOutEvent_01)
//{
//    QFocusEvent e(QEvent::FocusIn, Qt::OtherFocusReason);
//    mMonthWidget->focusOutEvent(&e);
//    EXPECT_FALSE(mMonthWidget->m_isFocus);
//}

//TEST_F(test_monthdayview, mousePress_01)
//{
//    mMonthWidget->mousePress(QPoint(0, 0));
//    EXPECT_FALSE(mMonthWidget->m_isFocus);
//}

//TEST_F(test_monthdayview, updateSize_01)
//{
//    mMonthWidget->updateSize();
//}

//TEST_F(test_monthdayview, getMousePosItem_01)
//{
//    mMonthWidget->getMousePosItem(QPointF());
//}

//TEST_F(test_monthdayview, updateShowDate_01)
//{
//    mMonthWidget->updateShowDate(QDate());
//}

//TEST_F(test_monthdayview, paintItem_01)
//{
//    mMonthRect->m_Date = QDate(2022, 4, 22);
//    QPainter pa(mMonthWidget);
//    mMonthRect->paintItem(&pa, QRect(), true);
//}

//TEST_F(test_monthdayview, paintItem_02)
//{
//    mMonthRect->m_Date = QDate(2022, 4, 22);
//    mMonthRect->m_SelectRect = mMonthRect;
//    QPainter pa(mMonthWidget);
//    mMonthRect->paintItem(&pa, QRect(), true);
//}

//TEST_F(test_monthdayview, setDevicePixelRatio_01)
//{
//    mMonthRect->setDevicePixelRatio(10);
//    EXPECT_EQ(mMonthRect->m_DevicePixelRatio, 10);
//}

//TEST_F(test_monthdayview, setTheMe_01)
//{
//    mMonthRect->setTheMe(0);
//    EXPECT_EQ(mMonthRect->m_defaultTextColor, Qt::black);
//}

//TEST_F(test_monthdayview, setTheMe_02)
//{
//    mMonthRect->setTheMe(2);
//    EXPECT_EQ(mMonthRect->m_defaultTextColor, QColor("#C0C6D4"));
//}
