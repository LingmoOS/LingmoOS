// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CGRAPHICSTRIANGLEITEM_H
#define CGRAPHICSTRIANGLEITEM_H

#include "rectitem.h"
#include <QPainterPath>

class DRAWLIB_EXPORT TriangleItem : public RectBaseItem
{
public:
    explicit TriangleItem(PageItem *parent = nullptr);
    explicit TriangleItem(const QRectF &rect, PageItem *parent = nullptr);
    explicit TriangleItem(qreal x, qreal y, qreal w, qreal h, PageItem *parent = nullptr);

    SAttrisList attributions() override;

    /**
     * @brief type 图元的类型
     * @return
     */
    int type() const override;

    /**
     * @brief loadGraphicsUnit 加载图元数据
     * @return
     */
    void loadUnit(const Unit &data) override;

    /**
     * @brief getGraphicsUnit 得到图元数据
     * @return
     */
    Unit getUnit(int reson) const override;

protected:
    /**
      * @brief selfOrgShape 图元的原始形状（rect类图元不包括边线）
      */
    QPainterPath calOrgShapeBaseRect(const QRectF &rect) const override;

private:
    void calcPoints_helper(QVector<QPointF> &outVector, qreal offset = 0.0) const;


    QPolygonF polyForPen;
};

#endif // CGRAPHICSTRIANGLEITEM_H
