// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

// dock position
#define DOCK_TOP        0
#define DOCK_RIGHT      1
#define DOCK_BOTTOM     2
#define DOCK_LEFT       3

#define DOCK_FASHION    0
#define DOCK_EFFICIENT  1

#define ROUND_CORNER    8
#define RECT_MARGINS    1
#define RECT_PENWIDTH   2

#define MONITOR_SERVICE "com.deepin.api.XEventMonitor"

#define DAYS_INWEEK  7
#define WEEKS_NUM  6
#define MONTH_METRIC_NUM  (DAYS_INWEEK * WEEKS_NUM)

#define TURNPAGE_DELTA  120

namespace DDECalendar {
//光标坐标关键字

static const int FontSizeEleven = 11;
static const int FontSizeTwelve = 12;
static const int FontSizeFourteen = 14;
static const int FontSizeSixteen = 16;
static const int FontSizeTwentyfour = 24;
static const int FontSizeTwenty = 20;
static const int FontSizeTen = 10;

};

#endif // CONSTANTS_H
