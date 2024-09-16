// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NotifyModel_H
#define NotifyModel_H

#include "../notification/constants.h"

#include <QAbstractListModel>
#include <QPointer>
#include <QListView>

#include <memory>

#define OVERLAPTIMEOUT_4_HOUR       (4 * 60 * 60)
#define OVERLAPTIMEOUT_7_DAY        (7 * 24 * 60 * 60)
#define TIMEOUT_CHECK_TIME          1000

class QTimer;
class Persistence;
class AbstractPersistence;
class NotifyListView;

typedef struct{
    QString appName;                // 应用名称
    QString lastTimeStamp;          // 此组应用最新的时间组
    QList<EntityPtr> showList;      // 显示列表
    QList<EntityPtr> hideList;      // 隐藏列表
} ListItem;

class NotifyModel : public QAbstractListModel
{
    Q_OBJECT
public:
    NotifyModel(QObject *parent = nullptr, AbstractPersistence *database = nullptr, NotifyListView *view = nullptr);
    NotifyListView *view() { return m_view; }
    ListItem getAppData(QString appName) const;

public:
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

public slots:
    void addNotify(EntityPtr entity);                   // 添加一条通知，并更新视图
    void removeNotify(EntityPtr entity);                // 删除一条通知，并更新视图
    void removeAppGroup(QString appName);               // 移除一组通知
    void removeAllData();                               // 清除所有通知
    void expandData(QString appName);                   // 展开通知
    void collapseData();                                // 折叠通知
    void removeTimeOutNotify();                         // 移除超过7天的通知
    void cacheData(EntityPtr entity);                   // 缓存暂未处理的通知
    void freeData();                                    // 将暂未处理的通知顺序添加到通知中心

Q_SIGNALS:
    void dataChanged();                                 // 数据库有添加数据时发送该信号
    void removedNotif();                                // 删除通知完成信号

private:
    void initData();                                    // 初始化数据
    void initConnect();                                 // 初始化信号
    void addAppData(EntityPtr entity);                  // 添加一条数据
    EntityPtr getEntityByRow(int row) const;            // 根据row获取数据
    bool checkTimeOut(EntityPtr ptr, int sec);          // 检查通知是否超时

private:
    NotifyListView *m_view = nullptr;
    Persistence *m_database = nullptr;
    QList<ListItem> m_notifications;                    //外层为app,内层为此app的消息
    QList<EntityPtr> m_cacheList;
    QTimer *m_freeTimer;
};

#endif // NotifyModel_H
