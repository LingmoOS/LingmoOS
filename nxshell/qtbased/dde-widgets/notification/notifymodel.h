// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NotifyModel_H
#define NotifyModel_H

#include "notification/constants.h"

#include <QAbstractListModel>
#include <QPointer>
#include <QListView>

#include <memory>

#define OVERLAPTIMEOUT_4_HOUR       (4 * 60 * 60)
#define OVERLAPTIMEOUT_7_DAY        (7 * 24 * 60 * 60)
#define TIMEOUT_CHECK_TIME          1000
static int AppShowMaxCount = 1;
static int NotifyShowMaxCount = 3;

class QTimer;
class PersistenceObserver;
class AbstractPersistence;
class NotifyListView;
class NotifySettingsObserver;

/**
 * @brief App 表示的数据及操作
 */
class ListItem {
public:
    explicit ListItem(const QString &appName);
    ListItem() = delete;
    ~ListItem()
    {
        if (m_isAppTopping) {
            delete m_isAppTopping;
            m_isAppTopping = nullptr;
        }
    }

    // 按时间顺序插入到队列中
    void push(const EntityPtr &entity);
    void remove(const EntityPtr &entity);
    QList<EntityPtr> take();
    QList<EntityPtr> take(const int from);
    // 展开或折叠此应用数据
    void toggleFolding(const bool collapse);
    inline bool isCollapse() const
    {
        return m_isCollapse;
    }
    inline EntityPtr at(const int index) const
    {
        return m_data.at(index);
    }
    inline int count() const
    {
        return m_data.count();
    }
    inline bool isEmpty() const
    {
        return m_data.isEmpty();
    }
    inline const QList<EntityPtr> &data() const
    {
        return m_data;
    }
    inline int showCount() const
    {
        return m_isCollapse ? qMin(AppShowMaxCount, m_data.count()) : m_data.count();
    }
    inline  int showLastRow() const
    {
        return showCount() - 1;
    }
    // 最后一个显示项
    inline EntityPtr showLast() const
    {
        Q_ASSERT(showCount() > 0);
        return m_data.at(showCount() - 1);
    }
    inline EntityPtr showAt(const int index) const
    {
        Q_ASSERT(0 <= index && index < showCount());
        return m_data.at(index);
    }
    // 第一个显示项
    inline EntityPtr showFirst() const
    {
        return m_data.at(0);
    }
    inline int hideCount() const
    {
        return m_data.count() - showCount();
    }
    inline EntityPtr hideFirst() const
    {
        return hideAt(0);
    }
    inline EntityPtr hideAt(const int index) const
    {
        return at(index + showCount());
    }
    inline int overlapCount() const
    {
        return qMin(2, hideCount());
    }
    inline EntityPtr title() const
    {
        return m_title;
    }
    inline QString appName() const
    {
        Q_ASSERT(m_title);
        return m_title->appName();
    }
    inline bool hasAppTopping() const
    {
        return m_isAppTopping;
    }
    inline bool isAppTopping() const
    {
        if (m_isAppTopping)
            return *m_isAppTopping;

        return false;
    }
    inline void setAppTopping(const bool topping)
    {
        if (!m_isAppTopping) {
            m_isAppTopping = new bool(topping);
        } else {
            *m_isAppTopping = topping;
        }
    }
private:
    void resetShowLastHideCount();
    void updateShowLastHideCount();
    void updateShowTitleTime();

private:
    EntityPtr m_title;
    bool m_isCollapse = true;
    using TimerQueue = QList<EntityPtr>;
    TimerQueue m_data;
    bool *m_isAppTopping = nullptr;
};
using ListItemPtr = std::shared_ptr<ListItem>;

class NotifyModel : public QAbstractListModel
{
    Q_OBJECT
public:
    NotifyModel(QObject *parent = nullptr, AbstractPersistence *database = nullptr, NotifyListView *view = nullptr);
    NotifyListView *view() { return m_view; }
    ListItemPtr getAppData(const QString &appName) const;

public:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    int remainNotificationCount() const;
    int rowOfFirstNotification() const;

public slots:
    void addNotify(EntityPtr entity);                   // 添加一条通知，并更新视图
    void removeNotify(EntityPtr entity);                // 删除一条通知，并更新视图
    void removeAppGroup(const QString &appName);               // 移除一组通知
    void removeAllData();                               // 清除所有通知
    void expandDataByAppName(const QString &appName);   // 展开通知
    void expandData();                               // 展开通知
    void collapseData();                                // 折叠通知
    void collapseDataByAppName(const QString &appName); // 折叠通知
    void removeTimeOutNotify();                         // 移除超过7天的通知
    void cacheData(EntityPtr entity);                   // 缓存暂未处理的通知
    void freeData();                                    // 将暂未处理的通知顺序添加到通知中心
    void refreshAppTopping();
    bool isAppTopping(const QString &appName) const;
    bool isAppTopping(const ListItemPtr &appItem) const;
    void setAppTopping(const QString &appName, bool isTopping);
    bool isCollapse() const;
    bool isCollapse(const QString &appName) const;
    bool fullCollapsed() const;
    void updateFullCollapsed();

Q_SIGNALS:
    void dataChanged();                                 // 数据库有添加数据时发送该信号
    void removedNotif();                                // 删除通知完成信号
    void appCountChanged();
    void fullCollapsedChanged(bool);

private Q_SLOTS:
    void onReceivedAppInfoChanged(const QString &id, uint item, QVariant var);

private:
    void initData();                                    // 初始化数据
    void initConnect();                                 // 初始化信号
    void addAppData(EntityPtr entity);                  // 添加一条数据
    EntityPtr getEntityByRow(int row) const;            // 根据row获取数据
    bool checkTimeOut(EntityPtr ptr, int sec);          // 检查通知是否超时
    bool contains(const QString &appName);
    int showCount() const;
    int countOfEachTitle(const bool collapse) const;
    void sortNotifications();

private:
    NotifyListView *m_view = nullptr;
    AbstractPersistence *m_database = nullptr;
    QList<ListItemPtr> m_notifications;                    //外层为app,内层为此app的消息
    QList<EntityPtr> m_cacheList;
    QTimer *m_freeTimer;
    bool m_isCollapse = true;
    bool m_fullCollapsed = true;
    QSet<QString> m_expandedApps;
    NotifySettingsObserver *m_settings = nullptr;
};

#endif // NotifyModel_H
