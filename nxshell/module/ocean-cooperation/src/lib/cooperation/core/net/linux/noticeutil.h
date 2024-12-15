// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NOTICEUTIL_H
#define NOTICEUTIL_H

#include <QObject>
#include <QTimer>

class QDBusInterface;
namespace cooperation_core {

class NoticeUtil : public QObject
{
    Q_OBJECT

public:
    explicit NoticeUtil(QObject *parent = nullptr);
    ~NoticeUtil();

    void notifyMessage(const QString &title, const QString &body, const QStringList &actions, QVariantMap hitMap, int expireTimeout);
    void closeNotification();
    void resetNotifyId();

Q_SIGNALS:
    void ActionInvoked(const QString &action);
    void onConfirmTimeout();

private Q_SLOTS:
    void onActionTriggered(uint replacesId, const QString &action);

private:
    void initNotifyConnect();

private:
    QTimer confirmTimer;

    QDBusInterface *notifyIfc { nullptr };
    uint recvNotifyId { 0 };
};

}   // namespace cooperation_core

#endif   // NOTICEUTIL_H
