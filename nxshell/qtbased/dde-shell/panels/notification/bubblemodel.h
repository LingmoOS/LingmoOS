// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"

#include <QAbstractListModel>

namespace notification {

class BubbleItem : public QObject
{
    Q_OBJECT
public:
    explicit BubbleItem(QObject *parent = nullptr);
    explicit BubbleItem(const QString &text, const QString &title, const QString &iconName, QObject *parent = nullptr);

    QString text() const;
    QString title() const;
    QString iconName() const;
    QString appName() const;
    int level() const;
    int id() const;
    int replaceId() const;

    void setLevel(int newLevel);
    void setParams(const QString &appName, int id, const QStringList &actions,
                   const QVariantMap hints, int replaceId, const int timeout,
                   const QVariantMap bubbleParams);

    QVariantMap toMap() const;

    bool hasDisplayAction() const;
    bool hasDefaultAction() const;
    QString defaultActionText() const;
    QString defaultActionId() const;
    QString firstActionText() const;
    QString firstActionId() const;
    QStringList actionTexts() const;
    QStringList actionIds() const;

Q_SIGNALS:
    void levelChanged();
    void timeout();

private:
    int defaultActionIdIndex() const;
    int defaultActionTextIndex() const;
    QStringList displayActions() const;
    QString displayText() const;

private:
    QString m_text;
    QString m_title;
    QString m_iconName;
    QString m_appName;
    int m_id = 0;
    QStringList m_actions;
    QVariantMap m_hints;
    int m_replaceId;
    int m_timeout = 0;
    QString m_ctime;
    QVariantMap m_extraParams;

private:
    int m_level = 0;
    const int TimeOutInterval{5000};
};

class BubbleModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum {
        Text = Qt::UserRole + 1,
        Title,
        IconName,
        Level,
        OverlayCount,
        hasDefaultAction,
        hasDisplayAction,
        FirstActionText,
        FirstActionId,
        DefaultActionId,
        ActionTexts,
        ActionIds,
    } BubbleRule;

    explicit BubbleModel(QObject *parent = nullptr);
    ~BubbleModel();

    void push(BubbleItem *bubble);
    BubbleItem *replaceBubble(BubbleItem *bubble);
    bool isReplaceBubble(BubbleItem *bubble) const;
    void clear();
    QList<BubbleItem *> items() const;
    Q_INVOKABLE void remove(int index);
    void remove(BubbleItem *bubble);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int displayRowCount() const;
    int overlayCount() const;

private:
    int replaceBubbleIndex(BubbleItem *bubble) const;
    void updateLevel();
    QList<BubbleItem *> m_bubbles;
    const int BubbleMaxCount{3};
    const int LastBubbleMaxIndex{BubbleMaxCount - 1};
    const int OverlayMaxCount{2};
    const int NoReplaceId{0};
};

}
