// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "constants.h"

#define ACTION_SEGMENT ("|")
#define HINT_SEGMENT ("|")
#define KEYVALUE_SEGMENT ("!!!")

class NotificationEntity;

class AbstractPersistence : public QObject
{
    Q_OBJECT

    Q_PROPERTY(uint recordCount READ recordCount NOTIFY recordCountChanged FINAL)

public:
    explicit AbstractPersistence(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    virtual void addOne(EntityPtr entity) = 0;
    virtual void addAll(QList<EntityPtr> entities) = 0;
    virtual void removeOne(const QString &id) = 0;
    virtual void removeApp(const QString &app_name) = 0;
    virtual void removeAll() = 0;

    virtual QList<EntityPtr> getAllNotify() = 0;
    virtual QString getAll() = 0;
    virtual QString getById(const QString &id) = 0;
    virtual EntityPtr getNotifyById(const QString &id) { return EntityPtr{}; }

    virtual QString getFrom(int rowCount, const QString &offsetId) = 0;
    virtual uint recordCount() = 0;

signals:
    void RecordAdded(EntityPtr entity);
    void recordCountChanged(uint count);
};

class Persistence : public AbstractPersistence
{
public:
    explicit Persistence(QObject *parent = nullptr);

    void addOne(EntityPtr entity) override;              //向数据库添加一条通知数据
    uint doAddOne(EntityPtr entity);
    void addAll(QList<EntityPtr> entities) override;     //向数据库添加多条通知数据
    void removeOne(const QString &id) override;          //根据通知的ID,从数据库删除一条通知.
    void removeApp(const QString &app_name) override;    //根据App名称从数据库删除App组的通知
    void removeAll() override;                           //从数据库删除所有通知

    QList<EntityPtr> getAllNotify() override;            //获取所有通知
    QString getAll() override;                           //将所有通知转为Json格式的字符串返回
    QString getById(const QString &id) override;         //根据ID获取通知信息
    EntityPtr getNotifyById(const QString &id) override;

    // the result starts with offset + 1
    // If rowcount is - 1, it is obtained from offset + 1 to the last.
    QString getFrom(int rowCount, const QString &offsetId) override;

    uint recordCount() override;                       //获取通知记录有多少条

private:
    void setRecordCount(uint count);
    void attemptCreateTable();  //在数据库中尝试创建一个表,记录通知信息
    QString ConvertMapToString(const QVariantMap &map); //将QVariantMap类型转换为QString类型
    QVariantMap ConvertStringToMap(const QString &text); //将QString类型转换为QVariantMap类型

    //判断数据库表中的属性名称是否有效,有效返回true,无效返回false
    bool IsAttributeValid(const QString &tableName, const QString &attributeName);
    //添加一个属性到数据库表中,成功返回true,失败返回false
    bool AddAttributeToTable(const QString &tableName, const QString &attributeName);
    EntityPtr fromJsonValue(const QJsonValue &jsonValue);

private:
    QSqlDatabase m_dbConnection;
    QSqlQuery m_query;
    uint m_recordCount;
};

#endif // PERSISTENCE_H
