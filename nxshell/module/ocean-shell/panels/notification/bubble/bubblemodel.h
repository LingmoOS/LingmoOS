// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"

#include <QAbstractListModel>

class QTimer;

namespace notification {

class BubbleItem;
class BubbleModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(qint64 delayRemovedBubble READ delayRemovedBubble WRITE setDelayRemovedBubble NOTIFY delayRemovedBubbleChanged FINAL)
public:
    enum {
        AppName = Qt::UserRole + 1,
        Id,
        Body,
        Summary,
        IconName,
        Level,
        CTime,
        TimeTip,
        BodyImagePath,
        OverlayCount,
        DefaultAction,
        Actions,
        Urgency,
        ContentRowCount
    } BubbleRole;

    explicit BubbleModel(QObject *parent = nullptr);
    ~BubbleModel() override;

public:
    void push(BubbleItem *bubble);

    BubbleItem *replaceBubble(BubbleItem *bubble);
    bool isReplaceBubble(const BubbleItem *bubble) const;

    QList<BubbleItem *> items() const;

    Q_INVOKABLE void remove(int index);
    void remove(const BubbleItem *bubble);
    BubbleItem *removeById(qint64 id);
    void clear();

    BubbleItem *bubbleItem(int bubbleIndex) const;

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int displayRowCount() const;
    int overlayCount() const;

    qint64 delayRemovedBubble() const;
    void setDelayRemovedBubble(qint64 newDelayRemovedBubble);

signals:
    void delayRemovedBubbleChanged();

private:
    void updateBubbleCount(int count);
    int replaceBubbleIndex(const BubbleItem *bubble) const;
    void updateLevel();
    void updateBubbleTimeTip();
    void updateContentRowCount(int rowCount);

private:
    QTimer *m_updateTimeTipTimer = nullptr;
    QList<BubbleItem *> m_bubbles;
    int BubbleMaxCount{3};
    int m_contentRowCount{6};
    const int OverlayMaxCount{2};
    QList<qint64> m_delayBubbles;
    qint64 m_delayRemovedBubble{-1};
    const int DelayRemovBubbleTime{1000};
};

}
