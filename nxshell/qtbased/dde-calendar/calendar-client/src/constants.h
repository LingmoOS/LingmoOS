// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

namespace DDECalendar {
//光标坐标关键字
const QString   CursorPointKey = "CursorPoint";
static const int HeaderItemWidth = 80;
static const int HeaderItemHeight = 40;

static const int CellWidth = 106;
static const int CellHeight = 70;

static const int CellHighlightWidth = 80;
static const int CellHighlightHeight = 70;

static const int QueryEarliestYear = 1900;
static const int QueryLatestYear = 2100;

static const int FestivalTypeID = 4;

static const int FontSizeEleven = 11;
static const int FontSizeTwelve = 12;
static const int FontSizeFourteen = 14;
static const int FontSizeSixteen = 16;
static const int FontSizeTwentyfour = 24;
static const int FontSizeTwenty = 20;
static const int FontSizeTen = 10;
static const int FontSizeOneHundred = 100;

static const int CalendarYearWindow = 0;
static const int CalendarMonthWindow = 1;
static const int CalendarWeekWindow = 2;
static const int CalendarDayWindow = 3;

static const int ThirtyMinutesWithSec = 1800;

static const int AtTimeOfEvent = 0;
static const int OnStartDay = 0;
static const int FifteenMinutesBefore = 15;
static const int ThirtyMinutesBefore = 30;
static const int OneHourBefore = 60;
static const int OneDayBeforeWithDay = 1;
static const int OneDayBeforeWithMinutes = 1440;
static const int TwoDayBeforeWithDay = 2;
static const int TwoDayBeforeWithMinutes = 2880;
static const int OneWeekBeforeWithDay = 7;
static const int OneWeekBeforeWithMinutes = 10080;

static const int NewScheduleLabelWidth = 70;

static const int QuarterOfAnhourWithMinute = 15;
} // namespace DDECalendar
namespace DDEYearCalendar {

static const int RectSizeOfEveryMonth = 42;
static const int FrameSizeOfEveryYear = 12;

static const int YearScheduleListMaxcount = 5;
static const int YHeaderItemWidth = 20;
static const int YHeaderItemHeight = 16;

static const int YCellWidth = 26;
static const int YCellHeight = 20;

static const int YCellHighlightWidth = 26;
static const int YCellHighlightHeight = 20;
static const int Y_MLabelHeight = 36;
static const int Y_YLabelHeight = 45;

static const int Y_MWindowWidth = 202;
static const int Y_MWindowHeight = 159;
} // namespace DDEYearCalendar
namespace DDEMonthCalendar {
static const int ItemSizeOfMonthDay = 42;

static const int AFewDaysOfWeek = 7;
static const int LinesNumOfMonth = 6;

static const int MonthNumOfYear = 12;

static const int MHeaderItemWidth = 60;
static const int MHeaderItemHeight = 33;
static const int MEventsItemHeight = 36;

static const int MCellWidth = 120;
static const int MCellHeight = 74;

static const int MCellHighlightWidth = 120;
static const int MCellHighlightHeight = 74;

static const int MWeekCellWidth = 120;
static const int MWeekCellHeight = 54;

static const int MDayCellWidth = 36;
static const int MDayCellHeight = 36;

static const int M_YTopHeight = 66;
static const int M_YLabelHeight = 36;
static const int M_YLunaLabelWindth = 66;
static const int M_YLunaLabelHeight = 20;

static const int MTodayWindth = 100;
static const int MTodayHeight = 36;

static const int M_MWindowWidth = 202;
static const int M_MWindowHeight = 162;
}
namespace DDEWeekCalendar {
static const int FirstDayOfWeekend = 6;
static const int AFewDaysofWeek = 7;

static const int NumWeeksDisplayed = 10;

static const int WCellHeaderItemWidth = 109;
static const int WCellHeaderItemHeight = 54;

static const int WMCellHeaderWidth = 80;

static const int MCellHighlightWidth = 120;
static const int MCellHighlightHeight = 74;

static const int WWeekCellWidth = 36;
static const int WWeekCellHeight = 36;

static const int W_YLabelHeight = 36;
static const int W_YLunatLabelWindth = 66;
static const int W_YLunatLabelHeight = 20;

static const int WTodayWindth = 100;
static const int WTodayHeight = 36;

static const int W_MWindowWidth = 202;
static const int W_MWindowHeight = 162;
}
namespace DDEDayCalendar {
static const int PainterCellNum = 42;

static const int DCellWidth = 35;
static const int DCellHeight = 28;

static const int DCellHighlightWidth = 33;
static const int DCellHighlightHeight = 26;
static const int D_MLabelHeight = 36;
static const int D_YLabelHeight = 36;
static const int DDLabelHeight = 117;
static const int DWLabelHeight = 22;
static const int DHuangLiInfoLabelHeight = 17;
static const int DHuangLiLabelHeight = 56;
static const int DHuangLiLabelMaxHeight = 96;
static const int DHuangLiLabelWidth = 330;
static const int D_MWindowWidth = 350;
static const int D_MWindowHeight = 564;
}

namespace DARROWRECT {
//提示框圆角
static const int DRADIUS = 10;
} // namespace DARROWRECT
enum Week {
    Monday    = 6,
    Tuesday   = 5,
    Wednesday = 4,
    Thursday  = 3,
    Friday    = 2,
    Saturday  = 1,
    Sunday    = 0,
};

#endif // CONSTANTS_H
