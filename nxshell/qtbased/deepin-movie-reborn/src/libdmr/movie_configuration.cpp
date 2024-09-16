// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"
#include "movie_configuration.h"
#include "utils.h"

#include <QtSql>
#include <atomic>

namespace dmr {
static std::atomic<MovieConfiguration *> _instance { nullptr };
static QMutex _instLock;

#define CHECKED_EXEC(q) do { \
    if (!(q).exec()) { \
        qCritical() << (q).lastError(); \
    } \
} while (0)

// storage as a database:
// table 1: urls
// url md5 timestamp
// md5 is local file's md5, if url is networked, md5 == 0
// table 2: infos (stores info about every url)
// url key value
class MovieConfigurationBackend: public QObject
{
public:
    explicit MovieConfigurationBackend(MovieConfiguration *cfg): QObject(cfg)
    {
        auto db_dir = QString("%1/%2/%3")
                      .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
                      .arg(qApp->organizationName())
                      .arg(qApp->applicationName());

        QDir d;
        d.mkpath(db_dir);

        auto db_path = QString("%1/movies.db").arg(db_dir);
        _db = QSqlDatabase::addDatabase("QSQLITE");
        _db.setDatabaseName(db_path);
        if(!_db.open()) {
            qCritical() << "open the movies database error";
            return;
        }

        auto ts = _db.tables(QSql::Tables);
        if (!ts.contains("urls") || !ts.contains("infos")) {
            QSqlQuery q(_db);
            if (!q.exec("create table if not exists urls (url TEXT primary key, "
                        "md5 TEXT, timestamp DATETIME)")) {
                qCritical() << q.lastError();
            }

            if (!q.exec("create table if not exists infos (url TEXT, "
                        "key TEXT, value BLOB, primary key (url, key))")) {
                qCritical() << q.lastError();
            }
        }
    }

    void deleteUrl(const QUrl &url)
    {
        if(_db.transaction()) {
            QSqlQuery q(_db);
            if(q.prepare("delete from infos where url = ?")) {
                q.addBindValue(url);
                if (!q.exec()) {
                    if(!_db.commit()) {
                        qCritical() << _db.lastError();
                    }
                    return;
                }
            }

            if (q.numRowsAffected() > 0) {
                QSqlQuery q_l(_db);
                if(q_l.prepare("delete from urls where url = ?")) {
                    q_l.addBindValue(url);
                    CHECKED_EXEC(q_l);
                }
            }
        }
    }

    bool urlExists(const QUrl &url)
    {
        QSqlQuery q(_db);
        if(q.prepare("select url from urls where url = ? limit 1")) {
            q.addBindValue(url);
            CHECKED_EXEC(q);
        }

        return q.first();
    }

    void clear()
    {
        if(_db.transaction()) {
            QSqlQuery q(_db);
            if (q.exec("delete from infos")) {
                if (q.exec("delete from urls")) {
                    if(!_db.commit()) {
                        qCritical() << _db.lastError();
                    }
                    return;
                }
            }

            if(!_db.rollback()) {
                qCritical() << _db.lastError();
            }
        }
    }

    void updateUrl(const QUrl &url, const QString &key, const QVariant &val)
    {
        qInfo() << url << key << val;

        if(_db.transaction()) {
            if (!urlExists(url)) {
                QString md5;
                if (url.isLocalFile()) {
                    md5 = utils::FastFileHash(QFileInfo(url.toLocalFile()));
                } else {
                    md5 = QString(QCryptographicHash::hash(url.toString().toUtf8(), QCryptographicHash::Md5).toHex());
                }

                QSqlQuery q(_db);
                if(q.prepare("insert into urls (url, md5, timestamp) values (?, ?, ?)")) {
                    q.addBindValue(url);
                    q.addBindValue(md5);
                    q.addBindValue(QDateTime::currentDateTimeUtc());
                    if (!q.exec()) {
                        if(!_db.rollback()) {
                            qCritical() << _db.lastError();
                        }
                        return;
                    }
                }
            }

            QSqlQuery q(_db);
            if(q.prepare("replace into infos (url, key, value) values (?, ?, ?)")) {
                q.addBindValue(url);
                q.addBindValue(key);
                q.addBindValue(val);
                CHECKED_EXEC(q);
                if(!_db.commit()) {
                    qCritical() << _db.lastError();
                }
            }
        }
    }

    QVariant queryValueByUrlKey(const QUrl &url, const QString &key)
    {
        if (!urlExists(url))
            return {};

        QSqlQuery q(_db);
        if(q.prepare("select value from infos where url = ? and key = ?")) {
            q.addBindValue(url);
            q.addBindValue(key);
            CHECKED_EXEC(q);

            if (q.next()) {
                return q.value(0);
            }
        }

        return QVariant();
    }

    QMap<QString, QVariant> queryByUrl(const QUrl &url)
    {
        if (!urlExists(url))
            return {};

        QSqlQuery q(_db);
        if(q.prepare("select key, value from infos where url = ?")) {
            q.addBindValue(url);
            CHECKED_EXEC(q);
        }

        QMap<QString, QVariant> res;
        while (q.next()) {
            res.insert(q.value(0).toString(), q.value(1));
        }

        return res;
    }

    ~MovieConfigurationBackend();

private:
    QSqlDatabase _db;
};

MovieConfigurationBackend::~MovieConfigurationBackend()
{
    _db.close();
    QSqlDatabase::removeDatabase(_db.connectionName());
}

MovieConfiguration &MovieConfiguration::get()
{
    if (_instance == nullptr) {
        QMutexLocker lock(&_instLock);
        _instance = new MovieConfiguration;
    }

    return *_instance;
}

void MovieConfiguration::removeUrl(const QUrl &url)
{
    _backend->deleteUrl(url);
}

bool MovieConfiguration::urlExists(const QUrl &url)
{
    return _backend->urlExists(url);
}

void MovieConfiguration::clear()
{
    _backend->clear();
}

void MovieConfiguration::updateUrl(const QUrl &url, const QString &key, const QVariant &val)
{
    _backend->updateUrl(url, key, val);
}

void MovieConfiguration::updateUrl(const QUrl &url, KnownKey key, const QVariant &val)
{
    updateUrl(url, knownKey2String(key), val);
}

void MovieConfiguration::append2ListUrl(const QUrl &url, KnownKey key, const QString &val)
{
    auto list = getByUrl(url, knownKey2String(key)).toString().split(';', QString::SkipEmptyParts);
    auto bytes = val.toUtf8().toBase64();
    list.append(bytes);
    updateUrl(url, key, list.join(';'));
}

void MovieConfiguration::removeFromListUrl(const QUrl &url, KnownKey key, const QString &val)
{
    ///add for warning by xxj ,no any means
    //val.isNull();
    auto list = getListByUrl(url, key);
}

QString MovieConfiguration::knownKey2String(KnownKey kk)
{
    switch (kk) {
    case KnownKey::SubDelay:
        return "sub-delay";
    case KnownKey::SubCodepage:
        return "sub-codepage";
    case KnownKey::SubId:
        return "sid";
    case KnownKey::StartPos:
        return "start";
    case KnownKey::ExternalSubs:
        return "external-subs";
    default:
        return "";
    }
}

QStringList MovieConfiguration::getListByUrl(const QUrl &url, KnownKey key)
{
    return decodeList(getByUrl(url, knownKey2String(key)));
}

QStringList MovieConfiguration::decodeList(const QVariant &val)
{
    auto list = val.toString().split(';', QString::SkipEmptyParts);
    std::transform(list.begin(), list.end(), list.begin(), [](const QString & s) {
        return QByteArray::fromBase64(s.toUtf8());
    });

    return list;
}

QVariant MovieConfiguration::getByUrl(const QUrl &url, const QString &key)
{
    return _backend->queryValueByUrlKey(url, key);
}

QVariant MovieConfiguration::getByUrl(const QUrl &url, KnownKey key)
{
    return getByUrl(url, knownKey2String(key));
}

QMap<QString, QVariant> MovieConfiguration::queryByUrl(const QUrl &url)
{
    return _backend->queryByUrl(url);
}

MovieConfiguration::~MovieConfiguration()
{
    delete _backend;
}

MovieConfiguration::MovieConfiguration()
    : QObject(nullptr)
{
}

#ifdef SQL_TEST
static void _backend_test()
{
    auto &mc = MovieConfiguration::get();
    mc.updateUrl(QUrl("movie1"), "sub-delay", -2.5);
    mc.updateUrl(QUrl("movie1"), "sub-delay", 1.5);
    mc.updateUrl(QUrl("movie2"), "sub-delay", 1.0);
    mc.updateUrl(QUrl("movie1"), "volume", 20);

    auto res = mc.queryByUrl(QUrl("movie1"));
    Q_ASSERT (res.size() == 2);
    qInfo() << res;

    mc.removeUrl(QUrl("movie1"));
    mc.updateUrl(QUrl("movie1"), "volume", 30);
    mc.updateUrl(QUrl("movie2"), "volume", 40);

    res = mc.queryByUrl(QUrl("movie1"));
    Q_ASSERT (res.size() == 1);
    qInfo() << res;
    mc.clear();
}
#endif

void MovieConfiguration::init()
{
    _backend = new MovieConfigurationBackend(this);
#ifdef SQL_TEST
    _backend_test();
#endif
}

}
