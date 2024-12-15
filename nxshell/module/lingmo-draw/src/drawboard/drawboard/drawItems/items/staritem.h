// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CGRAPHICSPOLYGONALSTARITEM_H
#define CGRAPHICSPOLYGONALSTARITEM_H


#include "rectitem.h"
#include <QPainterPath>

class DRAWLIB_EXPORT StarItem : public RectBaseItem
{
public:
    explicit StarItem(int anchorNum = 5, int innerRadius = 50, PageItem *parent = nullptr);
    explicit StarItem(int anchorNum, int innerRadius, const QRectF &rect, PageItem *parent = nullptr);
    explicit StarItem(int anchorNum, int innerRadius, qreal x, qreal y, qreal w, qreal h,
                      PageItem *parent = nullptr);


    SAttrisList attributions() override;
    void  setAttributionVar(int attri, const QVariant &var, int phase) override;

    int type() const override;

    void setAnchorNum(int num, bool preview = false);
    int anchorNum() const;

    void setInnerRadius(int radius, bool preview = false);
    int  innerRadius() const;

    void loadUnit(const Unit &data) override;
    Unit getUnit(int reson) const override;

protected:
    QPainterPath calOrgShapeBaseRect(const QRectF &rect) const override;
private:
    int m_anchorNum[2] = {5, 5};     //描点数
    int m_innerRadius[2] = {50, 50}; //内接圆半径
    bool m_preview[2] = {false, false};

    QRectF _tempRecordScalRectF;

private:
    void calcPolygon_helper(const QRectF &baseRect, QPolygonF &outPolygon, int n, qreal offset = 0.0)const;
};

#endif // CGRAPHICSPOLYGONALSTARITEM_H
