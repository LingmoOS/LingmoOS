// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_monthbrefwidget.h"

#include <QResizeEvent>
#include <QApplication>
#include <QMouseEvent>
#include <QTest>

test_monthbrefwidget::test_monthbrefwidget()
{

}

test_monthbrefwidget::~test_monthbrefwidget()
{

}

void test_monthbrefwidget::SetUp()
{
    mMonthBrefWidget = new MonthBrefWidget();
    mMonthDayRect = new CMonthDayRect();
}

void test_monthbrefwidget::TearDown()
{
    delete mMonthBrefWidget;
    mMonthBrefWidget = nullptr;
    delete mMonthDayRect;
    mMonthDayRect = nullptr;
}

QVector<QDate> getDateList()
{
    QDate currentdate = QDate::currentDate();
    QVector<QDate> dateList;
    for (int i = 0; i < 42; i++) {
        dateList.append(currentdate.addDays(i));
    }
    return dateList;
}

QVector<bool> getBoolList()
{
    QVector<bool> lineList;
    for (int i = 0; i < 42; i++) {
        if (i % 2 == 0)
            lineList.append(true);
        else
            lineList.append(false);
    }
    return lineList;
}

//void MonthBrefWidget::setDate(const int showMonth, const QVector<QDate> &showDate)
TEST_F(test_monthbrefwidget, setDate)
{
    const int month = 4;
    mMonthBrefWidget->setDate(month, getDateList());
}

//void MonthBrefWidget::setTheMe(int type)
TEST_F(test_monthbrefwidget, setTheMe)
{
    mMonthBrefWidget->setTheMe(1);
    mMonthBrefWidget->setTheMe(2);
}

//void MonthBrefWidget::setLintFlag(const QVector<bool> &lineFlag)
TEST_F(test_monthbrefwidget, setLintFlag)
{
    mMonthBrefWidget->setLintFlag(getBoolList());
}

//void MonthBrefWidget::setSearchScheduleFlag(const QVector<bool> &searchFlag)
TEST_F(test_monthbrefwidget, setSearchScheduleFlag)
{
    mMonthBrefWidget->setSearchScheduleFlag(getBoolList());
}

//void MonthBrefWidget::updateSize()
TEST_F(test_monthbrefwidget, updateSize)
{
    mMonthBrefWidget->updateSize();
}

//int MonthBrefWidget::getMousePosItem(const QPointF &pos)
TEST_F(test_monthbrefwidget, getMousePosItem)
{
    QPointF pointf;
    mMonthBrefWidget->getMousePosItem(pointf);
}

//void MonthBrefWidget::mousePress(const QPoint &point
TEST_F(test_monthbrefwidget, mousePress)
{
    QPoint point;
    mMonthBrefWidget->mousePress(point);
}

//void CMonthDayRect::setTheMe(int type)
TEST_F(test_monthbrefwidget, settheme)
{
    mMonthDayRect->setTheMe(1);
    mMonthDayRect->setTheMe(2);
}

//void CMonthDayRect::setDate(const QDate &date)
TEST_F(test_monthbrefwidget, setdate)
{
    mMonthDayRect->setDate(QDate::currentDate());
}

//QDate CMonthDayRect::getDate() const
TEST_F(test_monthbrefwidget, getdate)
{
    mMonthDayRect->setDate(QDate::currentDate());
    mMonthDayRect->getDate();
}

//void CMonthDayRect::setCellEvent(const CMonthDayRect::CellEventType &type)
TEST_F(test_monthbrefwidget, setCellEvent)
{
    mMonthDayRect->setCellEvent(CMonthDayRect::CellEventType::Cellhover);
}

//void CMonthDayRect::setIsCurrentMonth(const bool isCurrMonth)
TEST_F(test_monthbrefwidget, setIsCurrentMonth)
{
    mMonthDayRect->setIsCurrentMonth(true);
}

//QRectF CMonthDayRect::rect() const
TEST_F(test_monthbrefwidget, rect)
{
    mMonthDayRect->setRect(QRect(10, 10, 8, 8));
    mMonthDayRect->rect();
}

//void CMonthDayRect::setRect(const QRectF &rect)
TEST_F(test_monthbrefwidget, setRect)
{
    mMonthDayRect->setRect(QRect(10, 10, 8, 8));
}

//void CMonthDayRect::setRect(qreal x, qreal y, qreal w, qreal h)
TEST_F(test_monthbrefwidget, setrect)
{
//    mMonthDayRect->setRect(8, 8, 8, 8);
}

//void CMonthDayRect::setLineFlag(const bool flag)
TEST_F(test_monthbrefwidget, setLIneflag)
{
    mMonthDayRect->setLineFlag(true);
}

//void CMonthDayRect::setSearchScheduleFlag(const bool flag)
TEST_F(test_monthbrefwidget, setSearchScheduleflag)
{
    mMonthDayRect->setSearchScheduleFlag(true);
}

//void CMonthDayRect::setDevicePixelRatio(const qreal pixel)
TEST_F(test_monthbrefwidget, setDevicePixelRatio)
{
    mMonthDayRect->setDevicePixelRatio(2);
}

//void CMonthDayRect::setCurrentRect(CMonthDayRect *currrect)
TEST_F(test_monthbrefwidget, setCurrentrect)
{

    mMonthDayRect->setCurrentRect(mMonthDayRect);
}

//void CMonthDayRect::setSystemActiveColor(const QColor &activeColor)
TEST_F(test_monthbrefwidget, setSystemActiveColor)
{
    mMonthDayRect->setSystemActiveColor(Qt::red);
}

//getPixmap
TEST_F(test_monthbrefwidget, getPixmap)
{
    mMonthBrefWidget->setFixedSize(800, 500);
    mMonthBrefWidget->setDate(QDate::currentDate().month(), getDateList());
    QPixmap pixmap(mMonthBrefWidget->size());
    mMonthBrefWidget->render(&pixmap);
}

TEST_F(test_monthbrefwidget, resizeEvent)
{
    mMonthBrefWidget->setFixedSize(QSize(600, 500));
    QResizeEvent resizeEvent(QSize(600, 501), QSize(600, 500));
    QApplication::sendEvent(mMonthBrefWidget, &resizeEvent);
}

TEST_F(test_monthbrefwidget, mouseEvent)
{
    mMonthBrefWidget->setFixedSize(QSize(600, 500));
    mMonthBrefWidget->setDate(QDate::currentDate().month(), getDateList());
    QTest::mouseDClick(mMonthBrefWidget, Qt::MouseButton::LeftButton);
    QTest::mouseRelease(mMonthBrefWidget, Qt::MouseButton::LeftButton);
}

//mouseDoubleClickEvent
TEST_F(test_monthbrefwidget, mouseDoubleClickEvent)
{
    mMonthBrefWidget->setFixedSize(QSize(600, 500));
    mMonthBrefWidget->setDate(QDate::currentDate().month(), getDateList());
    QTest::mouseDClick(mMonthBrefWidget, Qt::MouseButton::LeftButton);
}
