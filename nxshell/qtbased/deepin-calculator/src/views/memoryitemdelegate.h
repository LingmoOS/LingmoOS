// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TESTITEMDELEGATE_H
#define TESTITEMDELEGATE_H

#include "memorylistwidget.h"
#include "memoryitemwidget.h"

#include <QStyledItemDelegate>
#include <QPainter>

/**
 * @brief 内存ListWidget委托
 */
class MemoryItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit MemoryItemDelegate(QObject *parent = nullptr);
    ~MemoryItemDelegate() override;

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // TESTITEMDELEGATE_H
