// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETAILVIEWDELEGATE_H
#define DETAILVIEWDELEGATE_H

#include <QStyledItemDelegate>
#include <QObject>

/**
 * @brief The DetailViewDelegate class
 * 表格代理类
 */
class DetailViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DetailViewDelegate(QObject *parent);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *, const QStyleOptionViewItem &,
                          const QModelIndex &) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

protected:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;

};

#endif // DETAILVIEWDELEGATE_H
