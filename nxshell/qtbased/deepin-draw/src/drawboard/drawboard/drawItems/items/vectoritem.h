// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VECTORITEM_H
#define VECTORITEM_H
#include "pageitem.h"

class DRAWLIB_EXPORT VectorItem: public PageItem
{
public:
    VectorItem(PageItem *parent = nullptr);

    SAttrisList attributions() override;
    void  setAttributionVar(int attri, const QVariant &var, int phase) override;

    QPen pen() const;
    QPen paintPen(Qt::PenJoinStyle style = Qt::MiterJoin) const;
    void setPen(const QPen &pen);
    void setPenColor(const QColor &c, bool isPreview = false);
    void setPenWidth(int w, bool isPreview = false);

    QBrush brush() const;
    QBrush paintBrush() const;
    void setBrush(const QBrush &brush);
    void setBrushColor(const QColor &c, bool isPreview = false);


    virtual QPainterPath getOrgShape() const = 0;

    QPainterPath penStrokerShape() const;

    QRectF orgRect() const final;
    QPainterPath orgShape() const final;
    QRectF itemRect() const final;
    QPainterPath itemShape() const final;

    void setCache(bool enable);
    bool isCached() const;

    void setAutoCache(bool autoCache, int autoCacheMs = 8);
    bool isAutoCache() const;

    QImage rasterSelf() const;

    bool isPosPenetrable(const QPointF &posLocal) override;
    bool isRectPenetrable(const QRectF &rectLocal) override;

    void updateShape() override;

    QPainterPath highLightPath() const override;

    bool isDrawFill();

    bool isDarwBorder();
    /**
     * @brief getGraphicsItemShapePathByOrg 根据画笔属性，把图元形状转为路径   此函数为Qt源码中自带的
     * @param path 形状路径
     * @param pen 画笔
     * @param penStrokerShape 为true表示返回orgPath的线条的填充路径，false表示返回orgPath的最外围路径
     * @param incW 线宽增量(可能的应用场景：虚拟提升线宽使更好选中)
     * @return  转换后的路径
     */
    static QPainterPath getPenStrokerPath(const QPainterPath &orgPath,
                                          const QPen &pen,
                                          bool penStrokerShape = false,
                                          const qreal incW = 0,
                                          bool doSimplified = true);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) final;
    void paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option) override;

    QVariant pageItemChange(int change, const QVariant &value) override;
    enum EPaintReson {EPaintForDynamic, EPaintForStatic, EPaintForNoCache = EPaintForDynamic, EPaintForCache = EPaintForStatic};

    void paintItemSelf(QPainter *painter, const QStyleOptionGraphicsItem *option, EPaintReson paintReson);

    void loadVectorData(const UnitHead &head);

protected:
    virtual QPainterPath getPenStrokerShape() const;
    virtual QPainterPath getOuterShape(const QPainterPath &orgPath, const QPainterPath &strokerPath) const;

    PRIVATECLASS(VectorItem)
};

#endif // VECTORITEM_H
