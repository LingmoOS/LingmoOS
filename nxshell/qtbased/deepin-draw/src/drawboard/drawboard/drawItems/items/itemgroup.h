// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CGRAPHICSITEMGROUP_H
#define CGRAPHICSITEMGROUP_H

#include "csizehandlerect.h"
#include "rectitem.h"
#include <QPainterPath>

/**
 * @brief The CGraphicsItemGroup class 组合图元管理类
 * 所有的图元操作都通过该类执行。
 */

class DRAWLIB_EXPORT GroupItem : public RectBaseItem
{
    //Q_OBJECT
public:
    explicit GroupItem(const QString &nam = "", PageItem *parent = nullptr);

    ~GroupItem();
    SAttrisList attributions() override;
    QString name() const;
    void setName(const QString &name);

    bool  isTopGroup() const;

    void addToGroup(PageItem *item);
    void addToGroup(const QList<PageItem *> &items);
    void removeFromGroup(PageItem *item);
    void removeFromGroup(const QList<PageItem *> &items);

    void clear();

    QPainterPath calOrgShapeBaseRect(const QRectF &rect) const override;

    void blockUpdateBoundingRect(bool b);
    void updateBoundingRect(bool force = false);

    int count();

    int notGroupCount()const;

    QList<PageItem *>  items(bool recursiveFind = false) const;
    QList<PageItem *>  getNotGroupItems(bool recursiveFind = false) const;
    QList<GroupItem *> getGroupItems(bool recursiveFind = false) const;

    int type() const override;

    bool testScaling(PageItemScalEvent *event) override;
    void doScaling(PageItemScalEvent *event) override;
    void doScalEnd(PageItemScalEvent *event) override;

    void loadUnit(const Unit &data) override;
    Unit getUnit(int reson) const override;

protected:
    bool isPosPenetrable(const QPointF &posLocal) override;
    bool isRectPenetrable(const QRectF &rectLocal) override;
    void paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option) override;
    QVariant pageItemChange(int changeType, const QVariant &value) override;
    bool pageItemChangeFilter(PageItem *item, int changeType, QVariant &value) override;

    PRIVATECLASS(GroupItem)
};
Q_DECLARE_METATYPE(GroupItem *)

#endif // CGRAPHICSITEMGROUP_H
