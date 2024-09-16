// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "panel.h"
#include <QQuickItem>

namespace notification {

class BubbleItem;
class BubbleModel;
class NotificationProxy;
class NotificationPanel : public DS_NAMESPACE::DPanel
{
    Q_OBJECT
    Q_PROPERTY(bool visible READ visible NOTIFY visibleChanged FINAL)
    Q_PROPERTY(BubbleModel *bubbles READ bubbles CONSTANT FINAL)
public:
    explicit NotificationPanel(QObject *parent = nullptr);
    ~NotificationPanel();

    virtual bool load() override;
    virtual bool init() override;

    bool visible() const;
    BubbleModel *bubbles() const;

public Q_SLOTS:
    void defaultActionInvoke(int bubbleIndex);
    void actionInvoke(int bubbleIndex, const QString &actionId);
    void close(int bubbleIndex);
    void delayProcess(int bubbleIndex);

Q_SIGNALS:
    void visibleChanged();

private Q_SLOTS:
    void onBubbleCountChanged();
    void onShowBubble(const QString &appName, uint replaceId,
                      const QString &appIcon, const QString &summary,
                      const QString &body, const QStringList &actions,
                      const QVariantMap hints, int expireTimeout,
                      const QVariantMap bubbleParams);

    void onBubbleTimeout();
private:
    void setVisible(const bool visible);
    BubbleItem *bubbleItem(int index);

private:
    bool m_visible = false;
    BubbleModel *m_bubbles = nullptr;
    NotificationProxy *m_interproxy = nullptr;
};

}
