// Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASICLISTDELEGATE_H
#define BASICLISTDELEGATE_H

#include <QAbstractItemDelegate>
#include <QApplication>
#include <QIcon>
#include <QImageReader>

namespace def {

namespace widgets {

QPixmap loadPixmap(const QString &path);

class BasicListDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    explicit BasicListDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

} // namespace widgets

} // namespace def

#endif // BASICLISTDELEGATE_H
