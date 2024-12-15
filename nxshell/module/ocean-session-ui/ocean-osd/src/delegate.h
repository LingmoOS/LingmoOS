// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DELEGATE_H
#define DELEGATE_H

#include <QStyledItemDelegate>

#include "common.h"

class AbstractOSDProvider;
/*!
 * \~chinese \class Delegate
 * \~chinese \brief 用来设置osd中item的样式
 */
class Delegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit Delegate(QWidget *parent = 0);

    void setProvider(AbstractOSDProvider *provider);

private:
    AbstractOSDProvider *m_provider;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // DELEGATE_H
