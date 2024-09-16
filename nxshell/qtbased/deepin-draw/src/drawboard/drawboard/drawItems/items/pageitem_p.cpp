// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pageitem_p.h"
#include "pagescene.h"
#include "cgraphicsitemevent.h"
#include "layeritem.h"
//#include "extendattribution.h"

bool PageItem::PageItem_private::paintSelectedBorderLine = true;
PageItem::PageItem_private::PageItem_private(PageItem *qp): q(qp)
{

}

QMap<int, QString> &PageItem::PageItem_private::registedClassNameMap()
{
    static QMap<int, QString> *s_map = nullptr;
    if (s_map == nullptr) {
        s_map = new QMap<int, QString>;
    }
    return *s_map;
}

QList<int> PageItem::PageItem_private::registedItemTypes()
{
    auto map = registedClassNameMap();
    QList<int> result;
    for (auto it = map.begin(); it != map.end(); ++it) {
        result.append(it.key());
    }
    return result;
}

void PageItem::PageItem_private::removeChildForChildList(PageItem *child)
{
    childrens.removeOne(child);
}

//void PageItem::PageItem_private::addChildToChildList(PageItem *child)
//{
//    if (!childrens.contains(child))
//        childrens.append(child);
//}

void PageItem::PageItem_private::operatingBegin_helper(PageItemEvent *event)
{
    m_operatingType = event->type();
    event->setItem(q);
    switch (event->type()) {
    case PageItemEvent::EMove: {
        PageItemMoveEvent *movEvt = static_cast<PageItemMoveEvent *>(event);
        q->doMoveBegin(movEvt);
        break;
    }
    case PageItemEvent::EScal: {
        PageItemScalEvent *sclEvt = static_cast<PageItemScalEvent *>(event);
        q->doScalBegin(sclEvt);
        break;
    }
    case PageItemEvent::ERot: {
        PageItemRotEvent *rotEvt = static_cast<PageItemRotEvent *>(event);
        q->changeTransCenterTo(event->centerPos());
        q->doRotBegin(rotEvt);
        break;
    }
    default:
        break;
    }
}

void PageItem::PageItem_private::operating_helper(PageItemEvent *event)
{
    event->setItem(q);
    switch (event->type()) {
    case PageItemEvent::EMove: {
        PageItemMoveEvent *movEvt = static_cast<PageItemMoveEvent *>(event);
        q->doMoving(movEvt);
        break;
    }
    case PageItemEvent::EScal: {
        PageItemScalEvent *sclEvt = static_cast<PageItemScalEvent *>(event);
        q->doScaling(sclEvt);
        break;
    }
    case PageItemEvent::ERot: {
        PageItemRotEvent *rotEvt = static_cast<PageItemRotEvent *>(event);
        q->doRoting(rotEvt);
        break;
    }
    default:
        break;
    }
}

bool PageItem::PageItem_private::testOpetating_helper(PageItemEvent *event)
{
    event->setItem(q);
    bool accept = false;
    switch (event->type()) {
    case PageItemEvent::EMove: {
        PageItemMoveEvent *movEvt = static_cast<PageItemMoveEvent *>(event);
        accept = q->testMoving(movEvt);
        break;
    }
    case PageItemEvent::EScal: {
        PageItemScalEvent *sclEvt = static_cast<PageItemScalEvent *>(event);
        accept = q->testScaling(sclEvt);
        break;
    }
    case PageItemEvent::ERot: {
        PageItemRotEvent *rotEvt = static_cast<PageItemRotEvent *>(event);
        accept = q->testRoting(rotEvt);
        break;
    }
    default:
        break;
    }
    return accept;
}

void PageItem::PageItem_private::operatingEnd_helper(PageItemEvent *event)
{
    m_operatingType = -1;
    event->setItem(q);
    switch (event->type()) {
    case PageItemEvent::EMove: {
        PageItemMoveEvent *movEvt = static_cast<PageItemMoveEvent *>(event);
        q->doMoveEnd(movEvt);
        q->updateHandle();
        break;
    }
    case PageItemEvent::EScal: {
        PageItemScalEvent *sclEvt = static_cast<PageItemScalEvent *>(event);
        q->doScalEnd(sclEvt);
        q->updateShape();
        break;
    }
    case PageItemEvent::ERot: {
        PageItemRotEvent *rotEvt = static_cast<PageItemRotEvent *>(event);
        q->doRotEnd(rotEvt);
        break;
    }
    case PageItemEvent::EBlur: {
        break;
    }
    default:
        break;
    }
}

void PageItem::PageItem_private::setLayer_helper(LayerItem *layer)
{
    foreach (auto child, childrens) {
        child->d_PageItem()->setLayer_helper(layer);
    }

    _layer = layer;
}

void PageItem::PageItem_private::notifyItemAndChildrenLayerChanged(LayerItem *layer)
{
    foreach (auto child, childrens) {
        child->d_PageItem()->notifyItemAndChildrenLayerChanged(layer);
    }
    notifyItemChange(PageItemLayerChanged, QVariant::fromValue<LayerItem *>(layer));
}

void PageItem::PageItem_private::setPageScene_helper(PageScene *scene)
{
    foreach (auto child, childrens) {
        child->d_PageItem()->setPageScene_helper(scene);
    }
    if (scene != nullptr) {
        scene->addItem(q);
    } else {
        if (q->scene() != nullptr) {
            q->scene()->removeItem(q);
        }
    }
}

void PageItem::PageItem_private::notifyItemAndChildrenPageSceneChanged(PageScene *scene)
{
    foreach (auto child, childrens) {
        child->d_PageItem()->notifyItemAndChildrenPageSceneChanged(scene);
    }
    notifyItemChange(PageItemSceneChanged, QVariant::fromValue<PageScene *>(scene));
}

void PageItem::PageItem_private::setSelected_helper(bool b)
{
    foreach (auto child, childrens) {
        child->d_PageItem()->setSelected_helper(b);
    }
    isSelected = b;
}

QVariant PageItem::PageItem_private::notifyItemChange(int doChange, const QVariant &value)
{
    if (blockNotify)
        return value;

    QVariant var = value;

    bool finished = false;
    foreach (auto flt, filters) {
        bool accept = flt->pageItemChangeFilter(q, doChange, var);
        if (accept) {
            finished = true;
            break;
        }
    }

    if (!finished) {
        if (ItemPositionHasChanged == doChange || ItemScenePositionHasChanged == doChange ||
                PageItemRectChanged == doChange || ItemSelectedHasChanged == doChange ||
                ItemTransformChange == doChange || PageItemMulSelectionChanged == doChange) {
            foreach (auto nd, m_handles) {
                nd->parentItemChanged(doChange, value);
            }
        }
        return q->pageItemChange(doChange, var);
    }
    return var;
}

void PageItem::PageItem_private::initStyleOption(QStyleOptionGraphicsItem *option,
                                                 const QTransform &worldTransform,
                                                 const QRegion &exposedRegion,
                                                 bool allItems) const
{
    Q_ASSERT(option);
//    Q_Q(const QGraphicsItem);

    // Initialize standard QStyleOption values.
    const QRectF brect = q->boundingRect();
    option->state = QStyle::State_None;
    option->rect = brect.toRect();
    option->exposedRect = brect;

    auto scene = q->scene();

    // Style animations require a QObject-based animation target.
    // If a plain QGraphicsItem is used to draw animated controls,
    // QStyle is let to send animation updates to the whole scene.
    option->styleObject = q->toGraphicsObject();
    if (!option->styleObject)
        option->styleObject = scene;

    if (isSelected)
        option->state |= QStyle::State_Selected;
    if (q->isEnabled())
        option->state |= QStyle::State_Enabled;
    if (q->hasFocus())
        option->state |= QStyle::State_HasFocus;
    if (scene) {
//        if (scene->d_func()->hoverItems.contains(q_ptr))
//            option->state |= QStyle::State_MouseOver;
        if (q == scene->mouseGrabberItem())
            option->state |= QStyle::State_Sunken;
    }

    if (!(q->flags() & QGraphicsItem::ItemUsesExtendedStyleOption))
        return;

    if (!allItems) {
        // Determine the item's exposed area
        option->exposedRect = QRectF();
        const QTransform reverseMap = worldTransform.inverted();
        for (const QRect &exposedRect : exposedRegion) {
            option->exposedRect |= reverseMap.mapRect(QRectF(exposedRect));
            if (option->exposedRect.contains(brect))
                break;
        }
        option->exposedRect &= brect;
    }
}

int PageItem::PageItem_private::maxZ() const
{
    int max = 0;
    foreach (auto p, childrens) {
        if (p->pageZValue() > max) {
            max = p->pageZValue();
        }
    }
    return max;
}

//void PageItem::PageItem_private::setZ_Helper(int zz)
//{
//    if(blockZAgin)
//        return;

//    z = zz;
//}

PageItem *zEndingItem(const QList<PageItem *> &pBzItems, int wantZitemTp)
{
    if (pBzItems.isEmpty())
        return nullptr;
    PageItem *result = nullptr;

    QList<PageItem *> allBzItems;

    for (auto p : pBzItems) {
        if (p->isPageGroup()) {
            allBzItems.append(static_cast<GroupItem *>(p)->getNotGroupItems(true));
        } else if (p->isBzItem()) {
            allBzItems.append(p);
            //qDebug() << "zzzzzz = " << p->zValue();
        }
    }

    if (wantZitemTp == -1) {
        auto lists = returnSortZItems(allBzItems, EAesSort);
        if (!lists.isEmpty())
            result = lists.first();
    } else if (wantZitemTp == -2) {
        auto lists = returnSortZItems(allBzItems, EDesSort);
        if (!lists.isEmpty())
            result = lists.first();
    }
    return result;
}

//降序排列用
bool zValueSortDES(QGraphicsItem *info1, QGraphicsItem *info2)
{
    //qWarning() << "info1 TYPE ===== " << info1->type() << info2->type();
    if (PageScene::isPageItem(info1) && PageScene::isPageItem(info2))
        return static_cast<PageItem *>(info1)->pageZValue() > static_cast<PageItem *>(info2)->pageZValue();
    return info1->zValue() > info2->zValue();
}
//升序排列用
bool zValueSortASC(QGraphicsItem *info1, QGraphicsItem *info2)
{
    if (PageScene::isPageItem(info1) && PageScene::isPageItem(info2))
        return static_cast<PageItem *>(info1)->pageZValue() < static_cast<PageItem *>(info2)->pageZValue();
    return info1->zValue() < info2->zValue();
}

QSet<PageItem *> PageItemNotifyBlocker::inBlockerItems = QSet<PageItem *>();
PageItemNotifyBlocker::PageItemNotifyBlocker(PageItem *item, bool notifyChangs)

{
    valid = (!inBlockerItems.contains(item));
    if (valid) {
        it = (item);
        blocked = (it->d_PageItem()->blockNotify);
        doNotifyChanges = (notifyChangs);
        parent = (it->parentPageItem());
        group = (it->pageGroup());
        layer = (it->layer());
        scene = (it->pageScene());
        it->d_PageItem()->blockNotify = true;
        inBlockerItems.insert(it);
    }
}

PageItemNotifyBlocker::~PageItemNotifyBlocker()
{
    if (valid) {
        it->d_PageItem()->blockNotify = blocked;

        if (doNotifyChanges) {
            if (it->parentPageItem() != parent)
                it->d_PageItem()->notifyItemChange(PageItem::PageItemParentChanged, QVariant::fromValue<PageItem *>(it->parentPageItem()));

            if (it->pageGroup() != group)
                it->d_PageItem()->notifyItemChange(PageItem::PageItemGroupChanged, QVariant::fromValue<GroupItem *>(it->pageGroup()));

            if (it->layer() != layer)
                it->d_PageItem()->notifyItemChange(PageItem::PageItemLayerChanged, QVariant::fromValue<LayerItem *>(it->layer()));

            if (it->pageScene() != scene)
                it->d_PageItem()->notifyItemChange(PageItem::PageItemSceneChanged, QVariant::fromValue<PageScene *>(it->pageScene()));
        }
        inBlockerItems.remove(it);
    }
}
