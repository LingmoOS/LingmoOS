// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mockpersistence.h"

PersistenceHelper::PersistenceHelper()
{
    QStringList appName;
    appName << "deepin-editor" << "google-chrome"
            << "deepin-image-viewer" << "dde-calendar"
            << "deepin-diskmanager" << "org.deepin.scanner";

    foreach(auto name, appName) {
        for (int i = 0; i < 10; i++) {
            auto notification = std::make_shared<NotificationEntity>(name);
            m_entityList << notification;
        }
    }
}

QList<EntityPtr> PersistenceHelper::getAllNotify() const
{
    return m_entityList;
}

QString PersistenceHelper::getAll()
{
    return QString();
}

void PersistenceHelper::removeAll()
{
    m_entityList.clear();
}

void PersistenceHelper::addOne(EntityPtr entiry)
{
    m_entityList << entiry;
}

void PersistenceHelper::removeApp(const QString &app_name)
{
    Q_UNUSED(app_name)
    return;
}

void PersistenceHelper::addAll(QList<EntityPtr> entiryList)
{
    m_entityList << entiryList;
}

void PersistenceHelper::removeOne(const QString &id)
{
    foreach(auto item, m_entityList) {
        if (item->id() == id) {
            m_entityList.removeOne(item);
            return;
        }
    }
}

int PersistenceHelper::getRecordCount()
{
    return  m_entityList.size();
}

QString PersistenceHelper::getById(const QString &id)
{
    Q_UNUSED(id)
    return QString();
}

QString PersistenceHelper::getFrom(int count, const QString &offsetId)
{
    Q_UNUSED(count)
    Q_UNUSED(offsetId)
    return QString();
}
