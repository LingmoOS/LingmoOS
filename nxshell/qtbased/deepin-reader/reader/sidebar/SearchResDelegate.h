// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHRESDELEGATE_H
#define SEARCHRESDELEGATE_H

#include <DStyledItemDelegate>

DWIDGET_USE_NAMESPACE
/**
 * @brief The SearchResDelegate class
 * 搜索代理
 */
class SearchResDelegate : public DStyledItemDelegate
{
public:
    explicit SearchResDelegate(QAbstractItemView *parent = nullptr);

protected:
    /**
     * @brief paint
     * 绘制事件
     * @param painter
     * @param option
     * @param index
     */
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    /**
     * @brief sizeHint
     * 节点大小
     * @param option
     * @param index
     * @return
     */
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QAbstractItemView *m_parent = nullptr;
};

#endif // SEARCHRESDELEGATE_H
