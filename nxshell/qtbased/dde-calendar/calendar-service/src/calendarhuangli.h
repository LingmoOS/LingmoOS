// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CALENDARHUANGLI_H
#define CALENDARHUANGLI_H

#include "dhuanglidatabase.h"
#include "huangliData/dbusdatastruct.h"
#include <QJsonArray>

#include <QObject>

class DHuangLiDataBase;

class CalendarHuangLi : public QObject
{
    Q_OBJECT
public:
    explicit CalendarHuangLi(QObject *parent = nullptr);

    QJsonArray getFestivalMonth(quint32 year, quint32 month);
    QString getHuangLiDay(quint32 year, quint32 month, quint32 day);
    QString getHuangLiMonth(quint32 year, quint32 month, bool fill);
    CaLunarDayInfo getLunarInfoBySolar(quint32 year, quint32 month, quint32 day);
    CaLunarMonthInfo getLunarCalendarMonth(quint32 year, quint32 month, bool fill);

private:
    DHuangLiDataBase *m_database;
};

#endif // CALENDARHUANGLI_H
