// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pageitem.h"
#include "textitem.h"
#include "cgraphicsproxywidget.h"
#include "pagescene.h"
#include "ctextedit.h"
#include "pageview.h"
#include "itemgroup.h"
#include "global.h"
#include "cgraphicsitemevent.h"
//#include "extendattribution.h"
#include "attributemanager.h"
#include <QDebug>
#include <QGraphicsScene>
#include <QPainterPath>
#include <QVariant>
#include <QtMath>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QApplication>

#include "attributewidget.h"

#include "penitem_compatible.h"
#include "imageitem_compatible.h"
#include "rasteritem.h"
#include "pageitem_p.h"
#include "layeritem.h"

REGISTITEMCLASS(PenItem_Compatible, PenType)
REGISTITEMCLASS(ImageItem_Compatible, PictureType)

const int inccW = 10;

PageItem *PageItem::creatItemInstance(int itemType, const Unit &data)
{
    PageItem *item = nullptr;
    auto registerItems = PageItem_private::registedClassNameMap();
    auto itf = registerItems.find(itemType);
    if (itf != registerItems.end()) {
        item =  ClassObjectFactory::creatObject<PageItem>(itf.value());
    }

    if (item != nullptr && data.isValid())
        item->loadItemUnit(data);

//    if ((PictureType == itemType || itemType == PenType) && item != nullptr) {
//        QImage img = static_cast<VectorItem *>(item)->rasterSelf();
//        auto jLay = new RasterItem(img);
//        auto gemo = new RasterGeomeCmd(item->pos(), item->drawRotation(),
//                                       item->zValue(), item->boundingRect(), item->transform());
//        jLay->appendComand(gemo, true, false);

//        jLay->setBlocked(false);
//        if (itemType == PictureType) {
//            jLay->setBlocked(true);
//            jLay->setRasterType(RasterItem::EImageType);
//        }

//        delete item;
//        item = jLay;
//    }

    return item;
}

QList<int> PageItem::registedItemTypes()
{
    return PageItem_private::registedItemTypes();
}

//PageItem *PageItem::zItem(const QList<PageItem *> &pBzItems, int wantZitemTp)
//{
//    if (pBzItems.isEmpty())
//        return nullptr;
//    PageItem *result = nullptr;

//    QList<PageItem *> allBzItems;

//    for (auto p : pBzItems) {
//        if (p->isPageGroup()) {
//            allBzItems.append(static_cast<ItemGroup *>(p)->getBzItems(true));
//        } else if (p->isBzItem()) {
//            allBzItems.append(p);
//            //qDebug() << "zzzzzz = " << p->zValue();
//        }
//    }

//    if (wantZitemTp == -1) {
//        auto lists = PageScene::returnSortZItems(allBzItems, PageScene::EAesSort);
//        if (!lists.isEmpty())
//            result = lists.first();
//    } else if (wantZitemTp == -2) {
//        auto lists = PageScene::returnSortZItems(allBzItems, PageScene::EDesSort);
//        if (!lists.isEmpty())
//            result = lists.first();
//    }
//    return result;
//}


PageItem::PageItem(PageItem *parent)
    : QGraphicsItem(nullptr), PageItem_d(new PageItem_private(this))
{
    this->setFlag(QGraphicsItem::ItemIsMovable, true);
    this->setFlag(QGraphicsItem::ItemIsSelectable, true);
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    this->setFlag(QGraphicsItem::ItemIsFocusable, false);
    this->setAcceptHoverEvents(true);

    setParentPageItem(parent);
}

PageItem::~PageItem()
{
    //1.委托给这个item进行处理的items(过滤委托者filteDelegators)在这个item被析构时需要从他们的过滤器列表中删除以防止出现野指针访问
    //notes:防止过滤器是自身时(p==this)导致removeChangeFilter时破坏了filteDelegators的迭代器从而引起崩溃
    auto lists = PageItem_d->filteDelegators;
    foreach (auto p, lists) {
        p->removeChangeFilter(this);
    }

    //2.设置为不被选择, 使自身从场景选择列表中删除
    setItemSelected(false);

    //3.从父列表中删除掉
    setParentPageItem(nullptr);

    //4.从所属组合中删除掉
    setPageGroup(nullptr);

    //5.从所属图层中删除掉
    setLayer(nullptr);
}

void PageItem::registerItem(const QString &classname, int classType)
{
    qWarning() << "classname = " << classname << "classType = " << classType;
    PageItem_private::registedClassNameMap().insert(classType, classname);
}

void PageItem::setPageScene(PageScene *scene)
{
    if (scene != nullptr)
        scene->addPageItem(this);
    else {
        pageScene()->removePageItem(this);
    }
}

void PageItem::setParentPageItem(PageItem *parent)
{
    if (parent != parentPageItem()) {
        setParentItem(parent);
        //not same with qt(setParentItem to null will make item to be one top level item)
        //we should remove item from scene if parent is null (only layerItem can be top level item)
        if (parentPageItem() == nullptr) {
            if (pageScene() != nullptr) {
                pageScene()->removePageItem(this);
            }
        }
    }
}

PageItem *PageItem::parentPageItem() const
{
    if (this->parentItem() == nullptr)
        return nullptr;
    return dynamic_cast<PageItem *>(this->parentItem());
}

PageItem *PageItem::rootParentPageItem() const
{
    PageItem *parent = const_cast<PageItem *>(this);
    while (PageItem *grandPa = parent->parentPageItem())
        parent = grandPa;
    return parent;
}

bool PageItem::isRootPageItem() const
{
    return parentPageItem() == nullptr;
}

void PageItem::removeChild(PageItem *child)
{
    child->setParentPageItem(this->parentPageItem());
}

void PageItem::addChild(PageItem *child)
{
    child->setParentPageItem(this);
}

QList<PageItem *> PageItem::childPageItems() const
{
    //return d_PageItem()->childrens;

    return switchListClassTo<QGraphicsItem, PageItem>(childItems());

    //auto children = childItems();
    //foreach
}

SAttrisList PageItem::attributions()
{
    auto children = childPageItems();
    if (children.isEmpty()) {
        return SAttrisList();
    }

    SAttrisList result = children.first()->attributions();
    foreach (auto p, children) {
        result = result.insected(p->attributions());
    }
    return result;
}

void PageItem::setAttributionVar(int attri, const QVariant &var, int phase)
{
    auto items = this->childPageItems();
    foreach (auto i, items) {
        if (i->attributions().haveAttribution(attri))
            i->setAttributionVar(attri, var, phase);
    }
}

PageView *PageItem::pageView() const
{
    PageView *parentView = nullptr;
    if (scene() != nullptr) {
        if (!scene()->views().isEmpty()) {
            parentView = dynamic_cast<PageView *>(scene()->views().first());
        }
    }
    return parentView;
}

PageScene *PageItem::pageScene() const
{
    if (scene() == nullptr)
        return nullptr;
    return qobject_cast<PageScene *>(scene());
}

Page *PageItem::page() const
{
    if (pageView() != nullptr)
        return pageView()->page();

    return nullptr;
}

bool PageItem::isIdle() const
{
    if (scene() == nullptr)
        return true;

    return (parentItem() != nullptr && parentItem()->type() == TrashBinItemType);
}

void PageItem::deleteSafe()
{
    if (scene() != nullptr) {
        scene()->removeItem(this);
    }
    qWarning() << "PageItem::deleteSafe() ===== " << this;
    delete this;
}
QPainterPath PageItem::itemShape() const
{
    QPainterPath path;
    path.addRect(itemRect());
    return path;
}

QRectF PageItem::boundingRect() const
{
    //因为使用了抗锯齿等功能，导致会多出一部分，所以添加两个像素保证刷新正确
//    if (itemRect().isValid())
//        return itemRect().adjusted(-2, -2, 2, 2);
//    return QRectF();

    return itemRect();
}

QPainterPath PageItem::shape() const
{
    return itemShape();
}

void PageItem::preparePageItemGeometryChange()
{
    prepareGeometryChange();
    foreach (auto node, handleNodes()) {
        node->update();
    }
    if (scene() != nullptr) {
        scene()->update(mapRectToScene(childrenBoundingRect() | boundingRect()));
    }
}

QRectF PageItem::selectedRectangle() const
{
    return orgRect();
}

bool PageItem::isMutiSelected() const
{
    if (GroupItem *group = this->pageGroup())
        return (group->isItemSelected());
    return d_PageItem()->isMulSelected;
}

bool PageItem::isItemSelected() const
{
    if (GroupItem *group = this->pageGroup())
        return group->isItemSelected();

    return d_PageItem()->isSelected;
}

void PageItem::setItemSelected(bool b)
{
    if (pageScene() != nullptr) {
        pageScene()->setPageItemSelected(this, b);
    }
}

bool PageItem::isSingleSelected() const
{
    return isItemSelected() && (!isMutiSelected());
}

QPainterPath PageItem::highLightPath() const
{
    return itemShape();
}

void PageItem::loadHeadData(const UnitHead &head)
{
    d_PageItem()->_roteAgnel = head.rotate;
    this->setPos(head.pos);
    this->setPageZValue(head.zValue);
    this->setTransform(head.trans);
}

UnitHead PageItem::getHeadData() const
{
    UnitHead head;
    head.dataType = type();
    head.zValue = this->zValue();
    head.rect = this->orgRect();
    head.pos = this->pos();
    head.rotate = this->drawRotation();
    head.trans = this->transform();

    return head;
}

int PageItem::type() const
{
    return Type;
}

bool PageItem::isBzItem() const
{
    return (this->type() > NoType && this->type() < GroupItemType);
}

void PageItem::doFilp(PageItem::EFilpDirect dir)
{
    if (dir == EFilpHor) {
        d_PageItem()->_flipHorizontal = !d_PageItem()->_flipHorizontal;
    } else if (dir == EFilpVer) {
        d_PageItem()->_flipVertical = !d_PageItem()->_flipVertical;
    }

    QPointF center = itemRect().center();
    QTransform trans(dir == EFilpHor ? -1 : 1, 0, 0,
                     0, dir == EFilpVer  ? -1 : 1, 0,
                     0, 0, 1);

    trans = (QTransform::fromTranslate(-center.x(), -center.y()) * trans * QTransform::fromTranslate(center.x(), center.y()));

    setTransform(trans, true);
    update();
}
void PageItem::rot90Angle(bool leftOrRight)
{
    QPointF center = this->itemRect().center();
    qreal angle = 0.0;

    // 左旋转减小 右旋转增大
    if (leftOrRight == true) {
        angle = - 90.0;
    } else {
        angle = 90.0;
    }

    // 矩阵变换
    QTransform trans;
    trans.translate(center.x(), center.y());
    trans.rotate(angle);
    trans.translate(-center.x(), -center.y());

    // 设置当前旋转角度
    setDrawRotatin(angle + drawRotation());
    setTransform(trans, true);
}

void PageItem::move(QPointF beginPoint, QPointF movePoint)
{
    auto offset = movePoint - beginPoint;
    foreach (PageItem *item, d_PageItem()->childrens) {
        item->moveBy(offset.x(), offset.y());
    }
    this->moveBy(offset.x(), offset.y());
}

void PageItem::setFilpBaseOrg(ImageItem_Compatible::EFilpDirect dir, bool b)
{
    if (dir == EFilpHor) {
        if (d_PageItem()->_flipHorizontal != b) {
            doFilp(dir);
        }
    } else if (dir == EFilpVer) {
        if (d_PageItem()->_flipVertical != b) {
            doFilp(dir);
        }
    }
}

bool PageItem::isFilped(ImageItem_Compatible::EFilpDirect dir) const
{
    return (dir == EFilpHor ? d_PageItem()->_flipHorizontal : d_PageItem()->_flipVertical);
}

void PageItem::changeTransCenterTo(const QPointF &newCenter)
{
    setTransformOriginPoint(newCenter);
}

void PageItem::updateHandle()
{
    foreach (auto nd, d_PageItem()->m_handles) {
        nd->parentItemChanged(ItemPositionHasChanged, orgRect());
    }
}

bool PageItem::isPosPenetrable(const QPointF &posLocal)
{
    return false;
}

bool PageItem::isRectPenetrable(const QRectF &rectLocal)
{
    return false;
}

LayerItem *PageItem::layer() const
{
    //return d_PageItem()->_layer;
    if (parentPageItem() == nullptr)
        return nullptr;
    if (parentPageItem()->isLayer()) {
        return static_cast<LayerItem *>(parentPageItem());
    }
    return nullptr;
}

void PageItem::setLayer(LayerItem *layer)
{
    if (this->layer() == layer)
        return;

    this->setParentPageItem(layer);
}

bool PageItem::isLayer() const
{
    return type() == LayerItemType;
}

LayerItem *PageItem::topLayer() const
{
    PageItem *layer = const_cast<PageItem *>(this);
    while (LayerItem *ly = layer->layer())
        layer = ly;

    if (layer == this) {
        return nullptr;
    }

    return static_cast<LayerItem *>(layer);
}

bool PageItem::isTopLayer() const
{
    return (layer() == nullptr && isLayer() && parentPageItem() == nullptr);
}

bool PageItem::isPageGroup() const
{
    return this->type() == GroupItemType;
}

GroupItem *PageItem::pageGroup() const
{
    if (parentPageItem() == nullptr)
        return nullptr;

    if (parentPageItem()->isPageGroup())
        return static_cast<GroupItem *>(parentPageItem());

    return nullptr;
}

bool PageItem::isTopPageGroup() const
{
    return (isPageGroup() && parentPageItem() == nullptr && pageGroup() == nullptr);
}

GroupItem *PageItem::topPageGroup() const
{
    PageItem *parent = const_cast<PageItem *>(this);
    while (GroupItem *grandPa = parent->pageGroup())
        parent = grandPa;

    if (parent == this) {
        return nullptr;
    }

    return static_cast<GroupItem *>(parent);
}

PageItem *PageItem::pageProxyItem(bool topleve)
{
    GroupItem *pTg = topleve ? topPageGroup() : pageGroup();
    if (pTg != nullptr)
        return pTg;

    return this;
}

void PageItem::setPageGroup(GroupItem *pGroup)
{
    //防止添加自己引起循环
    if (pGroup == this)
        return;

    //判断是否相等可终结循环
    if (pGroup == pageGroup())
        return;

    //先从原来的组合中删除
    if (pageGroup() != nullptr) {
        pageGroup()->removeFromGroup(this);
    }
    //再添加到新的组合中
    if (pGroup != nullptr) {
        pGroup->addToGroup(this);
    }
}

qreal PageItem::pageZValue() const
{
    return zValue();
}

void PageItem::setPageZValue(qreal z)
{
    setZValue(z);
}

bool PageItem::contains(const QPointF &point) const
{
    if (pageGroup() != nullptr)
        return false;
    return itemShape().contains(point);
}

int PageItem::operatingType() const
{
    return d_PageItem()->m_operatingType;
}

bool PageItem::isOperating() const
{
    return d_PageItem()->m_operatingType != -1;
}

void PageItem::operatingBegin(PageItemEvent *event)
{
    foreach (PageItem *pItem, childPageItems()) {
        QTransform thisToItem = this->itemTransform(pItem);
        PageItemEvent *childEvent = event->creatTransDuplicate(thisToItem, pItem->orgRect().size());
        childEvent->setItem(pItem);
        pItem->/*d_PageItem()->operatingBegin_helper*/operatingBegin(childEvent);
        delete childEvent;
    }
    d_PageItem()->operatingBegin_helper(event);
}

void PageItem::operating(PageItemEvent *event)
{
    bool accept = testOpetating(event);
    if (accept) {
        if (event->type() == PageItemEvent::EScal) {
            foreach (PageItem *pItem, childPageItems()) {
                //得到将自身坐标系映射到其他图元pItem坐标系的矩阵
                QTransform thisToItem = this->itemTransform(pItem);
                PageItemEvent *childEvent = event->creatTransDuplicate(thisToItem, pItem->orgRect().size());
                childEvent->setItem(pItem);
                //将自身要做的转换矩阵映射到pItem上
                QTransform childDoTrans = thisToItem.inverted() * event->trans() * thisToItem;
                childEvent->setTrans(childDoTrans);
                pItem->operating(childEvent);
                delete childEvent;
            }
        }
        d_PageItem()->operating_helper(event);
    }
}

bool PageItem::testOpetating(PageItemEvent *event)
{
    //1.先判断自身是否能接受这个操作事件
    bool accept = d_PageItem()->testOpetating_helper(event);
    if (accept) {
        //2.再判断孩子们是否能接受这个操作事件
        if (event->type() == PageItemEvent::EScal) {
            //auto doItems = items(true);
            foreach (PageItem *pItem, childPageItems()) {
                QTransform thisToItem = this->itemTransform(pItem);
                PageItemEvent *childEvent = event->creatTransDuplicate(thisToItem, pItem->orgRect().size());
                childEvent->setItem(pItem);
                childEvent->setDriverEvent(event);
                QTransform childDoTrans = thisToItem.inverted() * event->trans() * thisToItem;
                childEvent->setTrans(childDoTrans);
                bool isItemAccept = pItem->/*d_PageItem()->testOpetating_helper*/testOpetating(childEvent);
                if (!isItemAccept) {
                    accept = false;
                    delete childEvent;
                    break;
                }
                delete childEvent;
            }
        }
    }
    return accept;
}

void PageItem::operatingEnd(PageItemEvent *event)
{
    foreach (PageItem *pItem, childPageItems()) {
        QTransform thisToItem = this->itemTransform(pItem);
        PageItemEvent *childEvent = event->creatTransDuplicate(thisToItem, pItem->orgRect().size());
        childEvent->setItem(pItem);
        childEvent->setTrans(thisToItem.inverted() * event->trans() * thisToItem);
        pItem->operatingEnd(childEvent);
        delete childEvent;
    }
    d_PageItem()->operatingEnd_helper(event);
}

bool PageItem::installChangeFilter(PageItem *filter)
{
    //如果filter的事件已经委托给我(this)处理,那么我的事件不能再委托给filter处理，不然会引起循环
    if (d_PageItem()->filteDelegators.contains(filter)) {
        return false;
    }

    if (!d_PageItem()->filters.contains(filter)) {
        d_PageItem()->filters.append(filter);
        filter->d_PageItem()->filteDelegators.insert(this);
        return true;
    }
    return false;
}

void PageItem::removeChangeFilter(PageItem *filter)
{
    d_PageItem()->filters.removeOne(filter);
    filter->d_PageItem()->filteDelegators.remove(this);
}

void PageItem::render(QPainter *painter)
{
    QStyleOptionGraphicsItem option;
    d_PageItem()->initStyleOption(&option, painter->worldTransform(), QRegion(), true);
    paint(painter, &option, nullptr);
}

QImage PageItem::renderToImage()
{
    QImage img(itemRect().size().toSize(), QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter painter(&img);
    painter.translate(-itemRect().topLeft());
    render(&painter);
    painter.end();
    return img;
}

//ExtendAttribution *PageItem::getExtendObject()
//{
//    return ExtendAttribution::getInstance();
//}

void PageItem::doMoveBegin(PageItemMoveEvent *event)
{
    Q_UNUSED(event)
}

void PageItem::doMoving(PageItemMoveEvent *event)
{
    QPointF move = event->_scenePos - event->_oldScenePos;
    this->moveBy(move.x(), move.y());
}

bool PageItem::testMoving(PageItemMoveEvent *event)
{
    Q_UNUSED(event)
    return true;
}

void PageItem::doMoveEnd(PageItemMoveEvent *event)
{
    Q_UNUSED(event)
}

void PageItem::doScalBegin(PageItemScalEvent *event)
{
    Q_UNUSED(event);
}

void PageItem::doScaling(PageItemScalEvent *event)
{
    Q_UNUSED(event)
}

bool PageItem::testScaling(PageItemScalEvent *event)
{
    Q_UNUSED(event)
    return true;
}

void PageItem::doScalEnd(PageItemScalEvent *event)
{
    Q_UNUSED(event)
}

void PageItem::doRotBegin(PageItemRotEvent *event)
{
    Q_UNUSED(event);
}

void PageItem::doRoting(PageItemRotEvent *event)
{
    QPointF center = event->centerPos();
    qreal angle = 0.0;

    // 判断是否为直接使用角度值的计算
    if (event->isUsingAngle()) {
        angle = event->usingAngle();
    } else {
        QLineF l1 = QLineF(center, event->oldPos());
        QLineF l2 = QLineF(center, event->pos());
        // Qt使用负数表示顺时针旋转，计算时需要注意方向
        angle = l1.angle() - l2.angle();
    }

    if (qFuzzyIsNull(angle)) {
        return;
    }

    QTransform trans;
    trans.translate(center.x(), center.y());
    trans.rotate(angle);
    trans.translate(-center.x(), -center.y());
    d_PageItem()->_roteAgnel += angle;
    int n = int(d_PageItem()->_roteAgnel) / 360;
    d_PageItem()->_roteAgnel = d_PageItem()->_roteAgnel - n * 360;
    setTransform(trans, true);

    // 更新子节点的属性，用于群组图元
    for (PageItem *child : childPageItems()) {
        child->setDrawRotatin(child->drawRotation() + angle);
    }
}

bool PageItem::testRoting(PageItemRotEvent *event)
{
    Q_UNUSED(event)
    return true;
}

void PageItem::doRotEnd(PageItemRotEvent *event)
{
    Q_UNUSED(event)
}

void PageItem::loadUnit(const Unit &ut)
{
    loadHeadData(ut.head);
}

void PageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    beginCheckIns(painter);

    painter->save();

    paintSelf(painter, option);

    painter->restore();

    paintChidren(painter, returnSortZItems(childPageItems(), EAesSort), widget);

    paintMutBoundingLine(painter, option);

    endCheckIns(painter);
}

void PageItem::paintChidren(QPainter *painter, const QList<PageItem *> &childList, QWidget *widget)
{
    if (pageView() != nullptr && (!(pageView()->optimizationFlags()& QGraphicsView::IndirectPainting))) {
        return;
    }
    painter->save();
    QStyleOptionGraphicsItem option;
    foreach (auto p, childList) {
        painter->save();
        painter->setTransform(p->itemTransform(this), true);
        d_PageItem()->initStyleOption(&option, painter->worldTransform(), QRegion(), true);
        p->paint(painter, &option, widget);
        painter->restore();
    }
    foreach (auto p, this->childItems()) {
        painter->save();
        painter->setTransform(p->itemTransform(this), true);
        d_PageItem()->initStyleOption(&option, painter->worldTransform(), QRegion(), true);
        p->paint(painter, &option, widget);
        painter->restore();
    }
    painter->restore();
}

void PageItem::paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
}

void PageItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->setAccepted(false);
}

void PageItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    event->setAccepted(false);
}

void PageItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    event->setAccepted(false);
}

void PageItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    event->setAccepted(false);
}

Unit PageItem::getUnit(int use) const
{
    Unit ut(use);
    ut.head = getHeadData();
    return ut;
}

void PageItem::loadItemUnit(const Unit &ut)
{
    if (ut.usage & UnitUsage_All_NewChild) {
        for (int i = 0; i < ut.chidren.count(); ++i) {
            auto childUt = ut.chidren.at(i);
            this->addChild(creatItemInstance(childUt.head.dataType, childUt));
        }
    } else if (ut.usage & UnitUsage_All_DontNewChild) {
        if (ut.chidren.count() == childPageItems().count()) {
            for (int i = 0; i < childPageItems().count(); ++i) {
                auto child = childPageItems().at(i);
                child->loadItemUnit(ut.chidren.at(i));
            }
        } else {
            qWarning() << "load Unit failed because ut.children count not same with item children when unit usage=UnitUsage_All_DontNewChild";
            qWarning() << "ut.children count = " << ut.chidren.count() << "items children count = " << childPageItems().count();
        }
    }
    loadUnit(ut);
}

Unit PageItem::getItemUnit(int use) const
{
    Unit ut = getUnit(use);
    ut.usage = use;
    if ((ut.usage & UnitUsage_All_NewChild) || (ut.usage & UnitUsage_All_DontNewChild)) {
        for (int i = 0; i < childPageItems().count(); ++i) {
            auto child = childPageItems().at(i);
            ut.chidren.append(child->getItemUnit(use));
        }
    }
    return ut;
}

void PageItem::loadData(const PageVariant &ut)
{
    if (ut.canConvert<Unit>()) {
        loadItemUnit(ut.value<Unit>());
    }
}

PageVariant PageItem::getData(int use) const
{
    Unit ut = getItemUnit(use);
    return QVariant::fromValue<Unit>(ut);
}

void PageItem::updateShape()
{
    updateHandle();
}

void PageItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    Q_UNUSED(event);
    qWarning() << "event ====== " << event << this->type() << scene()->selectedItems();


    QGraphicsItem::contextMenuEvent(event);
}

void PageItem::notifyChanged(int changeType, const QVariant &value)
{
    d_PageItem()->notifyItemChange(changeType, value);
}

QVariant PageItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange) {
        foreach (auto nd, handleNodes()) {
            nd->update();
        }
        if (scene() != nullptr) {
            scene()->update(mapRectToScene(childrenBoundingRect() | boundingRect()));
        }
        this->update();
    }
    if (change == ItemZValueHasChanged) {
        if (page() != nullptr)
            emit page()->borad()->attributionManager()->helper()->internalAttibutionUpdate(EOrderProperty, 0, 0);
    }
    auto var = d_PageItem()->notifyItemChange(change, value);

    return var;
}

QVariant PageItem::pageItemChange(int changeType, const QVariant &value)
{
    return value;
}

bool PageItem::pageItemChangeFilter(PageItem *item, int changeType, QVariant &value)
{
    return false;
}

void PageItem::paintItemHelper(PageItem *item, QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    item->paint(painter, option, widget);
}

void PageItem::beginCheckIns(QPainter *painter)
{
    if (scene() == nullptr || !itemRect().isValid())
        return;

    painter->save();
    QRectF sceneRct = pageScene()->currentTopLayer()->sceneBoundingRect();//scene()->sceneRect();
    QRectF itemRct  = mapToScene(itemRect()).boundingRect();
    bool hasIntersects = sceneRct.intersects(itemRct);
    if (!hasIntersects) {
        painter->setOpacity(0.2);//透明度设置
    }
    painter->setClipping(hasIntersects);
    if (hasIntersects) {
        painter->setClipRect(mapRectFromScene(sceneRct), Qt::IntersectClip);
    }
}

void PageItem::endCheckIns(QPainter *painter)
{
    if (scene() == nullptr || !itemRect().isValid())
        return;

    painter->restore();
}

void PageItem::paintMutBoundingLine(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    if (!isPaintSelectingBorder())
        return;

    if (isMutiSelected() && pageGroup() == nullptr) {
        painter->setClipping(false);
        QPen pen;

        painter->setRenderHint(QPainter::Antialiasing, true);

        pen.setWidthF(1 / option->levelOfDetailFromTransform(painter->worldTransform()));

        //选中标识颜色需要加深；目前的色值是#E0E0E0；UI提供色值：#BBBBBB
        pen.setColor(QColor(187, 187, 187));

        painter->setPen(pen);
        painter->setBrush(QBrush(Qt::NoBrush));
        //painter->drawPath(orgShape());
        painter->drawRect(orgRect());
        painter->setClipping(true);
    }
}

void PageItem::setDrawRotatin(qreal angle)
{
    d_PageItem()->_roteAgnel = angle;

    // 进行角度取余数
    int n = int(d_PageItem()->_roteAgnel) / 360;
    d_PageItem()->_roteAgnel = d_PageItem()->_roteAgnel - n * 360;
}

qreal PageItem::drawRotation() const
{
    return d_PageItem()->_roteAgnel;
}

Handles PageItem::handleNodes() const
{
    return d_PageItem()->m_handles;
}

void PageItem::addHandleNode(HandleNode *node)
{
    d_PageItem()->m_handles.append(node);
}

void PageItem::removeHandleNode(HandleNode *node)
{
    d_PageItem()->m_handles.removeOne(node);
}

int PageItem::nodeCount() const
{
    return d_PageItem()->m_handles.count();
}

HandleNode *PageItem::handleNode(int direction) const
{
    foreach (HandleNode *pNode, d_PageItem()->m_handles) {
        if (pNode->nodeType() == direction) {
            return pNode;
        }
    }
    return nullptr;
}

void PageItem::setSingleSelectionBorderVisible(bool b)
{
    d_PageItem()->isSingleSelectionBorderVisible = b;
}

bool PageItem::isSingleSelectionBorderVisible() const
{
    return d_PageItem()->isSingleSelectionBorderVisible;
}

QTransform PageItem::getFilpTransform() const
{
    QPointF center = itemRect().center();
    QTransform trans(this->isFilped(PageItem::EFilpHor) ? -1 : 1, 0, 0,
                     0, this->isFilped(PageItem::EFilpVer)  ? -1 : 1, 0,
                     0, 0, 1);

    return (QTransform::fromTranslate(-center.x(), -center.y()) * trans * QTransform::fromTranslate(center.x(), center.y()));
}

//void PageItem::rasterToSelfLayer(bool deleteSelf)
//{
//    QPixmap pix = this->rasterSelf();
//    QTransform itemToLayer = this->itemTransform(this->layer());
//    QPainter painter(&this->layer()->layerImage());

//    //先保证painter的坐标系是layer，而不是layer的image
//    painter.translate(-this->layer()->boundingRect().topLeft());

//    //设置了从item到layer的矩阵变化，那么接下来的绘制可以假装是在item上绘制
//    painter.setTransform(itemToLayer, true);

//    painter.drawPixmap(this->rect().toRect(), pix);

//    if (deleteSelf) {
//        auto scene = pageScene();
//        auto pItem = this;
//        QMetaObject::invokeMethod(scene, [ = ]() {
//            scene->removeCItem(pItem);
//        }, Qt::QueuedConnection);
//    }
//}

QPointF PageItem::mapFromPageScene(const QPointF &posInDScene) const
{
//    //sceneTransform() is local item to scene,then sceneTransform().inverted() is scene to local item
//    auto trans = getFilpTransform() * sceneTransform().inverted()  /** getFilpTransform()*/;
//    auto result = trans.map(posInDScene);
//    return result;

    return getFilpTransform().map(mapFromScene(posInDScene));
}

QPointF PageItem::mapToPageScene(const QPointF &posInThis) const
{
//    auto trans = sceneTransform() * getFilpTransform().inverted();
//    auto result = trans.map(posInThis);
//    return result;

    return mapToScene(getFilpTransform().inverted().map(posInThis));
}

void PageItem::setPaintSelectingBorder(bool b)
{
    PageItem_private::paintSelectedBorderLine = b;
}

bool PageItem::isPaintSelectingBorder()
{
    return PageItem_private::paintSelectedBorderLine;
}

void PageItem::updateViewport()
{
    if (pageView() != nullptr) {
        pageView()->viewport()->update();
    }
}

// Cppcheck检测函数没有使用到
//void CGraphicsItem::drawItem(QPainter *painter, const QStyleOptionGraphicsItem *option)
//{
//    paint(painter, option, nullptr);
//}


RegistItemHelper::RegistItemHelper(const QString &itemname, int type)
{
    PageItem::registerItem(itemname, type);
}
