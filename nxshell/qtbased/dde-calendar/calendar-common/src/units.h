// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef UNITS_H
#define UNITS_H

#include <QString>
#include <QDateTime>
#include <QMap>
#include <QDir>
#include <QProcess>
#include <QSharedPointer>

const QString serviceBaseName = "com.deepin.dataserver.Calendar";
const QString serviceBasePath = "/com/deepin/dataserver/Calendar";
#define accountServiceInterface "com.deepin.dataserver.Calendar.Account"

static const QMap<QString, QString> GTypeColor = {
    {"0cecca8a-291b-46e2-bb92-63a527b77d46", "#FF5E97"},
    {"10af78a1-3c25-4744-91db-6fbe5e88083b", "#FF9436"},
    {"263d6c79-32b6-4b00-bf0d-741e50a9550f", "#FFDC00"},
    {"35e70047-98bb-49b9-8ad8-02d1c942f5d0", "#5BDD80"},
    {"406fc0df-87ce-4b3f-b1bc-65d89d791dbc", "#00B99B"},
    {"5bf13e88-e99f-4975-80a8-149fe0a315e3", "#4293FF"},
    {"6cfd1459-1085-47e9-8ca6-379d47ec319a", "#5D51FF"},
    {"70080e96-e68d-40af-9cca-2f41021f6142", "#A950FF"},
    {"8ac5c8bb-55ce-4264-8b0a-5d32116cf983", "#717171"}};

QString dtToString(const QDateTime &dt);
QDateTime dtFromString(const QString &st);

QString dateToString(const QDate &date);
QDate dateFromString(const QString &date);

//获取新数据库路径
QString getDBPath();

//是否为中文环境
bool isChineseEnv();

//获取家配置目录
QString getHomeConfigPath();
QDir getAppConfigDir();
QDir getAppCacheDir();
QSharedPointer<QProcess> DownloadFile(QString url, QString filename);

//时间转换
QDateTime dtConvert(const QDateTime &datetime);

//是否在显示时间范围内1900-2100
bool withinTimeFrame(const QDate &date);

//TODO:获取系统版本(专业版，社区版等)


#endif // UNITS_H
