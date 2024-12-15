// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMutex>
#include <QObject>
#include <QSqlDatabase>

#include "dataaccessor.h"

namespace notification {
class NotifyEntity;

/**
 * @brief The DBAccessor class
 */
class DBAccessor : public DataAccessor
{
public:
    explicit DBAccessor(const QString &key);
    ~DBAccessor() override;

    static DBAccessor *instance();

    bool open(const QString &dataPath);

    qint64 addEntity(const NotifyEntity &entity) override;
    qint64 replaceEntity(qint64 id, const NotifyEntity &entity) override;
    void updateEntityProcessedType(qint64 id, int processedType) override;

    NotifyEntity fetchEntity(qint64 id) override;
    int fetchEntityCount(const QString &appName, int processedType) const override;
    NotifyEntity fetchLastEntity(const QString &appName, int processedType) override;
    QList<NotifyEntity> fetchEntities(const QString &appName, int processedType, int maxCount) override;
    NotifyEntity fetchLastEntity(uint notifyId) override;
    QList<QString> fetchApps(int maxCount) const override;

    void removeEntity(qint64 id) override;
    void removeEntityByApp(const QString &appName) override;
    void clear() override;

private:
    void tryToCreateTable();

    bool isAttributeValid(const QString &tableName, const QString &attributeName) const;
    bool addAttributeToTable(const QString &tableName, const QString &attributeName, const QString &type) const;
    void updateProcessTypeValue();

private:
    NotifyEntity parseEntity(const QSqlQuery &query);

private:
    mutable QMutex m_mutex;
    QSqlDatabase m_connection;
    QString m_key;
};
}
