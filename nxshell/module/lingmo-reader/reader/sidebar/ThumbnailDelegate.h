// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEVIEWDELEGATE_H
#define IMAGEVIEWDELEGATE_H

#include <DStyledItemDelegate>

DWIDGET_USE_NAMESPACE
/**
 * @brief The ThumbnailDelegate class
 * 缩略图代理
 */
class ThumbnailDelegate : public DStyledItemDelegate
{
public:
    explicit ThumbnailDelegate(QAbstractItemView *parent = nullptr);

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
    /**
     * @brief drawBookMark
     * 绘制书签图标
     * @param painter
     * @param rect
     * @param visible
     */
    void drawBookMark(QPainter *painter, const QRect &rect, bool visible) const;

private:
    QAbstractItemView *m_parent = nullptr;
};

#endif // IMAGEVIEWDELEGATE_H
