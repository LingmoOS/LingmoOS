// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"

#include <QObject>

namespace notification {

class NotificationProxy : public QObject
{
    Q_OBJECT
public:
    explicit NotificationProxy(QObject *parent = nullptr);

    enum ClosedReason {
        Expired = 1,
        Dismissed = 2,
        Closed = 3,
        Unknown = 4,
        Action = 5,
        Processed = 6,
        NotProcessedYet,
    };

    bool isValid() const;

    bool replaceNotificationBubble(bool replace);
    void handleBubbleEnd(int type, int id);
    void handleBubbleEnd(int type, int id, const QVariantMap &bubbleParams);
    void handleBubbleEnd(int type, int id, const QVariantMap &bubbleParams, const QVariantMap &selectedHints);

Q_SIGNALS:
    void ShowBubble(const QString &appName, uint replacesId,
                    const QString &appIcon, const QString &summary,
                    const QString &body, const QStringList &actions,
                    const QVariantMap hints, int expireTimeout,
                    const QVariantMap bubbleParams);

private:
    bool m_valid = false;
};

}
