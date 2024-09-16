// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "units.h"
#include <QDir>
#include <QProcess>

#include <QTimeZone>
#include <QStandardPaths>
#include <QLocale>
#include <QSharedPointer>

QString dtToString(const QDateTime &dt)
{
    QTime _offsetTime = QTime(0, 0).addSecs(dt.timeZone().offsetFromUtc(dt));
    return QString("%1+%2").arg(dt.toString("yyyy-MM-ddThh:mm:ss")).arg(_offsetTime.toString("hh:mm"));
}

QDateTime dtConvert(const QDateTime &datetime)
{
    QDateTime dt = datetime;
    dt.setOffsetFromUtc(dt.offsetFromUtc());
    return dt;
}

QDateTime dtFromString(const QString &st)
{
    QDateTime &&dtSt = QDateTime::fromString(st, Qt::ISODate);
    //转换为本地时区
    return QDateTime(dtSt.date(),dtSt.time());
}

QString getDBPath()
{
    return getHomeConfigPath().append("/deepin/dde-calendar-service");
}

QDate dateFromString(const QString &date)
{
    return QDate::fromString(date, Qt::ISODate);
}

QString dateToString(const QDate &date)
{
    return date.toString("yyyy-MM-dd");
}

bool isChineseEnv()
{
    return QLocale::system().name().startsWith("zh_");
}

QString getHomeConfigPath()
{
    //根据环境变量获取config目录
    QString configPath = QString(qgetenv("XDG_CONFIG_HOME"));
    if(configPath.trimmed().isEmpty()) {
        configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    }
    return configPath;
}

QDir getAppConfigDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
}

QDir getAppCacheDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
}

QSharedPointer<QProcess> DownloadFile(QString url, QString filename)
{
    auto process = QSharedPointer<QProcess>::create();
    process->setEnvironment({"LANGUAGE=en"});
    process->start("wget", { "-c", "-N", "-O", filename, url });
    return process;
}

bool withinTimeFrame(const QDate &date)
{
    return date.isValid() && (date.year() >= 1900 && date.year() <=2100);
}

