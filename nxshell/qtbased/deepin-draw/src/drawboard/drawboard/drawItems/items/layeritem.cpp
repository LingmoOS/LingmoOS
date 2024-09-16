// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "layeritem.h"
#include "pagescene.h"
#include "pageitem_p.h"
//QList<PageItem *> m_items;
#include <QPainter>
REGISTITEMCLASS(LayerItem, LayerItemType)
class LayerItem::LayerItem_private
{
public:
    explicit LayerItem_private(LayerItem *qq): q(qq)
    {

    }
    void removeItem_helper(PageItem *pItem)
    {

    }

    int maxZ()
    {
        auto children = q->childPageItems();
        if (children.isEmpty())
            return 0;

        int max = children.first()->pageZValue();
        foreach (auto p, q->childPageItems()) {
            if (p->pageZValue() > max) {
                max = p->pageZValue();
            }
        }
        return max;
    }

    LayerItem *q;
    bool blockedAssginZ = false;
    //QList<PageItem *> items;
};
LayerItem::LayerItem(PageScene *scene):
    RasterItem(QImage(), EImageType, nullptr),
    LayerItem_d(new LayerItem_private(this))
{
    if (scene != nullptr) {
        scene->insertTopLayer(this);
        setRect(scene->sceneRect());
    } else {
        setRect(QRectF(0, 0, 1920, 1080));
    }
}

int LayerItem::type() const
{
    return LayerItemType;
}

void LayerItem::addItem(PageItem *pItem)
{
//    //1 step. nullptr return
//    if (pItem == nullptr)
//        return;

//    //2 step. 不能添加自己
//    if (pItem == this) {
//        return;
//    }

//    //3 step. 相同的layer什么都不用做
//    if (pItem->layer() == this)
//        return;

//    //自动检测pItem的变化，且析构时会触发notify变化
//    //PageItemNotifyBlocker blocker(pItem,true);

//    //4 step.如果pItem所处的场景和layer的场景不同，那么要从原场景中删除掉pItem，再加入到layer的场景中.
//    if (pItem->pageScene() != this->pageScene()) {
//        if (pItem->pageScene() != nullptr) {
//            pItem->pageScene()->removePageItem(pItem);
//        }

//        if (this->pageScene() != nullptr) {
//            //当前layer所处场景不为空那么添加到这个场景的这个layer就完成了
//            //(转交给了pageScene()的addPageItem完成;
//            //note:可能会担心引起循环，其实在addPageItem中会优先设置pItem的pageScene，因此在判断场景是否相同时(4 step一开始)会发现是相同的，所以不会引起循环)
//            pageScene()->addPageItem(pItem, this);
//            return;
//        }
//    }

//    //5 step.因为直系家庭成员必须是在一个layer中，如果pItem有parent那么进行这次layer变化后肯定将和parent的layer不同，所以需要将pItem的parent中抽出来
//    if (pItem->parentPageItem() != nullptr) {
//        pItem->setParentPageItem(nullptr);
//    }

//    //6 step.将pItem从原来的layer中删除(在第5步时如果pItem设置了自己的parent为空，那么setParentPageItem函数其实就会执行从layer中删掉pItem)
//    if (pItem->layer() != nullptr) {
//        pItem->layer()->removeItem(pItem);
//    }

//    //7 step.添加到这个layer中的列表中
//    d_LayerItem()->items.append(pItem);
//    pItem->d_PageItem()->setLayer_helper(this);

//    //8 给一个z值
//    pItem->setPageZValue(d_LayerItem()->maxZ() + 1);

//    //9 通知layer变化（包括其孩子结点）
//    pItem->d_PageItem()->notifyItemAndChildrenLayerChanged(this);


    //1 step. nullptr return
    if (pItem == nullptr)
        return;

    //2 step. 不能添加自己
    if (pItem == this) {
        return;
    }

    //3 step. 相同的layer什么都不用做
    if (pItem->layer() == this)
        return;

    pItem->setParentPageItem(this);
}

void LayerItem::removeItem(PageItem *pItem)
{
    pItem->setParentPageItem(this->parentPageItem());
}

void LayerItem::clear()
{
    foreach (auto p, items()) {
        removeItem(p);
    }
}

QList<PageItem *> LayerItem::items() const
{
    return childPageItems();
}

QList<PageItem *> LayerItem::pageItems(ESortItemTp tp) const
{
    return returnSortZItems(childPageItems(), tp);
}

QList<PageItem *> LayerItem::pageItems(const QPointF &pos, ESortItemTp tp) const
{
    QList<PageItem *> result;
    auto children = childPageItems();
    foreach (auto p, children) {
        auto posInp = p->mapFromItem(this, pos);
        if (p->itemRect().contains(posInp)) {
            result.append(p);
        }
    }
    return returnSortZItems(result, tp);
}

QList<PageItem *> LayerItem::pageItems(const QRectF &rect, ESortItemTp tp) const
{
    QList<PageItem *> result;
    auto children = childPageItems();
    foreach (auto p, children) {
        auto posInp = p->mapRectFromItem(this, rect);
        if (p->itemRect().intersects(posInp)) {
            result.append(p);
        }
    }
    return returnSortZItems(result, tp);
}

void LayerItem::blockAssignZValue(bool b)
{
    d_LayerItem()->blockedAssginZ = b;
}

bool LayerItem::isAssignZBlocked() const
{
    if (pageScene() == nullptr)
        return d_LayerItem()->blockedAssginZ;

    return pageScene()->isAssignZBlocked() || d_LayerItem()->blockedAssginZ;
}

void LayerItem::loadUnit(const Unit &data)
{
    Unit temp = data;
    LayerUnitData layerData = data.value<LayerUnitData>();
    temp.setValue<RasterUnitData>(layerData.rasterData);
    RasterItem::loadUnit(temp);
}

Unit LayerItem::getUnit(int reson) const
{
    Unit ut = RasterItem::getUnit(reson);
    LayerUnitData layerData;
    layerData.rasterData = ut.value<RasterUnitData>();
    ut.setValue<LayerUnitData>(layerData);
    return ut;
}
bool globel_LayerItem_ZAssignBLocked = false;
QVariant LayerItem::pageItemChange(int changeType, const QVariant &value)
{
    if (changeType == PageItemChildAdded) {
        if (!isAssignZBlocked() && !globel_LayerItem_ZAssignBLocked) {
            auto pItem = dynamic_cast<PageItem *>(value.value<QGraphicsItem *>());
            if (pItem != nullptr) {
                pItem->setPageZValue(d_LayerItem()->maxZ() + 1);
            } else {
                qWarning() << "layer add one null item? (check if layer add one not pageitme object)";
            }
        }
    }
    return RasterItem::pageItemChange(changeType, value);
}

bool LayerItem::contains(const QPointF &point) const
{
    return false;
}

bool LayerItem::isPosPenetrable(const QPointF &posLocal)
{
    return true;
}

bool LayerItem::isRectPenetrable(const QRectF &rectLocal)
{
    return true;
}

void LayerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setBrush(QBrush(/*QColor(248, 248, 251)*/Qt::white));
    painter->setPen(Qt::NoPen);
    painter->drawRect(orgRect());
    RasterItem::paint(painter, option, widget);
    //paintChidren(painter, returnSortZItems(childPageItems(), EAesSort), widget);
}

void LayerItem::updateShape()
{
//    QRectF rct;
//    if (scene() != nullptr) {
//        rct = scene()->sceneRect();
//    }
//    setRect(rct);
}
