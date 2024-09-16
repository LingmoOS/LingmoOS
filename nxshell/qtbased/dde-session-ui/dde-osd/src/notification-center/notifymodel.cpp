// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notifymodel.h"
#include "../notification/persistence.h"
#include "notifylistview.h"

#include <QDebug>
#include <QDateTime>
#include <QTimer>

NotifyModel::NotifyModel(QObject *parent, AbstractPersistence *database, NotifyListView *view)
    : QAbstractListModel(parent)
    , m_view(view)
    , m_database(static_cast<Persistence *>(database))
    , m_freeTimer(new QTimer(this))
{
    m_freeTimer->setInterval(AnimationTime + 100);
    initData();
    initConnect();
}

ListItem NotifyModel::getAppData(QString appName) const
{

    foreach (auto item, m_notifications) {
        if (item.appName == appName)
            return item;
    }
    Q_UNREACHABLE();
}

int NotifyModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    int count = 0;
    foreach (auto item, m_notifications) {
        count += item.showList.size();
    }
    count += m_notifications.size();
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
        return QVariant::fromValue(entity);
    }
    return QVariant();
}

Qt::ItemFlags NotifyModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        if (m_view != nullptr) m_view->openPersistentEditor(index);
        return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
    }
    return QAbstractListModel::flags(index);
}

void NotifyModel::addNotify(EntityPtr entity)
{
    beginResetModel();
    addAppData(entity);
    endResetModel();
}

void NotifyModel::removeNotify(EntityPtr entity)
{
    if (m_notifications.isEmpty())
        return;

    beginResetModel();
    for (int i = 0; i < m_notifications.size(); i++) {
        ListItem &AppGroup = m_notifications[i];
        if (AppGroup.appName == entity->appName()
                && AppGroup.showList.contains(entity)) {
            AppGroup.showList.removeOne(entity);

            if (AppGroup.showList.isEmpty() || (!AppGroup.hideList.isEmpty()
                    && !checkTimeOut(AppGroup.hideList.first(), OVERLAPTIMEOUT_4_HOUR))) {
                if (!AppGroup.showList.isEmpty()){
                    AppGroup.showList.last()->setHideCount(0);
                }
                if (!AppGroup.hideList.isEmpty()) {
                    AppGroup.showList.push_back(AppGroup.hideList.first());
                    AppGroup.hideList.pop_front();
                }
                if (!AppGroup.showList.isEmpty()) {
                    AppGroup.showList.last()->setHideCount(AppGroup.hideList.size() > 2 ? 2 : AppGroup.hideList.size());
                }
            }
            if (AppGroup.showList.isEmpty()) {
                m_notifications.removeAt(i);
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

void NotifyModel::removeAppGroup(QString appName)
{
    beginResetModel();
    if (m_notifications.isEmpty())
        return;
    for (int i = 0; i < m_notifications.size(); i++) {
        if (m_notifications[i].appName == appName) {
            m_notifications.removeAt(i);
        }
    }
    endResetModel();
    m_database->removeApp(appName);
}

void NotifyModel::removeAllData()
{
    beginResetModel();
    m_notifications.clear();
    endResetModel();
    m_database->removeAll();
}

void NotifyModel::expandData(QString appName)
{
    beginResetModel();
    for (int i = 0; i < m_notifications.size(); i++) {
        ListItem &AppGroup = m_notifications[i];
        if (AppGroup.appName == appName) {
            AppGroup.showList.last()->setHideCount(0);
            AppGroup.showList.append(AppGroup.hideList);
            AppGroup.hideList.clear();
        }
    }
    endResetModel();
}

void NotifyModel::collapseData()
{
    beginResetModel();
    for (int i = 0; i < m_notifications.size(); i++) {
        ListItem &AppGroup = m_notifications[i];
        int collapseRow;
        for(collapseRow = 0; collapseRow < AppGroup.showList.size(); collapseRow ++) {
            if (checkTimeOut(AppGroup.showList[collapseRow], OVERLAPTIMEOUT_4_HOUR) || collapseRow >= 3) {
                break;
            }
        }
        if (collapseRow == 0) {
            collapseRow = 1;
        }

        if (AppGroup.showList.size() == collapseRow) {
            continue;
        }
        QList<EntityPtr> showList = AppGroup.showList.mid(0, collapseRow);
        QList<EntityPtr> hideList = AppGroup.showList.mid(collapseRow,  AppGroup.showList.size() - collapseRow);
        AppGroup.showList = showList;
        AppGroup.hideList = hideList;
    }
    endResetModel();
}

void NotifyModel::removeTimeOutNotify()
{
    for (int i = 0; i < m_notifications.size(); i++) {
        ListItem &AppGroup = m_notifications[i];
        QList<EntityPtr> notifyList;
        if (checkTimeOut(AppGroup.showList.last(), OVERLAPTIMEOUT_7_DAY)) {
            beginResetModel();
            for (int j = 0; j < AppGroup.hideList.size(); j++) {
                m_database->removeOne(QString::number(AppGroup.hideList[j]->id()));
            }
            for (int j = 0; j < AppGroup.showList.size(); j++) {
                if(checkTimeOut(AppGroup.showList[j], OVERLAPTIMEOUT_7_DAY)) {
                    m_database->removeOne(QString::number(AppGroup.showList[j]->id()));
                } else {
                    notifyList.append(AppGroup.showList[j]);
                }
            }
            AppGroup.hideList = notifyList;
            if (notifyList.isEmpty()) {
                m_notifications.removeAt(i);
            }
            endResetModel();
            return;
        } else {
            for (int j = 0; j < AppGroup.hideList.size(); j++) {
                if(checkTimeOut(AppGroup.hideList[j], OVERLAPTIMEOUT_7_DAY)) {
                    m_database->removeOne(QString::number(AppGroup.hideList[j]->id()));
                } else {
                    notifyList.append(AppGroup.hideList[j]);
                }
            }

            if (AppGroup.hideList.size() != notifyList.size()) {
                beginResetModel();
                AppGroup.hideList = notifyList;
                endResetModel();
            }
        }
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
    if (!m_notifications.isEmpty() && m_notifications.first().appName == m_cacheList.first()->appName()) {
        m_view->createAddedAnimation(m_cacheList.first(), getAppData(m_cacheList.first()->appName()));
    } else {
        addNotify(m_cacheList.first());
    }
    m_cacheList.removeFirst();
    if (m_cacheList.isEmpty()) {
        m_freeTimer->stop();
    }
}

void NotifyModel::initData()
{
    if (m_database == nullptr)  return;
    QList<EntityPtr> notifications = m_database->getAllNotify();

    std::sort(notifications.begin(), notifications.end(), [=](const EntityPtr& ptr1,const EntityPtr& ptr2) {
        return ptr1->ctime().toLongLong() < ptr2->ctime().toLongLong();
    });

    foreach (auto notify, notifications) {
        if(!checkTimeOut(notify, OVERLAPTIMEOUT_7_DAY)) {
            addAppData(notify);
        } else {
            m_database->removeOne(QString::number(notify->id()));
        }
    }

}

void NotifyModel::initConnect()
{
    connect(m_database, &Persistence::RecordAdded, this, &NotifyModel::cacheData);
    connect(m_freeTimer, &QTimer::timeout, this, &NotifyModel::freeData);
    connect(m_view, &NotifyListView::addedAniFinished, this, &NotifyModel::addNotify);
    connect(m_view, &NotifyListView::removeAniFinished, this, &NotifyModel::removeNotify);
    connect(m_view, &NotifyListView::expandAniFinished, this, &NotifyModel::expandData);
    connect(m_view, &NotifyListView::refreshItemTime, this, &NotifyModel::removeTimeOutNotify);
}

void NotifyModel::addAppData(EntityPtr entity)
{
    bool isNewApp = true;
    bool entityTimeout = checkTimeOut(entity, OVERLAPTIMEOUT_4_HOUR);
    for (int i = 0; i < m_notifications.size(); i++) {
        ListItem &AppGroup = m_notifications[i];
        if (AppGroup.appName == entity->appName()) {
            isNewApp = false;
            if (entityTimeout) { //超时通知添加到隐藏列表
                if (checkTimeOut(AppGroup.showList.last(), OVERLAPTIMEOUT_4_HOUR)) {
                    AppGroup.showList.push_front(entity);
                    AppGroup.hideList.push_front(AppGroup.showList.last());
                    AppGroup.showList.removeLast();
                }
            } else if ((!entityTimeout && AppGroup.showList.size() == 3)
                       || checkTimeOut(AppGroup.showList.first(), OVERLAPTIMEOUT_4_HOUR)) { //通知未超时，显示列表已满
                AppGroup.showList.last()->setHideCount(0);
                AppGroup.showList.push_front(entity);
                AppGroup.hideList.push_front(AppGroup.showList.last());
                AppGroup.showList.removeLast();
                AppGroup.lastTimeStamp = entity->ctime();
            } else {
                AppGroup.showList.push_front(entity);
                AppGroup.lastTimeStamp = entity->ctime();
            }
            m_notifications.move(i, 0);
        }
    }

    if (isNewApp) {
        ListItem AppGroup;
        AppGroup.appName = entity->appName();
        AppGroup.lastTimeStamp = entity->ctime();
        AppGroup.showList.push_front(entity);
        m_notifications.append(AppGroup);
    }

    std::sort(m_notifications.begin(), m_notifications.end(), [=](const ListItem & item1, const ListItem & item2) {
        return item1.lastTimeStamp.toLongLong() > item2.lastTimeStamp.toLongLong();
    });
}

EntityPtr NotifyModel::getEntityByRow(int row) const
{
    Q_ASSERT(row <= rowCount(QModelIndex()) - 1);
    int index = 0;
    for (int i = 0; i < m_notifications.size(); ++i) {
        const auto &item = m_notifications[i];
        item.showList.last()->setHideCount(item.hideList.size() > 2 ? 2 : item.hideList.size());
        if (index == row) {
            EntityPtr ptr = std::make_shared<NotificationEntity>(item.appName);
            ptr->setTime(item.lastTimeStamp);
            ptr->setIsTitle(true);
            ptr->setCurrentIndex(row);
            return ptr;
        }
        index++;
        for (int j = 0; j < item.showList.size(); ++j) {
            if (index == row) {
                item.showList[j]->setCurrentIndex(row);
                return item.showList.at(j);
            }
            index ++;
        }
    }
    Q_UNREACHABLE();
}

bool NotifyModel::checkTimeOut(EntityPtr ptr, int sec)
{
    QDateTime t = QDateTime::fromMSecsSinceEpoch(ptr->ctime().toLongLong());
    return t.secsTo(QDateTime::currentDateTime()) > sec;
}

