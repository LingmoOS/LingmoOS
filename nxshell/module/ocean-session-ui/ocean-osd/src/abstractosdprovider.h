// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTOSDPROVIDER_H
#define ABSTRACTOSDPROVIDER_H

#include <QObject>
#include <QSize>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QListView>

#include "common.h"
/*!
 * \~chinese \class AbstractOSDProvider
 * \~chinese \brief 抽象一个osd类,当接收到不同通知时,用来展示不同的窗口,后缀带有Provider
 * \~chinese 的类都是继承于该类
 */
class AbstractOSDProvider : public QObject
{
    Q_OBJECT

public:
    typedef enum {
        ENABLE,
        DISABLE,
        HIDE
    } ModuleStatus;

    explicit AbstractOSDProvider(QObject *parent = nullptr);

    // match tell if we should handle the param.
    virtual bool match(const QString &param);

    // check whether the conditions are all met so we can show
    // our content.
    virtual bool checkConditions() const;

    // provide hint about how many space we will take.
    virtual QSize contentSize() const;

    virtual QMargins contentMargins() const;

    virtual QListView::Flow flow() const;

    virtual void highlightCurrent();
    virtual void highlightNext();

    virtual void sync();
    virtual void sync(const QModelIndex &index);

    virtual int currentRow();

    // model
    virtual int rowCount(const QModelIndex &parent) const = 0;
    virtual QVariant data(const QModelIndex &index, int role) const = 0;

    // delegate
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const = 0;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const = 0;

signals:
    void dataChanged();

protected:
    QStringList m_suitableParams;
    bool m_isVisible;
};

#endif // ABSTRACTOSDPROVIDER_H
