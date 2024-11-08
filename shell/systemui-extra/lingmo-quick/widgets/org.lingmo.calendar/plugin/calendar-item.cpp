/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: youdiansaodongxi <guojiaqi@kylinos.cn>
 *
 */

#include "calendar-item.h"
#include "lunar-calendar-info.h"

class CalendarItemPrivate
{
public:
    QDate m_date;
    QString m_lunar;
};

CalendarItem::CalendarItem(QDate date, QObject *parent) : QObject(parent), d(new CalendarItemPrivate)
{
    d->m_date = date;
    d->m_lunar = LunarCalendarInfo::Instance()->getLunarDay(date.year(), date.month(), date.day());
}

QDate CalendarItem::getItemDate()
{
    return d->m_date;
}

QString CalendarItem::getItemLunar()
{
    return d->m_lunar;
}
