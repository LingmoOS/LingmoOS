// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "notifyentity.h"

#include <QObject>

namespace notification {

class NotifyEntity;
class BubbleItem : public QObject
{
    Q_OBJECT
public:

    explicit BubbleItem(QObject *parent = nullptr);
    explicit BubbleItem(const NotifyEntity &entity, QObject *parent = nullptr);

public:
    void setEntity(const NotifyEntity &entity);

public:
    qint64 id() const;
    uint bubbleId() const;
    QString appName() const;
    QString appIcon() const;
    QString summary() const;
    QString body() const;
    uint replacesId() const;
    bool isReplace() const;
    int urgency() const;
    QString bodyImagePath() const;
    qint64 ctime() const;

    QString defaultAction() const;
    QVariantList actions() const;
    void updateActions();

    int level() const;
    void setLevel(int level);

    QString timeTip() const;
    void setTimeTip(const QString &timeTip);

    bool enablePreview() const;
    void setEnablePreview(bool enable);

signals:
    void levelChanged();
    void timeTipChanged();

private:
    QString displayText() const;

private:
    NotifyEntity m_entity;
    int m_level = 0;
    int m_urgency = NotifyEntity::Normal;
    QString m_timeTip;
    bool m_enablePreview = true;
    QVariantList m_actions;
    QString m_defaultAction;
};

}


