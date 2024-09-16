// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "calculate_util.h"
#undef private

#include <QTest>

#include <DGuiApplicationHelper>

#include <gtest/gtest.h>

DGUI_USE_NAMESPACE

class Tst_calculate : public testing::Test
{
};

TEST_F(Tst_calculate, calendarSelectIcon_test)
{
    CalculateUtil::instance()->setFullScreen(false);
    CalculateUtil::instance()->appIconSize();
    CalculateUtil::instance()->getAppBoxSize();
    CalculateUtil::instance()->getScreenScaleX();
    CalculateUtil::instance()->getScreenSize();

    QDate::currentDate().setDate(2000, 13, 0);
    QStringList iconList = CalculateUtil::instance()->calendarSelectIcon();
    QVERIFY2((iconList.size() == 4), "iconList size isn't equal to 4...");
    QVERIFY2(iconList.at(0).compare(":/icons/skin/icons/calendar_bg.svg"), "calendar background icon is abnormal...");

    int month_num = QDate::currentDate().month();
    switch(month_num)
    {
    case 1 ... 12:
        QVERIFY2(iconList.at(1).compare(QString(":/icons/skin/icons/calendar_month/month%1.svg").arg(month_num)), "month num is abnormal...");
        break;
    default:
        QVERIFY2(iconList.at(1).compare(QString(":/icons/skin/icons/calendar_month/month4.svg")), "default month num is abnormal...");
        break;
    }

    int day_num = QDate::currentDate().day();
    switch(day_num)
    {
    case 1 ... 31:
        QVERIFY2(iconList.at(2).compare(QString(":/icons/skin/icons/calendar_day/day%1.svg").arg(day_num)), "day num is abnormal...");
        break;
    default:
        QVERIFY2(iconList.at(2).compare(QString(":/icons/skin/icons/calendar_day/day23.svg")), "default day num is abnormal...");
        break;
    }

    int week_num = QDate::currentDate().dayOfWeek();
    switch(week_num)
    {
    case 1 ... 7:
        QVERIFY2(iconList.at(3).compare(QString(":/icons/skin/icons/calendar_week/week%1.svg").arg(week_num)), "week num is abnormal...");
        break;
    default:
        QVERIFY2(iconList.at(3).compare(QString(":/icons/skin/icons/calendar_week/week4.svg")), "default week num is abnormal...");
        break;
    }
}

TEST_F(Tst_calculate, size_test)
{
    int columnCount = CalculateUtil::instance()->appColumnCount();
    QVERIFY(columnCount == 7);

    CalculateUtil::instance()->setCurrentCategory(5);
    QVERIFY(CalculateUtil::instance()->currentCategory() != 4);

    CalculateUtil::instance()->setSearchWidgetSizeHint(QSize(40, 40));
    QVERIFY(!CalculateUtil::instance()->getSearchWidgetSizeHint().isNull());

    CalculateUtil::instance()->setNavigationWidgetSizeHint(QSize(40, 40));
    QVERIFY(!CalculateUtil::instance()->getNavigationWidgetSizeHint().isNull());
}

TEST_F(Tst_calculate, appIconSize_test)
{
    CalculateUtil::instance()->setFullScreen(true);
    CalculateUtil::instance()->appIconSize();

    CalculateUtil::instance()->setFullScreen(false);
    CalculateUtil::instance()->appIconSize();

    QVERIFY(CalculateUtil::instance()->increaseIconSize());
    QVERIFY(CalculateUtil::instance()->decreaseIconSize());
}

