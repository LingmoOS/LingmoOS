// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAbstractItemModel>
#include <QObject>
#include <QtQml/qqml.h>
#include "notifyitem.h"

namespace notifycenter {
class NotifyAccessor;
/**
 * @brief The NotifyModel class
 */
using namespace notification;
class NotifyModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString dataInfo READ dataInfo NOTIFY dataInfoChanged FINAL)
    Q_PROPERTY(bool collapse READ collapse NOTIFY collapseChanged FINAL)
public:
    enum NotifyRole {
        NotifyItemType = Qt::UserRole + 1,
        NotifyId,
        NotifyAppName,
        NotifyAppId,
        NotifyIconName,
        NotifyActions,
        NotifyDefaultAction,
        NotifyTime,
        NotifyTitle,
        NotifyContent,
        NotifyPinned,
        NotifyStrongInteractive,
        NotifyContentIcon,
        NotifyOverlapCount,
        NotifyContentRowCount
    };
    NotifyModel(QObject *parent = nullptr);

    Q_INVOKABLE void expandApp(int row);
    Q_INVOKABLE void collapseApp(int row);
    Q_INVOKABLE void remove(qint64 id);
    Q_INVOKABLE void removeByApp(const QString &appName);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void collapseAllApp();
    Q_INVOKABLE void expandAllApp();

    Q_INVOKABLE void open();
    Q_INVOKABLE void close();

    Q_INVOKABLE void invokeAction(qint64 id, const QString &actionId);
    Q_INVOKABLE void pinApplication(const QString &appName, bool pin);

    QString dataInfo() const;

    bool collapse() const;
    void setCollapse(bool newCollapse);

signals:
    void dataInfoChanged();
    void collapseChanged();
    void countChanged();

public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QHash<int, QByteArray> roleNames() const override;
    virtual void timerEvent(QTimerEvent *event) override;
    virtual void sort(int column, Qt::SortOrder order) override;

private slots:
    void doEntityReceived(qint64 id);
    void onCountChanged();

private:
    int notifyCount(const NotifyType &type) const;
    int notifyCount(const QString &appName) const;
    int notifyCount(const QString &appName, const NotifyType &type) const;
    int firstNotifyIndex(const QString &appName, const NotifyType &type) const;

    void updateTime();
    void append(const NotifyEntity &entity);
    QList<QString> fetchLastApps(int maxCount = -1) const;
    NotifyEntity greaterNotifyEntity(const AppNotifyItem *notifyItem) const;
    bool greaterNotify(const AppNotifyItem *item1, const AppNotifyItem *item2) const;
    bool greaterNotify(const NotifyEntity &item1, const NotifyEntity &item2) const;
    int lastNotifyIndex(const NotifyEntity &entity) const;
    int lastNotifyIndex(const AppNotifyItem *item) const;
    void sortNotifies();
    void trayUpdateGroupLastEntity(const NotifyEntity &entity);
    void trayUpdateGroupLastEntity(const QString &appName);
    void updateCollapseStatus();
    void updateContentRowCount(int rowCount);

private:
    QList<AppNotifyItem *> m_appNotifies;
    QPointer<NotifyAccessor> m_accessor;
    int m_refreshTimer = -1;
    bool m_collapse = false;
    int m_contentRowCount = 6;
};
}
