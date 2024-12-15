// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DHUANGLIDATABASE_H
#define DHUANGLIDATABASE_H

#include "ddatabase.h"
#include "huangliData/lunardatastruct.h"
#include "lunarandfestival/lunarandfestival.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QSettings>

class DHuangLiDataBase : public DDataBase
{
    Q_OBJECT
public:
    explicit DHuangLiDataBase(QObject *parent = nullptr);
    QJsonArray queryFestivalList(quint32 year, quint8 month);
    QList<stHuangLi> queryHuangLiByDays(const QList<stDay> &days);

    void initDBData() override;
private:
    QJsonDocument readJSON(QString filename, bool cache);
    QHash<QString, QJsonDocument> readJSONCache;
    void updateFestivalList();
    QSettings m_settings;
protected:
    //创建数据库
    void createDB() override;
};

#endif // DHUANGLIDATABASE_H
