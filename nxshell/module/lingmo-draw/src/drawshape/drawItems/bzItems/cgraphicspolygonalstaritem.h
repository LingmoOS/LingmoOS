// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CGRAPHICSPOLYGONALSTARITEM_H
#define CGRAPHICSPOLYGONALSTARITEM_H


#include "cgraphicsrectitem.h"
#include <QPainterPath>

class CGraphicsPolygonalStarItem : public CGraphicsRectItem
{
public:
    explicit CGraphicsPolygonalStarItem(int anchorNum = 0, int innerRadius = 0, CGraphicsItem *parent = nullptr);
    explicit CGraphicsPolygonalStarItem(int anchorNum, int innerRadius, const QRectF &rect, CGraphicsItem *parent = nullptr);
    explicit CGraphicsPolygonalStarItem(int anchorNum, int innerRadius, qreal x, qreal y, qreal w, qreal h,
                                        CGraphicsItem *parent = nullptr);


    DrawAttribution::SAttrisList attributions() override;

    void  setAttributionVar(int attri, const QVariant &var, int phase) override;

    /**
     * @brief type  图元类型
     * @return
     */
    int type() const override;

    /**
     * @brief setRect 设置图元在自身坐标系的包围矩形
     * @return
     */
    void setRect(const QRectF &rect) override;

    /**
     * @brief updatePolygonalStar 刷新设置锚点数和内元半径
     * @return
     */
    void updatePolygonalStar(int anchorNum, int innerRadius);

    /**
     * @brief setAnchorNum 设置锚点数
     * @return
     */
    void setAnchorNum(int num, bool preview = false);

    /**
     * @brief anchorNum 锚点数(多少个角的星星)
     * @return
     */
    int anchorNum() const;

    /**
     * @brief 内圆半径 （内角到中心的距离）
     * @return
     */
    void setInnerRadius(int radius, bool preview = false);

    /**
     * @brief 内圆半径 （内角到中心的距离）
     * @return
     */
    int innerRadius() const;

    /**
     * @brief loadGraphicsUnit 加载图元数据
     * @return
     */
    void loadGraphicsUnit(const CGraphicsUnit &data) override;

    /**
     * @brief loadGraphicsUnit 图元的数据
     * @return
     */
    CGraphicsUnit getGraphicsUnit(EDataReason reson) const override;

    void updateShape() override;

protected:
    /**
      * @brief selfOrgShape 图元的原始形状（rect类图元不包括边线）
      */
    QPainterPath getSelfOrgShape() const override;

    QPainterPath getPenStrokerShape() const override;

    /**
     * @brief shape 返回图元的外形状
     */
    QPainterPath getShape() const override;

    /**
     * @brief shape 返回真实显示的图元的外形状()
     */
    QPainterPath getTrulyShape() const override;

private:
    int m_anchorNum[2] = {5, 5};     //描点数
    int m_innerRadius[2] = {50, 50}; //内接圆半径
    bool m_preview[2] = {false, false};

    QRectF _tempRecordScalRectF;

private:
    void calcPolygon_helper(QPolygonF &outPolygon, int n, qreal offset = 0.0)const;
};

#endif // CGRAPHICSPOLYGONALSTARITEM_H
