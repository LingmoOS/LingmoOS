// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CGRAPHICSRECTITEM_H
#define CGRAPHICSRECTITEM_H
#include "csizehandlerect.h"
#include "vectoritem.h"
#include "pagecontext.h"
#include <QtGlobal>
#include <QPainterPath>
#include <QVector>
#include <QRectF>

class DRAWLIB_EXPORT RectBaseItem : public VectorItem
{
public:
    explicit RectBaseItem(PageItem *parent = nullptr);
    explicit RectBaseItem(const QRectF &rect, PageItem *parent = nullptr);
    explicit RectBaseItem(qreal x, qreal y, qreal w, qreal h, PageItem *parent = nullptr);
    ~RectBaseItem()  override;

    QRectF rect() const;
    void setRect(const QRectF &rect);

    void doScaling(PageItemScalEvent *event) override;
    bool testScaling(PageItemScalEvent *event) override;

    void loadUnit(const Unit &data) override;
    Unit getUnit(int reson) const override;

protected:
    virtual QPainterPath calOrgShapeBaseRect(const QRectF &rect) const = 0;

protected:
    QPainterPath getOrgShape() const final;

protected:
    void loadGraphicsRectUnit(const RectUnitData &rectData);

private:
    QRectF _rct;
};

class RectItem: public RectBaseItem
{
public:
    using RectBaseItem::RectBaseItem;

    int type() const override;

    SAttrisList attributions() override;
    void  setAttributionVar(int attri, const QVariant &var, int phase) override;

    void setXYRadius(int xRedius, int yRedius, bool preview = false);
    int  getXRadius()const;
    void setRectRadius(QVariantList &radius, bool preview = false);
protected:
    void loadUnit(const Unit &ut) override;
    Unit getUnit(int reson) const override;

    QPainterPath calOrgShapeBaseRect(const QRectF &rect) const override;
    void paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option) override;
private:
    void calibrationRadius(qreal &left, qreal &right, qreal &leftBottom, qreal &rightBottom, const QRectF &rect)const;
private:
    int  m_xRadius = 5;
    int  m_yRadius = 5;
    int  m_radiusForPreview = 0;
    bool m_isPreviewRadius  = false;

    int m_leftRadius = 0;
    int m_rightRadius = 0;
    int m_leftBottomRadius = 0;
    int m_rightBottomRadius = 0;
    bool m_bSameRadiusModel = true;
};

#endif // CGRAPHICSRECTITEM_H
