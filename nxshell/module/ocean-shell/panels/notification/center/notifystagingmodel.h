// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAbstractItemModel>
#include <QObject>
#include <QtQml/qqml.h>

#include "notifyitem.h"
#include "dataaccessor.h"

namespace notifycenter {
/**
 * @brief The NotifyStagingModel class
 */
using namespace notification;
class NotifyStagingModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
public:
    enum NotifyRole {
        NotifyItemType = Qt::UserRole + 1,
        NotifyId,
        NotifyAppId,
        NotifyAppName,
        NotifyIconName,
        NotifyActions,
        NotifyDefaultAction,
        NotifyTime,
        NotifyTitle,
        NotifyContent,
        NotifyStrongInteractive,
        NotifyContentIcon,
        NotifyOverlapCount,
        NotifyContentRowCount
    };
    NotifyStagingModel(QObject *parent = nullptr);

    Q_INVOKABLE void closeNotify(qint64 id);
    Q_INVOKABLE void invokeNotify(qint64 id, const QString &actionId);
    Q_INVOKABLE void open();
    Q_INVOKABLE void close();

public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QHash<int, QByteArray> roleNames() const override;
    virtual void timerEvent(QTimerEvent *event) override;

    int overlapCount() const;
    void updateOverlapCount(int count);
    void updateContentRowCount(int rowCount);

private slots:
    void push(const NotifyEntity &entity);
    void replace(const NotifyEntity &entity);
    void doEntityReceived(qint64 id);
    void onEntityClosed(qint64 id);

private:
    void remove(qint64 id);
    void updateTime();
    NotifyEntity notifyById(qint64 id) const;

private:
    QList<AppNotifyItem *> m_appNotifies;
    const int BubbleMaxCount{3};
    const int OverlayMaxCount{2};
    int m_refreshTimer = -1;
    DataAccessor *m_accessor = nullptr;
    int m_overlapCount = 0;
    int m_contentRowCount = 6;
};
}
