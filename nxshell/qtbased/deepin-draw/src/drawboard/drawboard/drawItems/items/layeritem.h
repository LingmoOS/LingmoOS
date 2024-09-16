// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LAYERITEM_H
#define LAYERITEM_H
#include <rasteritem.h>

class PageScene;
class DRAWLIB_EXPORT LayerItem: public RasterItem
{
public:
    LayerItem(PageScene *scene = nullptr);

    int  type() const override;

    void addItem(PageItem *pItem);
    void removeItem(PageItem *pItem);
    void clear();

    QList<PageItem *> items() const;
    QList<PageItem *> pageItems(ESortItemTp tp = EDesSort) const;
    QList<PageItem *> pageItems(const QPointF &pos, ESortItemTp tp = EDesSort) const;
    QList<PageItem *> pageItems(const QRectF &rect, ESortItemTp tp = EDesSort) const;

    void blockAssignZValue(bool b);
    bool isAssignZBlocked()const;

private:
    void loadUnit(const Unit &data) override;
    Unit getUnit(int reson) const override;

    bool contains(const QPointF &point) const override;
    bool isPosPenetrable(const QPointF &posLocal) override;
    bool isRectPenetrable(const QRectF &rectLocal) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
    void updateShape() override;
    QVariant pageItemChange(int changeType, const QVariant &value) override;

    PRIVATECLASS(LayerItem)
};
Q_DECLARE_METATYPE(LayerItem *)

#endif // LAYERITEM_H
