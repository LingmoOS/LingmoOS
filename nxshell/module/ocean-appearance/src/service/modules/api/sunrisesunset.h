// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SUNRISESUNSET_H
#define SUNRISESUNSET_H

#include <QDateTime>
//计算日出日落时间
class SunriseSunset
{
public:
    static bool getSunriseSunset(double latitude, double longitude, double utcOffset, const QDate &date, QDateTime &sunrise, QDateTime &sunset);
};

#endif // SUNRISESUNSET_H
