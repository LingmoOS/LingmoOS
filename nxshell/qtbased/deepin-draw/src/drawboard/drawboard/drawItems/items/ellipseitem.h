// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CGRAPHICSELLIPSEITEM_H
#define CGRAPHICSELLIPSEITEM_H
#include "rectitem.h"
#include <QPainterPath>

class DRAWLIB_EXPORT EllipseItem : public RectBaseItem
{
public:
    explicit EllipseItem(PageItem *parent = nullptr);
    explicit EllipseItem(qreal x, qreal y, qreal w, qreal h, PageItem *parent = nullptr);

    int type() const override;

    SAttrisList attributions() override;

    /**
     * @brief loadGraphicsUnit 加载图元数据
     * @return
     */
    void loadUnit(const Unit &data) override;

    /**
     * @brief getGraphicsUnit 返回图元数据
     * @return
     */
    Unit getUnit(int reson) const override;

protected:

    /**
     * @brief inSideShape 定义了内部形状
     * @return
     */
    QPainterPath calOrgShapeBaseRect(const QRectF &rect) const override;
};

#endif // CGRAPHICSELLIPSEITEM_H
