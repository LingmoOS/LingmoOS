// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LUNARMANAGER_H
#define LUNARMANAGER_H

#include "lunardbusinterface.h"
#include "huangliData/dbusdatastruct.h"
#include "huangliData/lunardatastruct.h"

#include <QObject>

class LunarManager : public QObject
{
    Q_OBJECT
public:
    explicit LunarManager(QObject *parent = nullptr);
    static LunarManager* instace();
    bool huangLiDay(quint32 year, quint32 month, quint32 day, CaHuangLiDayInfo &info);
    bool huangLiDay(const QDate &date, CaHuangLiDayInfo &out);
    CaHuangLiDayInfo huangLiDay(const QDate &date);

private:
    LunarDBusInterface* m_dbusInter;                   //dbus请求实例
    QMap<QDate, CaHuangLiDayInfo> m_lunarInfoMap;      //缓存的农历数据

};
#define gLunarManager LunarManager::instace()
#endif // LUNARMANAGER_H
