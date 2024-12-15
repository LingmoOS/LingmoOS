// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LUNARMANAGER_H
#define LUNARMANAGER_H

#include "huangliData/dbusdatastruct.h"

#include <QObject>

class LunarManager : public QObject
{
    Q_OBJECT
public:
    explicit LunarManager(QObject *parent = nullptr);
    static LunarManager* instace();

    void asyncRequestLunar(const QDate &date);

Q_SIGNALS:
    void lunarInfoReady(const CaHuangLiDayInfo &info);

};
#define gLunarManager LunarManager::instace()
#endif // LUNARMANAGER_H
