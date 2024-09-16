// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_yearview.h"

#include "../third-party_stub/stub.h"
#include <QTest>

test_yearview::test_yearview()
{
}

test_yearview::~test_yearview()
{
}

void test_yearview::SetUp()
{
    cYearView = new CYearView();
}

void test_yearview::TearDown()
{
    delete cYearView;
    cYearView = nullptr;
}

QVector<QDate> getListDate()
{
    QVector<QDate> listDate {};
    QDate date = QDate::currentDate();
    for (int i = 0; i < 42; i++) {
        listDate.append(date);
        date = date.addDays(1);
    }
    return listDate;
}

//void CYearView::slotDoubleClickDate(const QDate &date)
TEST_F(test_yearview, slotDoubltClickDate)
{
    cYearView->slotDoubleClickDate(QDate(2021, 1, 6));
}

//void CYearView::slotPressClickDate(const QDate &date)
TEST_F(test_yearview, slotPressClickDate)
{
    cYearView->slotPressClickDate(QDate(2021, 1, 6));
}

//void CYearView::setTheMe(int type)
TEST_F(test_yearview, setTheMe)
{
    cYearView->setTheMe(1);
    cYearView->setTheMe(2);
}

//void CYearView::setShowDate(const QDate &showMonth, const QVector<QDate> &showDate)
TEST_F(test_yearview, setShowDate)
{
    cYearView->setShowDate(getListDate().first(), getListDate());
}

//void CYearView::setHasScheduleFlag(const QVector<bool> &hasScheduleFlag)
TEST_F(test_yearview, setHasScheduleFlag)
{
    QVector<bool> listLintFlag {};
    listLintFlag.append(false);
    listLintFlag.append(true);

    cYearView->setHasScheduleFlag(listLintFlag);
}

//void CYearView::setHasSearchScheduleFlag(const QVector<bool> &hasSearchScheduleFlag)
TEST_F(test_yearview, setHasSearchScheduleFlag)
{
    QVector<bool> listLintFlag {};
    listLintFlag.append(false);
    listLintFlag.append(true);

    cYearView->setHasSearchScheduleFlag(listLintFlag);
}

void setDate_Stub(const int showMonth, const QVector<QDate> &showDate) {
    Q_UNUSED(showMonth)
        Q_UNUSED(showDate)}

//bool CYearView::getStartAndStopDate(QDate &startDate, QDate &stopDate)
TEST_F(test_yearview, getStartAndStopDate)
{
    QDate startDate(QDate(2021, 1, 7));
    QDate stopDate(QDate(2021, 1, 8));

    cYearView->setShowDate(getListDate().first(), getListDate());
    bool result = cYearView->getStartAndStopDate(startDate, stopDate);
    EXPECT_TRUE(result);

    QVector<QDate> listDate {};
    Stub stub;
    stub.set(ADDR(MonthBrefWidget, setDate), setDate_Stub);
    cYearView->setShowDate(getListDate().first(), listDate);
    bool result_false = cYearView->getStartAndStopDate(startDate, stopDate);
    EXPECT_FALSE(result_false);
}

bool hasFocus_Stub()
{
    return true;
}

TEST_F(test_yearview, paintEvent)
{
    Stub stub;
    stub.set(ADDR(QWidget, hasFocus), hasFocus_Stub);
    cYearView->setFixedSize(600, 600);
    QPixmap pixmap(cYearView->size());
    cYearView->render(&pixmap);
}

TEST_F(test_yearview, guitest)
{
    QWidget *currentMonth = cYearView->findChild<QWidget *>("currentMouth");
    if (currentMonth) {
        QTest::mouseDClick(currentMonth, Qt::LeftButton);
    }
}
