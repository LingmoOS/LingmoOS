// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notifymodel.h"

#include <QTimerEvent>
#include <QLoggingCategory>

#include "notifyentity.h"
#include "notifyitem.h"
#include "notifyaccessor.h"
#include "notifysetting.h"

namespace notification {
Q_DECLARE_LOGGING_CATEGORY(notifyLog)
}
namespace notifycenter {

NotifyModel::NotifyModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_accessor(NotifyAccessor::instance())
{
    connect(m_accessor, &NotifyAccessor::entityReceived, this, &NotifyModel::doEntityReceived);
    connect(this, &NotifyModel::countChanged, this, &NotifyModel::onCountChanged);
    connect(NotifySetting::instance(), &NotifySetting::contentRowCountChanged, this, &NotifyModel::updateContentRowCount);

    updateCollapseStatus();

    static const struct {
        const char *signalName;
        const char *slotName;
    } connectionTable[] = {
                           { SIGNAL(rowsInserted(QModelIndex,int,int)), SIGNAL(countChanged()) },
                           { SIGNAL(rowsRemoved(QModelIndex,int,int)), SIGNAL(countChanged()) },
                           { SIGNAL(modelReset()), SIGNAL(countChanged()) },
                           { SIGNAL(layoutChanged()), SIGNAL(dataInfoChanged()) },
                           };

    for (const auto &c : connectionTable) {
        connect(this, c.signalName, this, c.slotName);
    }
}

void NotifyModel::expandApp(int row)
{
    qDebug(notifyLog) << "Expand" << row;

    if (m_appNotifies.size() <= row || row < 0)
        return;

    auto notify = m_appNotifies[row];
    if (notify->type() != NotifyType::Overlap)
        return;

    const auto appName = notify->appName();
    beginRemoveRows(QModelIndex(), row, row);
    m_appNotifies.removeOne(notify);
    endRemoveRows();
    notify->deleteLater();

    auto entities = m_accessor->fetchEntities(appName);
    if(entities.size() >= 2) {
        QList<AppNotifyItem *> notifies;
        for (auto entity: entities) {
            auto item = new AppNotifyItem(entity);
            notifies.append(item);
        }

        beginInsertRows(QModelIndex(), row, row + notifies.size());
        auto group = new AppGroupNotifyItem(appName);
        const auto lastEntity = entities[0];
        group->updateLastEntity(lastEntity);
        m_appNotifies.insert(row, group);

        const auto start = row + 1;
        for (int i = 0; i < notifies.size(); i++) {
            auto item = notifies[i];
            m_appNotifies.insert(start + i, item);
        }
        endInsertRows();
    }
}

void NotifyModel::collapseApp(int row)
{
    qDebug(notifyLog) << "Collapse" << row;

    if (m_appNotifies.size() <= row || row < 0)
        return;

    auto notify = m_appNotifies[row];
    if (notify->type() != NotifyType::Group)
        return;

    const auto appName = notify->appName();

    QList<AppNotifyItem *> notifies;
    for (int i = row; i < m_appNotifies.size(); i++) {
        auto item = m_appNotifies[i];
        if (item->appName() == appName && item->type() == NotifyType::Normal) {
            notifies.append(item);
        }
    }

    const auto start = row;
    beginRemoveRows(QModelIndex(), start, start + notifies.size());
    // remove group
    m_appNotifies.removeOne(notify);
    notify->deleteLater();

    // remove normal
    for (int i = 0; i < notifies.size(); i++) {
        auto item = notifies[i];
        m_appNotifies.removeOne(item);
        item->deleteLater();
    }
    endRemoveRows();

    auto count = m_accessor->fetchEntityCount(appName);
    if (count >= 2) {
        auto entity = m_accessor->fetchLastEntity(appName);
        Q_ASSERT(entity.isValid());

        auto overlap = new OverlapAppNotifyItem(entity);
        overlap->updateCount(count);
        beginInsertRows(QModelIndex(), row, row);
        m_appNotifies.insert(row, overlap);
        endInsertRows();
    }
}

void NotifyModel::close()
{
    qDebug(notifyLog) << "close";

    beginResetModel();
    qDeleteAll(m_appNotifies);
    m_appNotifies.clear();
    endResetModel();
}

void NotifyModel::open()
{
    qDebug(notifyLog) << "Open";

    auto apps = fetchLastApps();
    for (auto appName : apps) {
        const auto tmp = m_accessor->fetchEntities(appName, 3);
        if (tmp.isEmpty())
            continue;

        const auto &entity = tmp.first();
        Q_ASSERT(entity.isValid());

        if (tmp.size() >= 2) {
            // add overlap
            qDebug(notifyLog) << "Add ovelay for the notify" << entity.id();

            const int start = m_appNotifies.size();
            auto overlap = new OverlapAppNotifyItem(entity);
            overlap->updateCount(tmp.size());
            beginInsertRows(QModelIndex(), start, start);
            m_appNotifies.append(overlap);
            endInsertRows();
        } else {
            // add normal
            const int start = m_appNotifies.size();
            beginInsertRows(QModelIndex(), start, start);
            auto notify = new AppNotifyItem(entity);
            m_appNotifies.append(notify);
            endInsertRows();
        }
    }
}

void NotifyModel::append(const NotifyEntity &entity)
{
    qDebug(notifyLog) << "Append notify of the app" << entity.appName() << entity.id();

    const auto appName = entity.appName();
    int row = -1;
    for (int i = 0; i < m_appNotifies.size(); i++) {
        auto item = m_appNotifies[i];
        if (item->appName() != appName)
            continue;
        row = i;
        break;
    }
    if (row >= 0) {
        auto notify = m_appNotifies[row];
        if (notify->type() == NotifyType::Normal) {
            // normal -> to overlap
            qDebug(notifyLog) << "Convert notify to overlay" << entity.id();

            beginRemoveRows(QModelIndex(), row, row);
            m_appNotifies.removeOne(notify);
            endRemoveRows();

            const auto index = lastNotifyIndex(entity);
            beginInsertRows(QModelIndex(), index, index);
            auto overlap = new OverlapAppNotifyItem(entity);
            m_appNotifies.insert(index, overlap);
            endInsertRows();

            notify->deleteLater();

        } else if (notify->type() == NotifyType::Overlap) {
            // overlap -> increase overlap count && update

            if (auto item = dynamic_cast<OverlapAppNotifyItem *>(notify)) {
                qDebug(notifyLog) << "Update ovelay to new notify" << entity.id();

                // count
                auto count = m_accessor->fetchEntityCount(appName);
                item->updateCount(count);
                item->setEntity(entity);

                // move or only update
                const auto index = lastNotifyIndex(entity);
                if (index != row) {

                    beginMoveRows({}, row, row, {}, index);
                    m_appNotifies.move(row, index);
                    endMoveRows();

                } else {
                    // update data by entity
                    const auto index = this->index(row);
                    dataChanged(index, index, {});
                }
            }
        } else if (notify->type() == NotifyType::Group) {
            // group -> insert first
            qDebug(notifyLog) << "Add notify to group" << entity.id();

            // insert to
            {
                const auto start = row + 1;
                beginInsertRows(QModelIndex(), start, start);
                auto item = new AppNotifyItem(entity);
                m_appNotifies.insert(start, item);
                endInsertRows();
            }

            const auto oldStart = lastNotifyIndex(notify);
            trayUpdateGroupLastEntity(entity);
            const auto newStart = lastNotifyIndex(notify);

            // move to new position.
            if (oldStart != newStart) {
                const int start = row;
                const auto count = notifyCount(notify->appName());

                beginMoveRows({}, start, start + count - 1, {}, newStart);
                const auto moved = m_appNotifies.mid(start, count);

                m_appNotifies.remove(start, count);
                for (int i = 0; i < moved.count(); i++) {
                    m_appNotifies.insert(newStart + i, moved.at(i));
                }
                endMoveRows();
            }
        }
    } else {
        // add normal
        const int start = lastNotifyIndex(entity);
        beginInsertRows(QModelIndex(), start, start);
        auto notify = new AppNotifyItem(entity);
        m_appNotifies.insert(start, notify);
        endInsertRows();
    }

    if (m_refreshTimer < 0) {
        m_refreshTimer = startTimer(std::chrono::milliseconds(1000));
    }
}

QList<QString> NotifyModel::fetchLastApps(int maxCount) const
{
    qDebug(notifyLog) << "Fetch last apps count" << maxCount;
    QStringList pinnedApps;
    auto allApps = m_accessor->fetchApps(-1);
    for (auto item : allApps) {
        if (m_accessor->applicationPin(item)) {
            pinnedApps << item;
            if (maxCount >= 0 && pinnedApps.size() >= maxCount)
                break;
        }
    }

    QList<NotifyEntity> lastEntities;
    for (auto item : allApps) {
        const auto entity = m_accessor->fetchLastEntity(item);
        if (!entity.isValid())
            continue;
        lastEntities << entity;
    }

    std::sort(lastEntities.begin(), lastEntities.end(), [this] (
                                                            const NotifyEntity &item1,
                                                            const NotifyEntity &item2) {
        return greaterNotify(item1, item2);
    });

    QStringList ret;
    for (int i = 0; i < lastEntities.size(); i++) {
        const auto entity = lastEntities[i];
        if (maxCount >= 0 && i >= maxCount)
            break;

        ret << entity.appName();
    }
    return ret;
}

NotifyEntity NotifyModel::greaterNotifyEntity(const AppNotifyItem *notifyItem) const
{
    if (const auto index = firstNotifyIndex(notifyItem->appName(), NotifyType::Group); index >= 0) {
        auto nextNotifyItem = dynamic_cast<AppGroupNotifyItem *>(m_appNotifies.at(index));
        return nextNotifyItem->lastEntity();
    }
    return notifyItem->entity();
}

bool NotifyModel::greaterNotify(const NotifyEntity &item1, const NotifyEntity &item2) const
{
    const bool item1Pin = m_accessor->applicationPin(item1.appName());
    const auto item2Pin = m_accessor->applicationPin(item2.appName());
    // using time when pin is same, otherwise using pin.
    if (item1Pin == item2Pin) {
        return item1.cTime() >= item2.cTime();
    }
    return item1Pin;
}

int NotifyModel::lastNotifyIndex(const NotifyEntity &entity) const
{
    int index = 0;
    for (auto item : std::as_const(m_appNotifies)) {
        const auto tmp = greaterNotifyEntity(item);
        if (greaterNotify(entity, tmp))
            return index;
        index++;
    }
    return index;
}

int NotifyModel::lastNotifyIndex(const AppNotifyItem *item) const
{
    if (item->type() == NotifyType::Group) {
        auto group = dynamic_cast<const AppGroupNotifyItem *>(item);
        return lastNotifyIndex(group->lastEntity());
    }
    return lastNotifyIndex(item->entity());
}

void NotifyModel::sortNotifies()
{
    sort(0, Qt::DescendingOrder);
}

void NotifyModel::trayUpdateGroupLastEntity(const NotifyEntity &entity)
{
    if (const auto index = firstNotifyIndex(entity.appName(), NotifyType::Group); index >= 0) {
        auto group = dynamic_cast<AppGroupNotifyItem *>(m_appNotifies.at(index));
        if (!group->lastEntity().isValid() || group->lastEntity().cTime() < entity.cTime()) {
            group->updateLastEntity(entity);
        }
    }
}

void NotifyModel::trayUpdateGroupLastEntity(const QString &appName)
{
    if (const auto index = firstNotifyIndex(appName, NotifyType::Group); index >= 0) {
        auto group = dynamic_cast<AppGroupNotifyItem *>(m_appNotifies.at(index));
        if (notifyCount(appName, NotifyType::Group) >= 1) {
            const auto nextNotifyItem = m_appNotifies.at(index + 1);
            const auto entity = nextNotifyItem->entity();
            if (!group->lastEntity().isValid() || group->lastEntity().cTime() < entity.cTime()) {
                group->updateLastEntity(entity);
            }
        }
    }
}

void NotifyModel::updateCollapseStatus()
{
    auto iter = std::find_if(m_appNotifies.begin(), m_appNotifies.end(), [](const AppNotifyItem *item) {
        return item->type() == NotifyType::Group;
    });
    const bool existGroup = iter != m_appNotifies.end();
    setCollapse(!existGroup);
}

void NotifyModel::updateContentRowCount(int rowCount)
{
    if (m_contentRowCount == rowCount)
        return;
    m_contentRowCount = rowCount;

    if (!m_appNotifies.isEmpty()) {
        dataChanged(index(0), index(m_appNotifies.size() - 1), {NotifyRole::NotifyContentRowCount});
    }
}

bool NotifyModel::greaterNotify(const AppNotifyItem *item1, const AppNotifyItem *item2) const
{
    const auto entity1 = greaterNotifyEntity(item1);
    const auto entity2 = greaterNotifyEntity(item2);
    // `group` is order high in same group.
    if (item1->appName() == item2->appName()) {
        if (item1->type() == item2->type()) {
            return entity1.cTime() >= entity2.cTime();
        }
        return item1->type() == NotifyType::Group;
    }

    return greaterNotify(entity1, entity2);
}

QString NotifyModel::dataInfo() const
{
    QStringList info;
    info.append(QString("notifyCount: %1").arg(m_appNotifies.size()));
    QString ret = info.join("\n");
    return ret;
}

bool NotifyModel::collapse() const
{
    return m_collapse;
}

void NotifyModel::remove(qint64 id)
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
    if (row >= 0) {
        auto notify = m_appNotifies[row];
        const auto appName = notify->appName();
        if (notify->type() == NotifyType::Normal) {
            // normal -> remove

            beginRemoveRows(QModelIndex(), row, row);
            m_appNotifies.removeOne(notify);
            m_accessor->removeEntity(id);
            notify->deleteLater();
            endRemoveRows();

            trayUpdateGroupLastEntity(appName);

        } else if (notify->type() == NotifyType::Overlap) {
            // overlap -> reduce overlap count && update || to normal

            if (auto item = dynamic_cast<OverlapAppNotifyItem *>(notify)) {
                // remove old entity
                const auto &deleteEntity = notify->entity();
                Q_ASSERT(deleteEntity.isValid());

                qDebug(notifyLog) << "Remove notify for ovelay" << deleteEntity.id();
                m_accessor->removeEntity(deleteEntity.id());

                auto count = m_accessor->fetchEntityCount(appName);
                item->updateCount(count);

                if (item->isEmpty()) {
                    // to normal

                    beginRemoveRows(QModelIndex(), row, row);
                    m_appNotifies.removeOne(notify);
                    endRemoveRows();

                    auto entity = m_accessor->fetchLastEntity(appName);
                    Q_ASSERT(entity.isValid());

                    qDebug(notifyLog) << "Convert ovelay to notify" << entity.id();

                    beginInsertRows(QModelIndex(), row, row);
                    auto newNotify = new AppNotifyItem(entity);
                    m_appNotifies.insert(row, newNotify);
                    endInsertRows();

                    notify->deleteLater();
                } else {
                    // update data
                    auto entity = m_accessor->fetchLastEntity(appName);
                    Q_ASSERT(entity.isValid());

                    qDebug(notifyLog) << "Update notify for ovelay" << entity.id();

                    // item->setEntity(entity);
                    // const auto index = this->index(row);
                    // dataChanged(index, index, {});

                    // TODO Reuse the item.
                    beginRemoveRows(QModelIndex(), row, row);
                    m_appNotifies.removeOne(notify);
                    endRemoveRows();

                    beginInsertRows(QModelIndex(), row, row);
                    auto newNotify = new OverlapAppNotifyItem(entity);
                    newNotify->updateCount(count);
                    m_appNotifies.insert(row, newNotify);
                    endInsertRows();

                    notify->deleteLater();
                }
            }
        }

        int row = -1;
        for (int i = 0; i < m_appNotifies.size(); i++) {
            auto item = m_appNotifies[i];
            if (item->appName() == appName && item->type() == NotifyType::Group) {
                row = i;
                break;
            }
        }

        if (row >= 0 && notifyCount(appName, NotifyType::Normal) <= 1) {
            // group -> remove group && to normal

            notify = m_appNotifies[row];

            beginRemoveRows(QModelIndex(), row, row);
            m_appNotifies.removeOne(notify);
            notify->deleteLater();
            endRemoveRows();
        }
    }
}

void NotifyModel::removeByApp(const QString &appName)
{
    qDebug(notifyLog) << "Remove all notifies for the app" << appName;

    int row = -1;
    for (int i = 0; i < m_appNotifies.size(); i++) {
        auto item = m_appNotifies[i];
        if (item->appName() == appName && item->type() == NotifyType::Group) {
            row = i;
            break;
        }
    }

    if (row >= 0) {
        auto notify = m_appNotifies[row];
        beginRemoveRows(QModelIndex(), row, row);
        m_appNotifies.removeOne(notify);
        endRemoveRows();
        notify->deleteLater();
    }

    QList<AppNotifyItem *> notifies;
    for (int i = row; i < m_appNotifies.size(); i++) {
        auto item = m_appNotifies[i];
        if (item->appName() == appName && item->type() == NotifyType::Normal) {
            notifies.append(item);
        }
    }

    const auto start = row;
    beginRemoveRows(QModelIndex(), start, start + notifies.size() - 1);
    for (int i = 0; i < notifies.size(); i++) {
        auto item = notifies[i];
        const auto id = item->id();
        m_appNotifies.removeOne(item);
        item->deleteLater();
    }
    endRemoveRows();

    m_accessor->removeEntityByApp(appName);
}

void NotifyModel::clear()
{
    beginResetModel();
    qDeleteAll(m_appNotifies);
    m_appNotifies.clear();
    endResetModel();

    m_accessor->clear();
}

void NotifyModel::collapseAllApp()
{
    close();
    open();
}

void NotifyModel::expandAllApp()
{
    QStringList existApps;
    for (auto item: std::as_const(m_appNotifies)) {
        if (existApps.contains(item->appName()))
            continue;
        existApps << item->appName();
    }

    const auto apps = fetchLastApps();
    for (auto appName : std::as_const(apps)) {
        if (existApps.contains(appName))
            continue;

        const auto tmp = m_accessor->fetchEntities(appName, 3);

        auto entity = tmp.first();
        Q_ASSERT(entity.isValid());

        if (tmp.size() >= 2) {
            // add overlap
            qDebug(notifyLog) << "Add ovelay for the notify" << entity.id();

            const int start = m_appNotifies.size();
            auto overlap = new OverlapAppNotifyItem(entity);
            overlap->updateCount(tmp.size());
            beginInsertRows(QModelIndex(), start, start);
            m_appNotifies.append(overlap);
            endInsertRows();
        } else {
            // add normal
            const int start = m_appNotifies.size();
            beginInsertRows(QModelIndex(), start, start);
            auto notify = new AppNotifyItem(entity);
            m_appNotifies.append(notify);
            endInsertRows();
        }
    }
}

void NotifyModel::invokeAction(qint64 id, const QString &actionId)
{
    qDebug(notifyLog) << "Invoke action for the notify" << id << actionId;
    auto entity = m_accessor->fetchEntity(id);
    if (!entity.isValid())
        return;

    m_accessor->invokeAction(entity, actionId);

    remove(id);
}

void NotifyModel::pinApplication(const QString &appName, bool pin)
{
    qDebug(notifyLog) << "Pin the application" << appName << pin;

    m_accessor->pinApplication(appName, pin);

    QList<AppNotifyItem *> notifies;
    for (const auto item : std::as_const(m_appNotifies)) {
        if (item->appName() != appName)
            continue;
        notifies << item;
    }
    if (!notifies.isEmpty()) {
        for (auto item : notifies) {
            item->setPinned(pin);
        }
        dataChanged(index(0), index(rowCount(QModelIndex()) - 1), {NotifyPinned});

        // sort
        sortNotifies();
    }
}

int NotifyModel::rowCount(const QModelIndex &parent) const
{
    return m_appNotifies.size();
}

QVariant NotifyModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row < 0 || row >= m_appNotifies.size())
        return QVariant();

    auto notify = m_appNotifies[row];
    if (role == NotifyRole::NotifyItemType) {
        static const QMap<NotifyType, QString> mapping {
            {NotifyType::Normal, QLatin1String("normal")},
            {NotifyType::Overlap, QLatin1String("overlap")},
            {NotifyType::Group, QLatin1String("group")}
        };
        return mapping.value(notify->type());
    } else if (role == NotifyRole::NotifyId) {
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
    } else if (role == NotifyRole::NotifyPinned) {
        return notify->pinned();
    } else if (role == NotifyRole::NotifyStrongInteractive) {
        return notify->strongInteractive();
    } else if (role == NotifyRole::NotifyContentIcon) {
        return notify->contentIcon();
    } else if (role == NotifyRole::NotifyOverlapCount) {
        if (auto item = dynamic_cast<OverlapAppNotifyItem *>(notify)) {
            return item->count();
        }
    } else if (role == NotifyRole::NotifyContentRowCount) {
        return NotifySetting::instance()->contentRowCount();
    }
    return QVariant::fromValue(notify);
}

int NotifyModel::notifyCount(const QString &appName, const NotifyType &type) const
{
    int count = 0;
    for (int i = 0; i < m_appNotifies.size(); i++) {
        auto item = m_appNotifies[i];
        if (item->appName() != appName || item->type() != type)
            continue;
        count++;
    }
    return count;
}

int NotifyModel::firstNotifyIndex(const QString &appName, const NotifyType &type) const
{
    for (int i = 0; i < m_appNotifies.size(); i++) {
        auto item = m_appNotifies[i];
        if (item->type() == type && item->appName() == appName)
            return i;
    }
    return -1;
}

void NotifyModel::updateTime()
{
    if (m_appNotifies.isEmpty())
        return;

    QList<AppNotifyItem *> tmp = m_appNotifies;
    for (auto item : tmp) {
        item->updateTime();
    }
    dataChanged(index(0), index(rowCount(QModelIndex()) - 1), {NotifyTime});
}

QHash<int, QByteArray> NotifyModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{{NotifyItemType, "type"},
                                              {NotifyId, "id"},
                                              {NotifyAppId, "appId"},
                                              {NotifyAppName, "appName"},
                                              {NotifyIconName, "iconName"},
                                              {NotifyActions, "actions"},
                                              {NotifyDefaultAction, "defaultAction"},
                                              {NotifyTime, "time"},
                                              {NotifyTitle, "title"},
                                              {NotifyContent, "content"},
                                              {NotifyPinned, "pinned"},
                                              {NotifyStrongInteractive, "strongInteractive"},
                                              {NotifyContentIcon, "contentIcon"},
                                              {NotifyOverlapCount, "overlapCount"},
                                              {NotifyContentRowCount, "contentRowCount"}};
    return roles;
}

void NotifyModel::doEntityReceived(qint64 id)
{
    qDebug(notifyLog) << "Receive entity" << id;
    auto entity = m_accessor->fetchEntity(id);
    if (!entity.isValid()) {
        qWarning(notifyLog) << "Received invalid entity:" << id << ", appName:" << entity.appName();
        return;
    }
    append(entity);
}

void NotifyModel::onCountChanged()
{
    updateCollapseStatus();
    emit dataInfoChanged();
}

int NotifyModel::notifyCount(const NotifyType &type) const
{
    int count = std::count_if(m_appNotifies.begin(), m_appNotifies.end(), [this, type] (AppNotifyItem *item) {
        return item->type() == type;
    });
    return count;
}

int NotifyModel::notifyCount(const QString &appName) const
{
    int count = std::count_if(m_appNotifies.begin(), m_appNotifies.end(), [this, appName] (AppNotifyItem *item) {
        return item->appName() == appName;
    });
    return count;
}

void NotifyModel::timerEvent(QTimerEvent *event)
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

void NotifyModel::sort(int column, Qt::SortOrder order)
{
    auto notifies = m_appNotifies;
    // emit layoutAboutToBeChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);
    beginResetModel();

    std::sort(notifies.begin(), notifies.end(), [this] (
                                                    const AppNotifyItem *item1,
                                                    const AppNotifyItem *item2) {
        return greaterNotify(item1, item2);
    });
    m_appNotifies.swap(notifies);

    endResetModel();
    // emit layoutChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);
}

void NotifyModel::setCollapse(bool newCollapse)
{
    if (m_collapse == newCollapse)
        return;
    m_collapse = newCollapse;
    emit collapseChanged();
}
}
