// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_yearwindow.h"
#include "customWidget/customframe.h"
#include "widget/yearWidget/yearscheduleview.h"
#include "calendarglobalenv.h"
#include "constants.h"

test_yearwindow::test_yearwindow()
{
    CalendarGlobalEnv::getGlobalEnv()->registerKey(DDECalendar::CursorPointKey, QPoint(20, 20));
    mYearWindow = new CYearWindow();
    dateaManger = new CalendarDateDataManager();
}

test_yearwindow::~test_yearwindow()
{
    delete mYearWindow;
    mYearWindow = nullptr;
    delete dateaManger;
    dateaManger = nullptr;
}

//void CYearWindow::setTheMe(int type)
TEST_F(test_yearwindow, setTheMe)
{
    int type = 1;
    mYearWindow->setTheMe(type);
    type = 2;
    mYearWindow->setTheMe(type);
}

TEST_F(test_yearwindow, slotMousePress)
{
    CalendarGlobalEnv::getGlobalEnv()->reviseValue(DDECalendar::CursorPointKey, QPoint(20, 20));
    QDate currentDate = QDate::currentDate();
    mYearWindow->slotMousePress(currentDate, 0);
    ASSERT_EQ(mYearWindow->getSelectDate(), currentDate);
    mYearWindow->slotMousePress(currentDate, 1);
    mYearWindow->slotMousePress(currentDate, 2);
    mYearWindow->slotMousePress(currentDate, 3);
    mYearWindow->slotMousePress(currentDate, 4);
}

TEST_F(test_yearwindow, calculateAzimuthAngle)
{
    QPointF startPoint(0, 0);
    QPointF stopPoint(30, 20);
    mYearWindow->calculateAzimuthAngle(startPoint, stopPoint);
    stopPoint.setX(20);
    stopPoint.setY(30);
    mYearWindow->calculateAzimuthAngle(startPoint, stopPoint);
    stopPoint.setX(-20);
    stopPoint.setY(30);
    mYearWindow->calculateAzimuthAngle(startPoint, stopPoint);
    stopPoint.setX(-30);
    stopPoint.setY(20);
    mYearWindow->calculateAzimuthAngle(startPoint, stopPoint);
}
