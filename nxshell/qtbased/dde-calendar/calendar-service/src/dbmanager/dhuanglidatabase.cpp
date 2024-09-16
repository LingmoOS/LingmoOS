// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dhuanglidatabase.h"

#include "commondef.h"
#include "units.h"
#include <QProcess>
#include <QTimer>

#include <QDebug>
#include <QSqlError>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRandomGenerator>

const QString HolidayDir = ":/holiday-cn";
const QString HolidayUpdateURLPrefix ="https://cdn-nu-common.uniontech.com/deepin-calendar";
const QString HolidayUpdateDateSetKey = "festivalUpdateDate";

DHuangLiDataBase::DHuangLiDataBase(QObject *parent)
    : DDataBase(parent)
    , m_settings(getAppConfigDir().filePath("config.ini"), QSettings::IniFormat)
{
    qDebug()<< getAppConfigDir().filePath("config.ini");
    QString huangliPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                 QString("dde-calendar/data/huangli.db"),
                                                 QStandardPaths::LocateFile);
    setDBPath(huangliPath);
    qCDebug(ServiceLogger) << "huangli database" << huangliPath;
    setConnectionName("HuangLi");
    dbOpen();
    // 延迟一段时间后，从网络更新节假日
    QTimer::singleShot(QRandomGenerator::global()->bounded(1000, 9999),
                       this,
                       &DHuangLiDataBase::updateFestivalList);
}

// readJSON 会读取一个JSON文件，如果 cache 为 true，则会缓存对象，以供下次使用
QJsonDocument DHuangLiDataBase::readJSON(QString filename, bool cache)
{
    if (cache && readJSONCache.contains(filename)) {
        return readJSONCache.value(filename);
    }
    qCDebug(ServiceLogger) << "read json file" << filename;
    QJsonDocument doc;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(ServiceLogger) << "cannot open json file" << filename;
    } else {
        auto data = file.readAll();
        doc = QJsonDocument::fromJson(data);
    }
    readJSONCache.insert(filename, doc);
    return readJSONCache.value(filename);
}

void DHuangLiDataBase::updateFestivalList()
{
    auto now = QDateTime::currentDateTime();
    auto festivalUpdateDate = now.toString("yyyy-MM-dd");
    // 每天更新一次
    if (m_settings.value(HolidayUpdateDateSetKey, "") == festivalUpdateDate) {
        return;
    }
    m_settings.setValue(HolidayUpdateDateSetKey, festivalUpdateDate);
    // 获取今年和明年的节假日数据
    for (auto i = 0; i < 2; i++) {
        auto year = now.date().year() + i;
        auto filename = getAppCacheDir().filePath(QString("%1.json").arg(year));
        auto url = QString("%1/%2.json").arg(HolidayUpdateURLPrefix).arg(year);
        auto process = DownloadFile(url, filename);
        qCDebug(ServiceLogger) << "Download File" << url << filename;
        connect(process.get(), &QProcess::readyReadStandardError, [process]() {
            qCDebug(ServiceLogger) << "Download Error: " << process->readAllStandardError();
        });
    }
}

// queryFestivalList 查询指定月份的节假日列表
QJsonArray DHuangLiDataBase::queryFestivalList(quint32 year, quint8 month)
{
    qCDebug(ServiceLogger) << "query festival list"
                           << "year" << year << "month" << month;
    QJsonArray dataset;
    auto filename = getAppCacheDir().filePath(QString("%1.json").arg(year));
    if (!QFile(filename).exists()) {
        filename = QString("%1/%2.json").arg(HolidayDir).arg(year);
    }
    qCDebug(ServiceLogger) << "festival file name" << filename;
    auto doc = readJSON(filename, true);
    for (auto val : doc.object().value("days").toArray()) {
        auto day = val.toObject();
        auto name = day.value("name").toString();
        auto date = QDate::fromString(day.value("date").toString(), "yyyy-MM-dd");
        auto isOffday = day.value("isOffDay").toBool();
        if (quint32(date.year()) == year && quint32(date.month()) == month) {
            qCDebug(ServiceLogger) << "festival day" << name << date << isOffday;
            QJsonObject obj;
            obj.insert("name", name);
            obj.insert("date", date.toString("yyyy-MM-dd"));
            obj.insert("status", isOffday ? 1 : 2);
            dataset.append(obj);
        }
    }
    return dataset;
}

QList<stHuangLi> DHuangLiDataBase::queryHuangLiByDays(const QList<stDay> &days)
{
    QList<stHuangLi> infos;
    SqliteQuery query(m_database);
    foreach (stDay d, days) {
        // 查询的id
        qint64 id = QString().asprintf("%d%02d%02d", d.Year, d.Month, d.Day).toInt();
        QString strsql("SELECT id, avoid, suit FROM huangli WHERE id = %1");
        strsql = strsql.arg(id);
        // 数据库中的宜忌信息是从2008年开始的
        stHuangLi sthuangli;
        // 因此这里先将sthuangli内容初始化
        sthuangli.ID = id;
        // 如果数据库中有查询到数据，则进行赋值，如果没有，则使用初始值
        if (query.exec(strsql) && query.next()) {
            sthuangli.ID = query.value("id").toInt();
            sthuangli.Avoid = query.value("avoid").toString();
            sthuangli.Suit = query.value("suit").toString();
        }
        // 将黄历数据放到list中
        infos.append(sthuangli);
    }
    if (query.isActive()) {
        query.finish();
    }
    return infos;
}

void DHuangLiDataBase::initDBData() { }

void DHuangLiDataBase::createDB() { }
