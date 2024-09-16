// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemgroup.h"
#include "pagecontext.h"
#include "pageview.h"
#include "pageitem.h"
#include "penitem_compatible.h"
#include "drawboardtoolmgr.h"
#include "textitem.h"

#include "cgraphicsitemevent.h"
#include "pagescene.h"
#include "pageitem_p.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsScene>
#include <QDebug>
#include <QStyleOptionGraphicsItem>
#include <QMatrix4x4>
#include <QGraphicsTransform>

REGISTITEMCLASS(GroupItem, GroupItemType);
class GroupItem::GroupItem_private
{
public:
    explicit GroupItem_private(GroupItem *qq): q(qq) {}

    GroupItem *q;

    bool   _isCancelable = true;

    QString _name;

    bool _blockedCalBounding = false;
};
GroupItem::~GroupItem()
{
}

SAttrisList GroupItem::attributions()
{
    auto children = childPageItems();
    if (children.isEmpty()) {
        return SAttrisList();
    }

    SAttrisList result = children.first()->attributions();
    foreach (auto p, children) {
        result = result.insected(p->attributions());
    }

    // 对于旋转属性，特殊处理，使用群组图元当前的角度值
    auto itr = std::find_if(result.begin(), result.end(), [](const SAttri & attr) {
        return bool(ERotProperty == attr.attri);
    });
    if (result.end() != itr) {
        (*itr).var = drawRotation();
    } else {
        // 由直线组成的群组图元无旋转属性，单独添加旋转属性
        result << SAttri(ERotProperty, drawRotation());
    }

    return result;
}

GroupItem::GroupItem(const QString &nam, PageItem *parent)
    : /*QObject(nullptr),*/ RectBaseItem(parent), GroupItem_d(new GroupItem_private(this))
{
    setName(nam);
    //d_GroupItem()->group = new QGraphicsItemGroup(this);
}

QString GroupItem::name() const
{
    return d_GroupItem()->_name;
}

void GroupItem::setName(const QString &name)
{
    d_GroupItem()->_name = name;
}

bool GroupItem::isTopGroup() const
{
    return pageGroup() == nullptr;
}

void GroupItem::clear()
{
    if (count() == 0)
        return;

    {
        blockUpdateBoundingRect(true);
        //QSignalBlocker blocker(this);
        auto lists = childPageItems();
        foreach (PageItem *item, lists) {
            item->setPageGroup(nullptr);
        }
        blockUpdateBoundingRect(false);
    }
    d_PageItem()->_roteAgnel = 0;
    this->PageItem::resetTransform();
    //this->setPageGroup(nullptr);

    //childPageItems().clear();
    updateBoundingRect();

    //emit childrenChanged(d_PageItem()->childrens);
}

QPainterPath GroupItem::calOrgShapeBaseRect(const QRectF &rect) const
{
    QPainterPath path;
    path.addRect(rect);
    return path;
}

void GroupItem::blockUpdateBoundingRect(bool b)
{
    d_GroupItem()->_blockedCalBounding = b;
}

void GroupItem::updateBoundingRect(bool force)
{
    //setRect(d_GroupItem()->getGroup()->boundingRect());

    if (operatingType() == 3)
        return;

    if (d_GroupItem()->_blockedCalBounding)
        return;

    QRectF rect(0, 0, 0, 0);

    auto chidren = childPageItems();
    if (chidren.size() > 1) {
        //没有旋转过,那么重新获取大小
        if (transform().isIdentity() || force) {
            auto items =  getNotGroupItems(true);
            foreach (QGraphicsItem *item, items) {
                PageItem *pItem = dynamic_cast<PageItem *>(item);
                if (pItem != nullptr && pItem->type() != BlurType) {
                    rect = rect.united(pItem->mapRectToScene(pItem->selectedRectangle()));
                }
            }
            this->setTransformOriginPoint(rect.center());
            this->setRotation(0);
            if (force) {
                resetTransform();
                d_PageItem()->_roteAgnel = 0;
            }
            rect = mapFromScene(rect).boundingRect();

        }
    } else if (chidren.size() == 1) {
        PageItem *pItem = chidren.first();

        //不存在节点的图元就需要多选图元进行管理
        if (pItem->nodeCount() == 0 || pageScene()->isGroupItem(pItem)) {

            rect = pItem->selectedRectangle();

            this->setTransformOriginPoint(pItem->transformOriginPoint());

            this->setRotation(pItem->rotation());

            this->setPos(pItem->pos());

            this->setTransform(pItem->transform());

            d_PageItem()->_roteAgnel = pItem->drawRotation();
        }
    }

    //qWarning()<<"set group rect ======== "<<rect<<rect.size();
    setRect(rect);
}

int GroupItem::count()
{
    return childPageItems().count();
}

int GroupItem::notGroupCount() const
{
    return getNotGroupItems(true).count();
}

QList<PageItem *> GroupItem::items(bool recursiveFind) const
{
    if (!recursiveFind)
        return childPageItems();
    QList<PageItem *> result;
    foreach (auto p, childPageItems()) {
        result.append(p);
        if (p->isPageGroup()) {
            GroupItem *pGroup = static_cast<GroupItem *>(p);
            result.append(pGroup->items(recursiveFind));
        }
    }
    return result;
}

QList<PageItem *> GroupItem::getNotGroupItems(bool recursiveFind) const
{
    QList<PageItem *> result;
    foreach (auto p, childPageItems()) {
        if (p->isBzItem()) {
            result.append(p);
        } else if (recursiveFind && p->isPageGroup()) {
            GroupItem *pGroup = static_cast<GroupItem *>(p);
            result.append(pGroup->getNotGroupItems(recursiveFind));
        }
    }
    return result;
}

QList<GroupItem *> GroupItem::getGroupItems(bool recursiveFind) const
{
    QList<GroupItem *> result;
    foreach (auto p, childPageItems()) {
        if (p->isPageGroup()) {
            GroupItem *pGroup = static_cast<GroupItem *>(p);
            result.append(pGroup);

            if (recursiveFind)
                result.append(pGroup->getGroupItems(recursiveFind));
        }
    }
    return result;
}

void GroupItem::addToGroup(PageItem *item)
{
    // COMBINE
    bool ok;
    QTransform itemTransform = item->itemTransform(this, &ok);

    if (!ok) {
        qWarning("GroupItem::addToGroup: could not find a valid transformation from item to group coordinates");
        return;
    }
    QTransform newItemTransform(itemTransform);
    item->setPos(mapFromItem(item, item->x(), item->y()));

    addChild(item);

    // removing position from translation component of the new transform
    if (!item->pos().isNull())
        newItemTransform *= QTransform::fromTranslate(-item->x(), -item->y());

    // removing additional transformations properties applied with itemTransform()
    QPointF origin = item->transformOriginPoint();
    QMatrix4x4 m;
    QList<QGraphicsTransform *> transformList = item->transformations();
    for (int i = 0; i < transformList.size(); ++i)
        transformList.at(i)->applyTo(&m);
    newItemTransform *= m.toTransform().inverted();
    newItemTransform.translate(origin.x(), origin.y());
    newItemTransform.rotate(-item->rotation());
    newItemTransform.scale(1 / item->scale(), 1 / item->scale());
    newItemTransform.translate(-origin.x(), -origin.y());

    item->setTransform(newItemTransform);
}

void GroupItem::removeFromGroup(PageItem *item)
{
    PageItem *newParent = parentPageItem();

    // COMBINE
    bool ok;
    QTransform itemTransform;
    if (newParent)
        itemTransform = item->itemTransform(newParent, &ok);
    else
        itemTransform = item->sceneTransform();

    QPointF oldPos = item->mapToItem(newParent, 0, 0);
    item->setParentPageItem(newParent);
    item->setPos(oldPos);

    // removing position from translation component of the new transform
    if (!item->pos().isNull())
        itemTransform *= QTransform::fromTranslate(-item->x(), -item->y());

    // removing additional transformations properties applied
    // with itemTransform() or sceneTransform()
    QPointF origin = item->transformOriginPoint();
    QMatrix4x4 m;
    QList<QGraphicsTransform *> transformList = item->transformations();
    for (int i = 0; i < transformList.size(); ++i)
        transformList.at(i)->applyTo(&m);
    itemTransform *= m.toTransform().inverted();
    itemTransform.translate(origin.x(), origin.y());
    itemTransform.rotate(-item->rotation());
    itemTransform.scale(1 / item->scale(), 1 / item->scale());
    itemTransform.translate(-origin.x(), -origin.y());

    // ### Expensive, we could maybe use dirtySceneTransform bit for optimization

    item->setTransform(itemTransform);
}

void GroupItem::removeFromGroup(const QList<PageItem *> &items)
{
    blockUpdateBoundingRect(true);
    foreach (auto it, items) {
        removeFromGroup(it);
    }
    blockUpdateBoundingRect(false);
    updateBoundingRect(true);
}

void GroupItem::addToGroup(const QList<PageItem *> &items)
{
    //QSignalBlocker blocker(this);
    blockUpdateBoundingRect(true);
    foreach (auto it, items) {
        addToGroup(it);
    }
    blockUpdateBoundingRect(false);
    updateBoundingRect(true);

    //emit childrenChanged(d_PageItem()->childrens);
}

int GroupItem::type() const
{
    return GroupItemType;
}

bool GroupItem::testScaling(PageItemScalEvent *event)
{
    //当组合大小是无效时(隐藏时),那么默认绕过组合的判定,比如单选直线的情况,因为直线拥有自己的节点,那么就会隐藏选择框,隐藏的方式就是设置rect为无效
    if (!orgRect().isValid())
        return true;

    bool accept = true;
    QTransform trans = event->trans();
    QRectF rct = this->orgRect();
    QPointF pos1 = trans.map(rct.topLeft());
    QPointF pos4 = trans.map(rct.bottomRight());

    QRectF wantedRect(pos1, pos4);
    event->setMayResultPolygon(this->PageItem::mapToScene(wantedRect));
    accept = wantedRect.isValid();
    return accept;
}

void GroupItem::doScaling(PageItemScalEvent *event)
{
    QTransform trans = event->trans();
    QRectF rct = this->orgRect();
    QPointF pos1 = trans.map(rct.topLeft());
    QPointF pos4 = trans.map(rct.bottomRight());
    setRect(QRectF(pos1, pos4));
}

void GroupItem::doScalEnd(PageItemScalEvent *event)
{
    Q_UNUSED(event)
//    QRectF unitRect;
//    auto children = childPageItems();
//    for (int i = 0; i < children.size(); ++i) {
//        auto item = children.at(i);
//        auto rectInGroup = item->mapRectToItem(this, item->orgRect());
//        if (!orgRect().adjusted(-1, -1, 1, 1).contains(rectInGroup)) {
//            updateBoundingRect(true);
//            return;
//        }
//        unitRect = unitRect.united(rectInGroup);
//    }
//    if (unitRect != orgRect())
//        updateBoundingRect(true);
}

//bool GroupItem::contains(const QPointF &point) const
//{
//    if (groupType() == ESelectGroup)
//        return false;
//    return PageItem::contains(point);
//}

//bool GroupItem::isPosPenetrable(const QPointF &posLocal)
//{
//    return true;
//}

//bool GroupItem::isRectPenetrable(const QRectF &rectLocal)
//{
//    return true;
//}

//QRectF ItemGroup::selfRect() const
//{
//    return m_boundingRect;
//}

void GroupItem::loadUnit(const Unit &data)
{
    GroupUnitData i = data.data.data.value<GroupUnitData>();
    setName(i.name);
    loadVectorData(data.head);
    setRect(data.head.rect);
}

Unit GroupItem::getUnit(int reson) const
{
    Unit unit;

    unit.usage = reson;

    unit.head.dataType = this->type();
    unit.head.dataLength = sizeof(RectUnitData);
    unit.head.pos    = this->pos();
    unit.head.rotate = this->drawRotation();
    unit.head.zValue = this->zValue();
    unit.head.trans  = this->transform();
    unit.head.rect   = this->orgRect();

    GroupUnitData data;
    data.name = this->name();
    data.isCancelAble = d_GroupItem()->_isCancelable;
    unit.data.data.setValue<GroupUnitData>(data);

    return unit;
}

bool GroupItem::isPosPenetrable(const QPointF &posLocal)
{
    return pageGroup() != nullptr;
}

bool GroupItem::isRectPenetrable(const QRectF &rectLocal)
{
    return pageGroup() != nullptr;
}

void GroupItem::paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    paintMutBoundingLine(painter, option);
}

extern void SelectionItem_NotSelectHelper(PageItem *p);
QVariant GroupItem::pageItemChange(int changeType, const QVariant &value)
{
    if (changeType == PageItemChildAdded) {
        updateBoundingRect(true);

        PageItem *child = static_cast<PageItem *>(value.value<QGraphicsItem *>());
        child->installChangeFilter(this);
        SelectionItem_NotSelectHelper(child);
    } else if (changeType == PageItemChildRemoved) {
        updateBoundingRect(true);

        PageItem *child = static_cast<PageItem *>(value.value<QGraphicsItem *>());
        child->removeChangeFilter(this);
    }

    return RectBaseItem::pageItemChange(changeType, value);
}

bool GroupItem::pageItemChangeFilter(PageItem *item, int changeType, QVariant &value)
{
    if (item->pageGroup() == this) {
        if (changeType == PageItemRectChanged || changeType == ItemPositionChange) {
            //updateBoundingRect(true);
        }
    }
    return RectBaseItem::pageItemChangeFilter(item, changeType, value);
}

