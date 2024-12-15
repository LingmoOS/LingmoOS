// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MASKITEM_H
#define MASKITEM_H
#include "pageitem.h"

class PageScene;
class MaskItem: public PageItem
{
public:
    MaskItem(PageScene *scene = nullptr);

    int  type() const override;

    /**
     * @brief type 图元的包裹矩形
     */
    QRectF itemRect() const override;

    /**
     * @brief setRect 设置蒙板矩形
     */
    void setRect(const QRectF &rect);

    /**
     * @brief rect 蒙板矩形
     */
    QRectF rect() const;

private:

    bool contains(const QPointF &point) const override;
    bool isPosPenetrable(const QPointF &posLocal) override;
    bool isRectPenetrable(const QRectF &rectLocal) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
    void updateShape() override;

    QRectF  m_rect;
};
Q_DECLARE_METATYPE(MaskItem *)

#endif // MASKITEM_H
