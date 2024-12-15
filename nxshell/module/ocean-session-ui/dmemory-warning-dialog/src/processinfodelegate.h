// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESSINFODELEGATE_H
#define PROCESSINFODELEGATE_H

#include <QStyledItemDelegate>

class ProcessInfoDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ProcessInfoDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // PROCESSINFODELEGATE_H
