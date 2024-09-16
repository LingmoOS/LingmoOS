// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notifymodel.h"
#include "notification/persistence.h"
#include "notification/notifysettings.h"
#include "notifylistview.h"

#include <QDebug>
#include <QDateTime>
#include <QTimer>

NotifyModel::NotifyModel(QObject *parent, AbstractPersistence *database, NotifyListView *view)
    : QAbstractListModel(parent)
    , m_view(view)
    , m_database(database)
    , m_freeTimer(new QTimer(this))
    , m_settings(new NotifySettingsObserver(this))
{
    m_freeTimer->setInterval(AnimationTime + 100);
    initData();
    initConnect();
}

ListItemPtr NotifyModel::getAppData(const QString &appName) const
{
    auto iter = std::find_if(m_notifications.begin(), m_notifications.end(),
                             [this, appName](const ListItemPtr &item) {
        return item->appName() == appName;
    });
    Q_ASSERT(iter != m_notifications.end());
    return *iter;
}

int NotifyModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    int count = 0;
    for (int i = 0; i < showCount(); i++) {
        count += countOfEachTitle(m_notifications[i]->isCollapse()) + m_notifications[i]->showCount();
    }
    return count;
}

QVariant NotifyModel::data(const QModelIndex &index, int role) const
{
    //返回空值，不绘制check状态
    if (Qt::CheckStateRole == role)
        return QVariant();

    if (!index.isValid() || index.row() > (rowCount(QModelIndex()) - 1)) {
        return QVariant();
    }

    EntityPtr entity = getEntityByRow(index.row());
    if (entity) {
        entity->setCurrentIndex(index.row());
        return QVariant::fromValue(entity);
    }
    return QVariant();
}

Qt::ItemFlags NotifyModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        if (m_view != nullptr) m_view->openPersistentEditor(index);
        return QAbstractListModel::flags(index);
    }
    return QAbstractListModel::flags(index);
}

int NotifyModel::remainNotificationCount() const
{
    // 剩余条数 = 未显示项的所有条数(不显示项的隐藏条数)
    int count = 0;
    int i = showCount();
    while (i < m_notifications.count()) {
        count += m_notifications[i++]->count();
    }
    return count;
}

int NotifyModel::rowOfFirstNotification() const
{
    if (m_notifications.count() <= 0)
        return -1;
    return countOfEachTitle(m_notifications[0]->isCollapse());
}

void NotifyModel::addNotify(EntityPtr entity)
{
    qDebug() << "addNotify(): Notification's appName:" << entity->appName()
             << ", id:" << entity->id();
    beginResetModel();
    addAppData(entity);
    sortNotifications();
    endResetModel();
}

void NotifyModel::removeNotify(EntityPtr entity)
{
    if (m_notifications.isEmpty())
        return;

    beginResetModel();
    for (int i = 0; i < m_notifications.size(); i++) {
        const ListItemPtr &appGroup = m_notifications[i];
        if (appGroup->appName() == entity->appName()) {
            appGroup->remove(entity);

            if (appGroup->isEmpty()) {
                m_notifications.removeAt(i);
                Q_EMIT appCountChanged();
                break;
            }
        }
    }
    endResetModel();

    if (m_database != nullptr) {
        m_database->removeOne(QString::number(entity->id()));
    }
    Q_EMIT removedNotif();
}

void NotifyModel::removeAppGroup(const QString &appName)
{
    beginResetModel();
    if (m_notifications.isEmpty())
        return;
    QList<EntityPtr> removedEntities;
    for (int i = 0; i < m_notifications.size(); i++) {
        if (m_notifications[i]->appName() == appName) {
            removedEntities = m_notifications[i]->take();
            m_notifications.removeAt(i);
            Q_EMIT appCountChanged();
            // it's unique item with same appName in m_notifications, so we can break,
            // otherwise, it maybe error remove more than one item.
            break;
        }
    }
    sortNotifications();
    endResetModel();
    // removeApp is not provided by `Notification`.
    for (auto entity : removedEntities) {
        m_database->removeOne(QString::number(entity->id()));
    }
}

void NotifyModel::removeAllData()
{
    beginResetModel();
    m_notifications.clear();
    Q_EMIT appCountChanged();
    endResetModel();
    m_database->removeAll();
}

void NotifyModel::expandDataByAppName(const QString &appName)
{
    beginResetModel();
    for (int i = 0; i < m_notifications.size(); i++) {
        const ListItemPtr &appGroup = m_notifications[i];
        if (appName.isEmpty() || appGroup->appName() == appName) {
            appGroup->toggleFolding(false);
            m_expandedApps.insert(appName);
            updateFullCollapsed();
        }
    }
    endResetModel();
}

void NotifyModel::expandData()
{
    m_isCollapse = false;
    // 展开Model，但对每项需要收起
    beginResetModel();
    for (int i = 0; i < m_notifications.size(); i++) {
        const ListItemPtr &appGroup = m_notifications[i];
        appGroup->toggleFolding(true);
    }
    endResetModel();
    m_expandedApps.clear();
    updateFullCollapsed();
}

void NotifyModel::collapseData()
{
    m_isCollapse = true;
    collapseDataByAppName(QString());
    m_expandedApps.clear();
    updateFullCollapsed();
}

void NotifyModel::collapseDataByAppName(const QString &appName)
{
    beginResetModel();
    for (int i = 0; i < m_notifications.size(); i++) {
        ListItemPtr &appGroup = m_notifications[i];
        if (appName.isEmpty() || appGroup->appName() == appName) {
            appGroup->toggleFolding(true);
            m_expandedApps.remove(appName);
            updateFullCollapsed();
        }
    }
    endResetModel();
}

void NotifyModel::removeTimeOutNotify()
{
    QList<EntityPtr> notifyList;
    for (int i = m_notifications.size() - 1; i >= 0; i--) {
        const auto &data = m_notifications[i]->data();
        // it's timer order, so we can check last item rough first.
        Q_ASSERT(!data.isEmpty());
        if (!checkTimeOut(data.last(), OVERLAPTIMEOUT_7_DAY)) {
            continue;
        }

        auto iter = std::find_if(data.begin(), data.end(), [this](const EntityPtr &entity) {
            return checkTimeOut(entity, OVERLAPTIMEOUT_7_DAY);
        });
        if (iter == data.end()) {
            continue;
        }
        const int index = iter - data.begin();

        beginResetModel();
        notifyList << m_notifications[i]->take(index);
        if (index == 0) {
            m_notifications.removeAt(i);
            Q_EMIT appCountChanged();
        }
        endResetModel();
    }

    for (int j = 0; j < notifyList.size(); j++) {
        m_database->removeOne(QString::number(notifyList[j]->id()));
    }
}

void NotifyModel::cacheData(EntityPtr entity)
{
    if (m_view->isVisible()) {
        m_cacheList.push_front(entity);
        if (!m_freeTimer->isActive()) {
            m_freeTimer->start();
        }
    } else {
        addNotify(entity);
    }
}

void NotifyModel::freeData()
{
    if (!m_notifications.isEmpty() && m_notifications.first()->appName() == m_cacheList.first()->appName()) {
        m_view->createAddedAnimation(m_cacheList.first(), getAppData(m_cacheList.first()->appName()));
    } else {
        addNotify(m_cacheList.first());
    }
    m_cacheList.removeFirst();
    if (m_cacheList.isEmpty()) {
        m_freeTimer->stop();
    }
}

bool NotifyModel::isAppTopping(const QString &appName) const
{
    // we use local cache instead of `settings` in most scenarios.
    const auto item = getAppData(appName);
    return isAppTopping(item);
}

bool NotifyModel::isAppTopping(const ListItemPtr &appItem) const
{
    if (appItem->hasAppTopping())
        return appItem->isAppTopping();

    const bool res = m_settings->getAppSetting(appItem->appName(), AbstractNotifySetting::SHOWNOTIFICATIONTOP).toBool();
    appItem->setAppTopping(res);
    return res;
}

void NotifyModel::setAppTopping(const QString &appName, bool isTopping)
{
    m_settings->setAppSetting(appName, AbstractNotifySetting::SHOWNOTIFICATIONTOP, isTopping);
}

bool NotifyModel::isCollapse(const QString &appName) const
{
    return getAppData(appName)->isCollapse();
}

bool NotifyModel::fullCollapsed() const
{
    return m_fullCollapsed;
}

void NotifyModel::updateFullCollapsed()
{
    auto value = m_isCollapse && m_expandedApps.empty();
    if (value == m_fullCollapsed) {
        return;
    }
    m_fullCollapsed = value;
    Q_EMIT fullCollapsedChanged(value);
}

bool NotifyModel::isCollapse() const
{
    return m_isCollapse;
}

void NotifyModel::refreshAppTopping()
{
    beginResetModel();
    sortNotifications();
    endResetModel();
}

void NotifyModel::initData()
{
    if (m_database == nullptr)  return;
    QList<EntityPtr> notifications = m_database->getAllNotify();

    std::sort(notifications.begin(), notifications.end(), [](const EntityPtr& ptr1,const EntityPtr& ptr2) {
        return ptr1->ctime().toLongLong() > ptr2->ctime().toLongLong();
    });

    foreach (auto notify, notifications) {
        if(!checkTimeOut(notify, OVERLAPTIMEOUT_7_DAY)) {
            addAppData(notify);
        } else {
            m_database->removeOne(QString::number(notify->id()));
        }
    }
    qDebug() << "initData(): Notification count:" << notifications.count()
             << ", App's count:" << m_notifications.count();

    sortNotifications();
}

void NotifyModel::initConnect()
{
    connect(m_database, &AbstractPersistence::RecordAdded, this, &NotifyModel::cacheData);
    connect(m_freeTimer, &QTimer::timeout, this, &NotifyModel::freeData);
    connect(m_view, &NotifyListView::addedAniFinished, this, &NotifyModel::addNotify);
    connect(m_view, &NotifyListView::removeAniFinished, this, &NotifyModel::removeNotify);
    connect(m_view, &NotifyListView::expandAniFinished, this, &NotifyModel::expandDataByAppName);
    connect(m_view, &NotifyListView::refreshItemTime, this, &NotifyModel::removeTimeOutNotify);
    connect(m_settings, &AbstractNotifySetting::appSettingChanged, this, &NotifyModel::onReceivedAppInfoChanged);
}

void NotifyModel::onReceivedAppInfoChanged(const QString &id, uint item, QVariant var)
{
    bool changed = false;
    if (item == AbstractNotifySetting::SHOWNOTIFICATIONTOP) {
        for (int i = 0; i < m_notifications.size(); i++) {
            if (m_notifications[i]->appName() == id) {
                m_notifications[i]->setAppTopping(var.toBool());
                changed = true;
                break;
            }
        }
    }
    if (changed)
        sortNotifications();
}

void NotifyModel::addAppData(EntityPtr entity)
{
    if (contains(entity->appName())) {
        for (int i = 0; i < m_notifications.size(); i++) {
            const ListItemPtr &appGroup = m_notifications[i];
            if (appGroup->appName() == entity->appName()) {
                appGroup->push(entity);
                m_notifications.move(i, 0);
                break;
            }
        }
    } else {
        ListItemPtr appGroup = std::make_shared<ListItem>(entity->appName());
        appGroup->push(entity);
        m_notifications.append(appGroup);
        Q_EMIT appCountChanged();
    }
}

EntityPtr NotifyModel::getEntityByRow(int row) const
{
    Q_ASSERT(row < rowCount(QModelIndex()));

    for (int i = 0, index = 0; i < showCount(); ++i) {
        const auto &item = m_notifications[i];
        const int nextIndex = index + countOfEachTitle(item->isCollapse()) + item->showCount();
        if (nextIndex <= row) {
            index = nextIndex;
            continue;
        }

        if (index == row) {
            if (item->isCollapse())
                return item->showFirst();

            return item->title();
        }
        return item->showAt(row - index - countOfEachTitle(item->isCollapse()));
    }
    Q_UNREACHABLE();
}

bool NotifyModel::checkTimeOut(EntityPtr ptr, int sec)
{
    QDateTime t = QDateTime::fromMSecsSinceEpoch(ptr->ctime().toLongLong());
    return t.secsTo(QDateTime::currentDateTime()) > sec;
}

bool NotifyModel::contains(const QString &appName)
{
    return std::any_of(m_notifications.begin(), m_notifications.end(),
                       [this, appName](const ListItemPtr &item) {
        return item->appName() == appName;
    });
}

int NotifyModel::showCount() const
{
    return m_isCollapse ? qMin(NotifyShowMaxCount, m_notifications.count()) : m_notifications.count();
}

int NotifyModel::countOfEachTitle(const bool collapse) const
{
    return collapse ? 0 : 1;
}

void NotifyModel::sortNotifications()
{
    std::sort(m_notifications.begin(), m_notifications.end(), [ this ](const ListItemPtr &item1,const ListItemPtr &item2) {
        const bool item1IsTopping = isAppTopping(item1);
        const bool item2IsTopping = isAppTopping(item2);
        // using ctime when topping is same, otherwise using topping.
        if (item1IsTopping == item2IsTopping)
            return item1->title()->ctime().toLongLong() > item2->title()->ctime().toLongLong();

        return item1IsTopping;
    });
}

ListItem::ListItem(const QString &appName)
{
    m_title = std::make_shared<NotificationEntity>(appName);
    m_title->setIsTitle(true);
}

void ListItem::push(const EntityPtr &entity)
{
    const qlonglong ctime = entity->ctime().toLongLong();
    auto iter = std::find_if(m_data.begin(), m_data.end(), [ctime](const EntityPtr &entity) {
        return entity->ctime().toLongLong() < ctime;
    });

    resetShowLastHideCount();
    if (iter == m_data.end()) {
        m_data.append(entity);
    } else {
        m_data.insert(iter, entity);
    }
    updateShowLastHideCount();
    updateShowTitleTime();
}

void ListItem::remove(const EntityPtr &entity)
{
    auto index = m_data.indexOf(entity);
    if (index >= 0 && index < showCount()) {
        resetShowLastHideCount();
        m_data.removeOne(entity);

        if (showCount() > 0) {
            updateShowLastHideCount();
            updateShowTitleTime();
        }
    }
}

QList<EntityPtr> ListItem::take()
{
    QList<EntityPtr> tmp;
    tmp.swap(m_data);
    return tmp;
}

QList<EntityPtr> ListItem::take(const int from)
{
    Q_ASSERT(0 <= from && from < count());

    QList<EntityPtr> tmp = m_data.mid(from);
    auto iter = m_data.begin() + from;
    while (iter != m_data.end()) {
        resetShowLastHideCount();
        iter = m_data.erase(iter);
        updateShowLastHideCount();
        updateShowTitleTime();
    }
    return tmp;
}

void ListItem::toggleFolding(const bool collapse)
{
    resetShowLastHideCount();
    m_isCollapse = collapse;
    if (m_isCollapse) {
        updateShowLastHideCount();
    } else {
        resetShowLastHideCount();
    }
}

void ListItem::resetShowLastHideCount()
{
    if (showCount() > 0) {
        showLast()->setHideCount(0);
    }
}

void ListItem::updateShowLastHideCount()
{
    if (showCount() > 0) {
        showLast()->setHideCount(overlapCount());
    }
}

void ListItem::updateShowTitleTime()
{
    if (showCount() > 0) {
        m_title->setTime(showFirst()->ctime());
    }
}
