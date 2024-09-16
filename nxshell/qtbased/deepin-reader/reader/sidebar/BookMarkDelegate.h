// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BOOKMARKDELEGATE_H
#define BOOKMARKDELEGATE_H

#include <DStyledItemDelegate>

DWIDGET_USE_NAMESPACE

/**
 * @brief The BookMarkDelegate class
 * 书签目录代理类
 */
class BookMarkDelegate : public DStyledItemDelegate
{
public:
    explicit BookMarkDelegate(QAbstractItemView *parent = nullptr);

protected:
    /**
     * @brief paint
     * 书签目录节点绘制
     * @param painter
     * @param option
     * @param index
     */
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QAbstractItemView *m_parent = nullptr;
};

#endif // BOOKMARKDELEGATE_H
