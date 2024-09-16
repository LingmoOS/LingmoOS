// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HUANGLISERVICE_H
#define HUANGLISERVICE_H

#include "dservicebase.h"
#include "huangliData/dbusdatastruct.h"
#include "calendarhuangli.h"

/**
 * @brief The DHuangliService class      黄历服务
 */
class DHuangliService : public DServiceBase
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.dataserver.HuangLi")
public:
    explicit DHuangliService(QObject *parent = nullptr);
public slots:
    Q_SCRIPTABLE QString getFestivalMonth(quint32 year, quint32 month);
    Q_SCRIPTABLE QString getHuangLiDay(quint32 year, quint32 month, quint32 day);
    Q_SCRIPTABLE QString getHuangLiMonth(quint32 year, quint32 month, bool fill);
    Q_SCRIPTABLE CaLunarDayInfo getLunarInfoBySolar(quint32 year, quint32 month, quint32 day);
    Q_SCRIPTABLE CaLunarMonthInfo getLunarMonthCalendar(quint32 year, quint32 month, bool fill);
private:
    CalendarHuangLi *m_huangli;
};

#endif // HUANGLISERVICE_H
