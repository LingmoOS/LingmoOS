// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOCKPERSISTENCE_H
#define MOCKPERSISTENCE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "notification-center/notifycenterwidget.h"
#include "notification/persistence.h"

class MockPersistence : public AbstractPersistence
{
public:
    MOCK_METHOD0(getAllNotify, QList<EntityPtr>());
    MOCK_METHOD0(getAll,QString());
    MOCK_METHOD0(removeAll,void());
    MOCK_METHOD1(addOne, void(EntityPtr));
    MOCK_METHOD1(addAll, void(QList<EntityPtr>));
    MOCK_METHOD1(removeOne, void(const QString&));
    MOCK_METHOD1(removeApp, void(const QString&));
    MOCK_METHOD0(recordCount, uint());
    MOCK_METHOD1(getById, QString(const QString&));
    MOCK_METHOD2(getFrom, QString(int, const QString&));
};

// 虚拟数据类
class PersistenceHelper
{
public:
    PersistenceHelper();
    QList<EntityPtr> getAllNotify() const;
    QString getAll();
    void removeAll();
    void addOne(EntityPtr entiry);
    void removeApp(const QString &app_name);
    void addAll(QList<EntityPtr> entiryList);
    void removeOne(const QString& id);
    int getRecordCount();
    QString getById(const QString& id);
    QString getFrom(int count, const QString &offsetId);
private:
    QList<EntityPtr> m_entityList;
};

#endif // MOCKPERSISTENCE_H
