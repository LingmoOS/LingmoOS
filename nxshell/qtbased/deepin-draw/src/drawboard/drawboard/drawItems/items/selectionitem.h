// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SELECTIONITEM_H
#define SELECTIONITEM_H
#include <rectitem.h>

class PageScene;
class DRAWLIB_EXPORT SelectionItem: public QObject, public RectBaseItem
{
    Q_OBJECT
public:
    explicit SelectionItem(PageScene *scene);
    ~SelectionItem();

    SAttrisList attributions() override;
    void  setAttributionVar(int attri, const QVariant &var, int phase) override;

    int type()const override;

    void select(PageItem *p);
    void select(const QList<PageItem *> &items);

    void notSelect(PageItem *p);
    void notSelect(const QList<PageItem *> &items);
    void clear();

    QList<PageItem *> selectedItems() const;
    int selectedCount() const;

    PageItem *singleSelectedItem()const;
    void setBlockUpdateRect(bool bBlock);
    void operatingBegin(PageItemEvent *event) override;
    void operating(PageItemEvent *event) override;
    bool testOpetating(PageItemEvent *event) override;
    void operatingEnd(PageItemEvent *event) override;

signals:
    void selectionChanged(const QList<PageItem * > &selectedItems);

protected:
    bool contains(const QPointF &point) const override;

    void loadUnit(const Unit &data) override;
    Unit getUnit(int reson) const override;

    void updateRect();



    bool pageItemChangeFilter(PageItem *item, int changeType, QVariant &value) override;

    void paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option) override;
    QPainterPath calOrgShapeBaseRect(const QRectF &rect) const override;

    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

    PRIVATECLASS(SelectionItem)
};

class DRAWLIB_EXPORT SelectionItemNode: public HandleNode
{
    Q_OBJECT
public:
    using HandleNode::HandleNode;

protected:
    bool isVisbleCondition() const override;
};
#endif // SELECTIONITEM_H
