// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "panel.h"
#include "dataaccessor.h"
#include <appletproxy.h>

#include <QQuickItem>

namespace notification {

class NotifyEntity;
class BubbleModel;
class BubbleItem;

class BubblePanel : public DS_NAMESPACE::DPanel
{
    Q_OBJECT
    Q_PROPERTY(bool visible READ visible NOTIFY visibleChanged FINAL)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged FINAL)
    Q_PROPERTY(BubbleModel *bubbles READ bubbles CONSTANT FINAL)
public:
    explicit BubblePanel(QObject *parent = nullptr);
    ~BubblePanel();

    bool load() override;
    bool init() override;

    bool visible() const;
    BubbleModel *bubbles() const;

    bool enabled() const;

public Q_SLOTS:
    void invokeAction(int bubbleIndex, const QString &actionId);
    void close(int bubbleIndex);
    void delayProcess(int bubbleIndex);
    void setEnabled(bool newEnabled);

Q_SIGNALS:
    void visibleChanged();

    void enabledChanged();

private Q_SLOTS:
    void onNotificationStateChanged(qint64 id, int processedType);
    void addBubble(qint64 id);
    void closeBubble(qint64 id);
    void onBubbleCountChanged();

private:
    void onBubbleExpired(BubbleItem *);
    void onActionInvoked(qint64 id, uint bubbleId, const QString &actionId);
    void onBubbleClosed(qint64 id, uint bubbleId, uint reason);
    void setVisible(const bool visible);
    bool enablePreview(const QString &appId) const;

    BubbleItem *bubbleItem(int index);

private:
    bool m_visible = false;
    BubbleModel *m_bubbles = nullptr;
    DS_NAMESPACE::DAppletProxy *m_notificationServer = nullptr;
    DataAccessor *m_accessor = nullptr;
    bool m_enabled = true;
};

}
