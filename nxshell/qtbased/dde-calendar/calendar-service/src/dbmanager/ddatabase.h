// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DDATABASE_H
#define DDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QThread>
#include <QVector>
#include <QMutex>
#include <QMap>

//账户数据库的实例
#define DBAccountManager QSqlDatabase::database(DDataBase::NameAccountManager)
//云同步的临时数据库的实例
#define DBSync QSqlDatabase::database(DDataBase::NameSync)

class DDataBase : public QObject
{
    Q_OBJECT
public:
    //账户数据库的别名
    static const QString NameAccountManager;
    //云同步的临时数据库的别名
    static const QString NameSync;
    //账户
    static const QString sql_create_account;
    //日程表
    static const QString sql_create_schedules;
    //类型表
    static const QString sql_create_scheduleType;
    //颜色表
    static const QString sql_create_typeColor;
    //创建上传任务表
    static const QString sql_create_uploadTask;
    //创建提醒任务表
    static const QString sql_create_remindTask;
    //创建帐户管理表
    static const QString sql_create_accountManager;
    //日历通用设置
    static const QString sql_create_calendargeneralsettings;

    //工作颜色id
    static const QString GWorkColorID;
    //生活颜色id
    static const QString GLifeColorID;
    //其他颜色id
    static const QString GOtherColorID;
    //节假日颜色id
    static const QString GFestivalColorID;

public:
    explicit DDataBase(QObject *parent = nullptr);
    virtual ~DDataBase();

    QString getDBPath() const;
    void setDBPath(const QString &DBPath);
    static QString createUuid();

    QString getConnectionName() const;
    void setConnectionName(const QString &connectionName);

    //初始化数据库数据，会创建数据库文件和相关数据表
    virtual void initDBData();
    void dbOpen();

    //判断数据库是否存在
    bool dbFileExists();

    //删除db数据库文件
    void removeDB();

protected:
    //创建数据库
    virtual void createDB() = 0;

protected:
    QSqlDatabase m_database;
    QString m_DBPath;
    QString m_connectionName;
};

/**
 * @brief The DbPathMutex struct 整理了sqlite数据库文件锁的相关操作
 */
struct SqliteMutex {
private:
    /**
     * @brief The SqliteMutex struct 用于跳过QMutex的拷贝构造函数和operator=函数的调用
     */
    struct UnCopyMutex {
        UnCopyMutex(){}
        UnCopyMutex(const UnCopyMutex &) {}
        UnCopyMutex &operator=(const UnCopyMutex &) {return *this;}
        void lock();
        void unlock();

    private:
        QMutex m;
    } m;                                //数据库文件锁
    bool transactionLocked = false;     //是否开启事务 及 数据库文件是否被锁定
    qint64 transactionThreadId = 0;     //开启事务的线程id

public:
    /**
     * @brief lock 数据库文件被锁定，用于非事务场景
     */
    void lock();

    /**
     * @brief unlock 数据库文件被解锁，用于非事务场景
     */
    void unlock();

    /**
     * @brief transactionLock 数据库文件被锁定，用非事务场景
     */
    void transactionLock();

    /**
     * @brief transactionUnlock 数据库文件被解锁，用非事务场景
     */
    void transactionUnlock();
};

/**
 * @brief The SqliteQuery class 根据数据库文件锁来执行sql语句的类
 */
class SqliteQuery : public QSqlQuery {
public:
    explicit SqliteQuery(QSqlDatabase db);
    explicit SqliteQuery(const QString &connectionName);
    SqliteQuery(const QString &query, QSqlDatabase db);

    bool exec(QString sql);
    bool exec();

    void transaction();
    void commit();
    void rollback();

private:
    QSqlDatabase _db;
};

#endif // DDATABASE_H
