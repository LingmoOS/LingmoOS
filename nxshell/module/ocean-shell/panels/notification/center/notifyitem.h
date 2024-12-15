// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QVariantMap>
#include "notifyentity.h"

namespace notifycenter {
enum NotifyType {
    Normal = 0,
    Overlap,
    Group
};
/**
 * @brief The AppNotifyItem class
 */
using namespace notification;
class AppNotifyItem : public QObject
{
    Q_OBJECT
public:
    AppNotifyItem(const NotifyEntity &entity);

    void setEntity(const NotifyEntity &entity);
    NotifyEntity entity() const;

    virtual NotifyType type() const;
    QString appName() const;
    QString appId() const;
    qint64 id() const;
    QString time() const;
    void updateTime();
    bool strongInteractive() const;
    QString contentIcon() const;

    QString defaultAction() const;
    QVariantList actions() const;
    void updateActions();
    void updateStrongInteractive();

    void refresh();

    bool pinned() const;
    void setPinned(bool newPinned);

protected:
    QString m_appId;
    QString m_time;
    QVariantList m_actions;
    QString m_defaultAction;
    NotifyEntity m_entity;
    bool m_pinned = false;
    bool m_strongInteractive = false;
};

class OverlapAppNotifyItem : public AppNotifyItem
{
public:
    OverlapAppNotifyItem(const NotifyEntity &entity);
    virtual NotifyType type()const override;
    // source 3 -> overlap 2, 2 -> overlap 1, 1-> 0.
    void updateCount(int source);
    int count() const;
    bool isEmpty() const;

public:
    static const int EmptyCount = 0;
    static const int FullCount = 2;

private:
    // overlap count
    int m_count = 1;
};

class AppGroupNotifyItem : public AppNotifyItem
{
public:
    AppGroupNotifyItem(const QString &appName);
    virtual NotifyType type() const override;

    void updateLastEntity(const NotifyEntity& entity);
    NotifyEntity lastEntity() const;

private:
    NotifyEntity m_lastEntity;
};
}
