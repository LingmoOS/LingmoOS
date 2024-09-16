// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <QDBusInterface>
#include <QObject>

#include "constants.h"

#define ACTION_SEGMENT ("|")
#define HINT_SEGMENT ("|")
#define KEYVALUE_SEGMENT ("!!!")

class NotificationEntity;

class AbstractPersistence : public QObject
{
    Q_OBJECT
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

    virtual QString getFrom(int rowCount, const QString &offsetId) = 0;
    virtual int getRecordCount() = 0;

signals:
    void RecordAdded(EntityPtr entity);
};

class PersistenceObserver : public AbstractPersistence
{
    Q_OBJECT
public:
    explicit PersistenceObserver(QObject *parent = nullptr);

    void addOne(EntityPtr entity) override { Q_UNUSED(entity); Q_UNREACHABLE(); }
    void addAll(QList<EntityPtr> entities) override { Q_UNUSED(entities); Q_UNREACHABLE(); }
    void removeOne(const QString &id) override;          //根据通知的ID,从数据库删除一条通知.
    void removeApp(const QString &appName) override { Q_UNUSED(appName); Q_UNREACHABLE(); }
    void removeAll() override;                           //从数据库删除所有通知

    QList<EntityPtr> getAllNotify() override;            //获取所有通知
    QString getAll() override { Q_UNREACHABLE(); }
    QString getById(const QString &id) override { Q_UNUSED(id); Q_UNREACHABLE(); }

    // the result starts with offset + 1
    // If rowcount is - 1, it is obtained from offset + 1 to the last.
    QString getFrom(int rowCount, const QString &offsetId) override { Q_UNUSED(rowCount); Q_UNUSED(offsetId); Q_UNREACHABLE(); }

    int getRecordCount() override { Q_UNREACHABLE(); }

private Q_SLOTS:
    void onReceivedRecord(const QString &id);

private:
    EntityPtr json2Entity(const QString &data);
    EntityPtr json2Entity(const QJsonObject &obj);

    QDBusInterface *notifyObserver();

private:
    QScopedPointer<QDBusInterface> m_notifyObserver;
};

#endif // PERSISTENCE_H
