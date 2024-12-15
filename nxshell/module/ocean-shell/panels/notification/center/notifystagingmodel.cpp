// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notifystagingmodel.h"

#include <QTimerEvent>
#include <QLoggingCategory>

#include "notifyentity.h"
#include "notifyitem.h"
#include "notifyaccessor.h"
#include "dbaccessor.h"
#include "notifysetting.h"

namespace notification {
Q_DECLARE_LOGGING_CATEGORY(notifyLog)
}
namespace notifycenter {

NotifyStagingModel::NotifyStagingModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_accessor(DBAccessor::instance())
{
    connect(NotifyAccessor::instance(), &NotifyAccessor::stagingEntityReceived, this, &NotifyStagingModel::doEntityReceived);
    connect(NotifyAccessor::instance(), &NotifyAccessor::stagingEntityClosed, this, &NotifyStagingModel::onEntityClosed);
    connect(NotifySetting::instance(), &NotifySetting::contentRowCountChanged, this, &NotifyStagingModel::updateContentRowCount);
}

void NotifyStagingModel::close()
{
    qDebug(notifyLog) << "close";

    beginResetModel();
    qDeleteAll(m_appNotifies);
    m_appNotifies.clear();
    endResetModel();
}

void NotifyStagingModel::push(const NotifyEntity &entity)
{
    qDebug(notifyLog) << "Append notify of the app" << entity.appName() << entity.id();

    beginInsertRows(QModelIndex(), 0, 0);
    auto notify = new AppNotifyItem(entity);
    m_appNotifies.prepend(notify);
    endInsertRows();

    {
        const int count = m_appNotifies.size();
        if (count > BubbleMaxCount) {
            const int row = count - 1;
            auto notify = m_appNotifies[row];
            beginRemoveRows(QModelIndex(), row, row);
            m_appNotifies.removeOne(notify);
            endRemoveRows();
            notify->deleteLater();
        }
    }
    {
        // update count
        auto count = m_accessor->fetchEntityCount(DataAccessor::AllApp(), NotifyEntity::NotProcessed);
        updateOverlapCount(count);
    }

    if (m_refreshTimer < 0) {
        m_refreshTimer = startTimer(std::chrono::milliseconds(1000));
    }
}

void NotifyStagingModel::closeNotify(qint64 id)
{
    auto entity = m_accessor->fetchEntity(id);
    if (entity.isValid()) {
        NotifyAccessor::instance()->closeNotify(entity);
    }
    remove(id);
}

void NotifyStagingModel::invokeNotify(qint64 id, const QString &actionId)
{
    const auto entity = notifyById(id);
    if (entity.isValid()) {
        NotifyAccessor::instance()->invokeNotify(entity, actionId);
    }
    remove(id);
}

void NotifyStagingModel::remove(qint64 id)
{
    qDebug(notifyLog) << "Remove notify by id" << id;

    int row = -1;
    for (int i = 0; i < m_appNotifies.size(); i++) {
        auto item = m_appNotifies[i];
        if (item->id() != id)
            continue;
        row = i;
        break;
    }

    auto entities = m_accessor->fetchEntities(DataAccessor::AllApp(), NotifyEntity::NotProcessed, BubbleMaxCount + OverlayMaxCount);

    if (row >= 0) {
        auto notify = m_appNotifies[row];
        const auto entity = notify->entity();

        beginRemoveRows(QModelIndex(), row, row);
        m_appNotifies.removeOne(notify);
        notify->deleteLater();
        endRemoveRows();

        auto newEntities = entities;
        newEntities.removeIf([this] (const NotifyEntity &entity) {
            for (int i = 0; i < m_appNotifies.size(); i++) {
                auto item = m_appNotifies[i];
                if (item->entity() == entity)
                    return true;
            }
            return false;
        });

        if (!newEntities.isEmpty()) {
            int insertedIndex = -1;
            NotifyEntity newEntity;

            for (int i = 0; i < m_appNotifies.size(); i++) {
                auto item = m_appNotifies[i];
                const auto id = item->id();
                const auto time = item->entity().cTime();

                for (auto entity : newEntities) {
                    if (time < entity.cTime()) {
                        insertedIndex = i;
                        newEntity = entity;
                        break;
                    }
                }
            }
            if (insertedIndex < 0) {
                insertedIndex = m_appNotifies.size();
                newEntity = newEntities.first();
            }

            qDebug(notifyLog) << "Insert notify" << newEntity.id();
            beginInsertRows(QModelIndex(), insertedIndex, insertedIndex);
            auto notify = new AppNotifyItem(newEntity);
            m_appNotifies.insert(insertedIndex, notify);
            endInsertRows();
        }
    }
    updateOverlapCount(entities.size());
}

void NotifyStagingModel::open()
{
    qDebug(notifyLog) << "Open staging model";

    auto entities = m_accessor->fetchEntities(DataAccessor::AllApp(), NotifyEntity::NotProcessed, BubbleMaxCount + OverlayMaxCount);

    qDebug(notifyLog) << "Fetched staging size" << entities.size();
    if (entities.size() <= 0)
        return;

    beginResetModel();

    const auto count = std::min(static_cast<int>(entities.size()), BubbleMaxCount);
    for (int i = 0; i < count; i++) {
        auto notify = new AppNotifyItem(entities.at(i));
        m_appNotifies << notify;
    }
    updateOverlapCount(entities.size());

    endResetModel();
}

int NotifyStagingModel::rowCount(const QModelIndex &parent) const
{
    return m_appNotifies.size();
}

QVariant NotifyStagingModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row < 0 || row >= m_appNotifies.size())
        return QVariant();

    auto notify = m_appNotifies[row];
    if (role == NotifyRole::NotifyId) {
        return notify->id();
    } else if (role == NotifyRole::NotifyAppId) {
        return notify->appId();
    } else if (role == NotifyRole::NotifyAppName) {
        return notify->appName();
    } else if (role == NotifyRole::NotifyIconName) {
        return notify->entity().appIcon();
    } else if (role == NotifyRole::NotifyTitle) {
        return notify->entity().summary();
    } else if (role == NotifyRole::NotifyContent) {
        return notify->entity().body();
    } else if (role == NotifyRole::NotifyActions) {
        return notify->actions();
    } else if (role == NotifyRole::NotifyDefaultAction) {
        return notify->defaultAction();
    } else if (role == NotifyRole::NotifyTime) {
        return notify->time();
    } else if (role == NotifyRole::NotifyStrongInteractive) {
        return notify->strongInteractive();
    } else if (role == NotifyRole::NotifyContentIcon) {
        return notify->contentIcon();
    } else if (role == NotifyRole::NotifyOverlapCount) {
        if (row < BubbleMaxCount - 1) {
            return 0;
        } else {
            return std::min(OverlayMaxCount, overlapCount());
        }
    } else if (role == NotifyRole::NotifyContentRowCount) {
        return NotifySetting::instance()->contentRowCount();
    }
    return QVariant::fromValue(notify);
}

void NotifyStagingModel::updateTime()
{
    if (m_appNotifies.isEmpty())
        return;

    QList<AppNotifyItem *> tmp = m_appNotifies;
    for (auto item : tmp) {
        item->updateTime();
    }
    dataChanged(index(0), index(rowCount(QModelIndex()) - 1), {NotifyTime});
}

NotifyEntity NotifyStagingModel::notifyById(qint64 id) const
{
    for (int i = 0; i < m_appNotifies.size(); i++) {
        auto item = m_appNotifies[i];
        if (item->id() == id)
            return item->entity();
    }
    return {};
}

void NotifyStagingModel::replace(const NotifyEntity &entity)
{
    for (int i = 0; i < m_appNotifies.size(); i++) {
        auto item = m_appNotifies[i];
        if (item->entity().id() == entity.id()) {
            item->setEntity(entity);
            const auto index = this->index(i, 0, {});
            dataChanged(index, index);
            break;
        }
    }
}

QHash<int, QByteArray> NotifyStagingModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{{NotifyItemType, "type"},
                                              {NotifyId, "id"},
                                              {NotifyAppName, "appName"},
                                              {NotifyAppId, "appId"},
                                              {NotifyIconName, "iconName"},
                                              {NotifyActions, "actions"},
                                              {NotifyDefaultAction, "defaultAction"},
                                              {NotifyTime, "time"},
                                              {NotifyTitle, "title"},
                                              {NotifyContent, "content"},
                                              {NotifyStrongInteractive, "strongInteractive"},
                                              {NotifyContentIcon, "contentIcon"},
                                              {NotifyOverlapCount, "overlapCount"},
                                              {NotifyContentRowCount, "contentRowCount"}};
    return roles;
}

void NotifyStagingModel::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_refreshTimer) {
        updateTime();

        if (m_appNotifies.isEmpty()) {
            killTimer(m_refreshTimer);
            m_refreshTimer = -1;
        }
    }
    return QAbstractListModel::timerEvent(event);
}

int NotifyStagingModel::overlapCount() const
{
    return m_overlapCount;
}

void NotifyStagingModel::doEntityReceived(qint64 id)
{
    qDebug(notifyLog) << "Receive entity" << id;
    auto entity = m_accessor->fetchEntity(id);
    if (!entity.isValid()) {
        qWarning(notifyLog) << "Received invalid entity:" << id << ", appName:" << entity.appName();
        return;
    }
    if (entity.isReplace() && notifyById(id).isValid()) {
        replace(entity);
    } else {
        push(entity);
    }
}

void NotifyStagingModel::onEntityClosed(qint64 id)
{
    remove(id);
}

void NotifyStagingModel::updateOverlapCount(int count)
{
    count = std::min(count, BubbleMaxCount + OverlayMaxCount);
    auto diff = std::max(0, count - static_cast<int>(m_appNotifies.size()));
    m_overlapCount = diff;

    if (!m_appNotifies.isEmpty()) {
        dataChanged(index(0), index(m_appNotifies.size() - 1), {NotifyRole::NotifyOverlapCount});
    }
}

void NotifyStagingModel::updateContentRowCount(int rowCount)
{
    if (m_contentRowCount == rowCount)
        return;

    m_contentRowCount = rowCount;

    if (!m_appNotifies.isEmpty()) {
        dataChanged(index(0), index(m_appNotifies.size() - 1), {NotifyRole::NotifyContentRowCount});
    }
}
}
