// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cgraphicsitemselectedmgr.h"
#include "cdrawparamsigleton.h"
#include "frame/cviewmanagement.h"
#include "frame/cgraphicsview.h"
#include "cgraphicsitem.h"
#include "cgraphicspenitem.h"
#include "toptoolbar.h"
#include "cgraphicstextitem.h"
#include "application.h"
#include "cdrawtoolmanagersigleton.h"
#include "ccuttool.h"
#include "cgraphicsitemevent.h"
#include "cdrawscene.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsScene>
#include <QDebug>
#include <QStyleOptionGraphicsItem>

CGraphicsItemGroup::~CGraphicsItemGroup()
{
}

CGraphicsItemGroup::CGraphicsItemGroup(EGroupType tp, const QString &nam)
    : QObject(nullptr), CGraphicsItem(nullptr)
{
    setGroupType(tp);

    setName(nam);

    m_listItems.clear();

    initHandle();

    static int s_indexForTest = 0;
    _indexForTest = s_indexForTest;
    ++s_indexForTest;


    //qWarning() << "CGraphicsItemGroup creat = " << (++numbbbbbbbb);
}

DrawAttribution::SAttrisList CGraphicsItemGroup::attributions()
{
    //return DrawAttribution::SAttrisList();

    auto allitms = this->items();
    DrawAttribution::SAttrisList attris;
    if (!allitms.isEmpty()) {
        attris = allitms.first()->attributions();
        foreach (auto it, allitms) {
            attris = attris.insected(it->attributions());
        }
    }
    return attris;
}

void CGraphicsItemGroup::setAttributionVar(int attri, const QVariant &var, int phase)
{
    auto items = this->getBzItems(true);
    foreach (auto i, items) {
        if (i->attributions().haveAttribution(attri))
            i->setAttributionVar(attri, var, phase);
    }
}

QString CGraphicsItemGroup::name() const
{
    return _name;
}

void CGraphicsItemGroup::setName(const QString &name)
{
    _name = name;
}

CGraphicsItemGroup::EGroupType CGraphicsItemGroup::groupType() const
{
    return _type;
}

void CGraphicsItemGroup::setGroupType(CGraphicsItemGroup::EGroupType tp)
{
    _type = tp;
}

bool CGraphicsItemGroup::isTopBzGroup() const
{
    return bzGroup() == nullptr;
}

void CGraphicsItemGroup::setCancelable(bool enable)
{
    _isCancelable = enable;
}

bool CGraphicsItemGroup::isCancelable() const
{
    return _isCancelable;
}

void CGraphicsItemGroup::clear()
{
    prepareGeometryChange();
    foreach (CGraphicsItem *item, m_listItems) {
        item->setBzGroup(nullptr);
    }
    this->_roteAgnel = 0;
    this->resetTransform();
    this->setBzGroup(nullptr);
    m_listItems.clear();
    updateBoundingRect();
    if (groupType() == ESelectGroup) {
        updateAttributes();
    }
}

//QRectF CGraphicsItemGroup::boundingRect() const
//{
//    return _rct;
//}

void CGraphicsItemGroup::updateShape()
{
    updateBoundingRect(true);
}

void CGraphicsItemGroup::updateBoundingRect(bool force)
{
    if (m_operatingType == 3)
        return;

    //qWarning() << "updateBoundingRect ======= force = " << force << "is normol group = " << (groupType() == ENormalGroup);
    prepareGeometryChange();

    QRectF rect(0, 0, 0, 0);

    QRectF _rct;
    if (m_listItems.size() > 1) {
        //没有旋转过,那么重新获取大小
        if (transform().isIdentity() || force) {
            auto items = groupType() == ESelectGroup ? m_listItems : getBzItems(true);
            foreach (QGraphicsItem *item, items) {
                CGraphicsItem *pItem = dynamic_cast<CGraphicsItem *>(item);
                if (pItem != nullptr && pItem->type() != BlurType) {
                    rect = rect.united(pItem->mapRectToScene(pItem->selectedBoundingRect()));
                }
            }
            this->setTransformOriginPoint(rect.center());
            this->setRotation(0);
            if (force) {
                resetTransform();
                _roteAgnel = 0;
            }
            _rct = mapFromScene(rect).boundingRect();

        }
    } else if (m_listItems.size() == 1) {
        CGraphicsItem *pItem = m_listItems.first();

        //不存在节点的图元就需要多选图元进行管理
        if (!pItem->isSizeHandleExisted() || drawScene()->isNormalGroupItem(pItem)) {

            _rct = pItem->selectedBoundingRect();

            this->setTransformOriginPoint(pItem->transformOriginPoint());

            this->setRotation(pItem->rotation());

            this->setPos(pItem->pos());

            this->setTransform(pItem->transform());

            _roteAgnel = pItem->drawRotation();
        }
    } else {
        _rct = rect;
    }

    setRect(_rct);
    //m_boundingRectTrue = _rct;
    //m_boundingRect = _rct;

    updateHandlesGeometry();
}

QPainterPath CGraphicsItemGroup::getSelfOrgShape() const
{
    QPainterPath path;

    path.addRect(this->boundingRect());

    return path;
}

qreal CGraphicsItemGroup::incLength() const
{
    return 0;
}

int CGraphicsItemGroup::count()
{
    return m_listItems.count();
}

int CGraphicsItemGroup::allCount() const
{
    return getBzItems(true).count();
}

QList<CGraphicsItem *> CGraphicsItemGroup::items(bool recursiveFind) const
{
    if (!recursiveFind)
        return m_listItems;
    QList<CGraphicsItem *> result;
    for (auto p : m_listItems) {
        result.append(p);
        if (p->isBzGroup()) {
            CGraphicsItemGroup *pGroup = static_cast<CGraphicsItemGroup *>(p);
            result.append(pGroup->items(recursiveFind));
        }
    }
    return result;
}

QList<CGraphicsItem *> CGraphicsItemGroup::getBzItems(bool recursiveFind) const
{
    QList<CGraphicsItem *> result;
    for (auto p : m_listItems) {
        if (p->isBzItem()) {
            result.append(p);
        } else if (recursiveFind && p->isBzGroup()) {
            CGraphicsItemGroup *pGroup = static_cast<CGraphicsItemGroup *>(p);
            result.append(pGroup->getBzItems(recursiveFind));
        }
    }
    return result;
}

QList<CGraphicsItemGroup *> CGraphicsItemGroup::getGroups(bool recursiveFind) const
{
    QList<CGraphicsItemGroup *> result;
    for (auto p : m_listItems) {
        if (p->isBzGroup()) {
            CGraphicsItemGroup *pGroup = static_cast<CGraphicsItemGroup *>(p);
            result.append(pGroup);

            if (recursiveFind)
                result.append(pGroup->getGroups(recursiveFind));
        }
    }
    return result;
}

qreal CGraphicsItemGroup::drawZValue()
{
    if (_type == ENormalGroup) {
        if (_zIsDirty) {
            updateZValue();
        }
    }
    return zValue();
}

void CGraphicsItemGroup::updateZValue()
{
    if (m_listItems.isEmpty())
        return;

    //1.保证孩子组合的z值是正确的
    for (auto child : m_listItems) {
        if (child->isBzGroup()) {
            auto pChildGp = static_cast<CGraphicsItemGroup *>(child);
            if (pChildGp->_zIsDirty) {
                pChildGp->updateZValue();
            }
        }
    }

    //2.得到顺序的孩子们,并获得z值最小的孩子图元
    auto sortedChildren = PageScene::returnSortZItems(m_listItems, PageScene::EDesSort);
    auto minZItem = sortedChildren.last();

    //3.设置组合的z值与最小z值的孩子图元一样,并保证孩子图元在组合上
    qreal z = minZItem->zValue();
    setZValue(z);
    this->stackBefore(minZItem);

    _maxZ = sortedChildren.first()->zValue();

    //4.z值已经获取到不再是脏的
    _zIsDirty = false;
}

void CGraphicsItemGroup::add(CGraphicsItem *item, bool updateAttri, bool updateRect)
{
    if (item == nullptr)
        return;

    //防止添加自己
    if (item == this)
        return;

    if (!m_listItems.contains(item)) {
        if (dynamic_cast<CGraphicsItem *>(item) != nullptr) {
            m_listItems.push_back(item);
            if (groupType() == ESelectGroup)
                item->setSelected(true);
            item->setBzGroup(this);

            if (updateAttri) {
                updateAttributes();
            }
            itemAdded(item);
            emit childrenChanged(m_listItems);
            _zIsDirty = true;   //置为true下次获取就会刷新z值
        }
        _addTp = EOneByOne;
    }

    if (updateRect)
        updateBoundingRect(true);
}

void CGraphicsItemGroup::remove(CGraphicsItem *item, bool updateAttri, bool updateRect)
{
    //防止删除自己
    if (item == this)
        return;

    if (m_listItems.contains(item)) {
        m_listItems.removeOne(item);

        if (groupType() == ESelectGroup)
            item->setSelected(false);

        item->setBzGroup(nullptr);
        if (updateAttri)
            updateAttributes(true);

        itemRemoved(item);
        emit childrenChanged(m_listItems);
        _zIsDirty = true;   //置为true下次获取就会刷新z值
    }

    if (updateRect)
        updateBoundingRect(true);
}

void CGraphicsItemGroup::move(QPointF beginPoint, QPointF movePoint)
{
    foreach (CGraphicsItem *item, m_listItems) {
        item->move(beginPoint, movePoint);
    }
    updateBoundingRect();
}

int CGraphicsItemGroup::type() const
{
    return MgrType;
}

QPointF CGraphicsItemGroup::getCenter(CSizeHandleRect::EDirection dir)
{
    QPointF center;
    CGraphicsItem *pItem = this;
    QRectF rect = pItem->rect();

//    if (count() > 0) {
//        auto bzItems = this->getBzItems(true);

//        if (bzItems.count() > 0) {
//            qreal maxX = bzItems.first()->rect().right();
//            qreal minX = bzItems.first()->rect().left();

//            qreal maxY = bzItems.first()->rect().bottom();
//            qreal minY = bzItems.first()->rect().top();

//            for (auto p : bzItems) {
//                QRectF rctF = p->mapRectToItem(this, p->rect());
//                if (rctF.left() < minX) {
//                    minX = rctF.left();
//                }
//                if (rctF.right() > maxX) {
//                    maxX = rctF.right();
//                }
//                if (rctF.bottom() > maxY) {
//                    maxY = rctF.bottom();
//                }
//                if (rctF.top() < minY) {
//                    minY = rctF.top();
//                }
//            }
//            rect = QRectF(QPointF(minX, minY), QPointF(maxX, maxY));
//        }
//    }

    switch (dir) {
    case CSizeHandleRect::LeftTop: {
        center = rect.bottomRight();
        break;
    }
    case CSizeHandleRect::Top:
        center = QPointF(rect.center().x(), rect.bottom());
        break;
    case CSizeHandleRect::RightTop:
        center = rect.bottomLeft();
        break;
    case CSizeHandleRect::Right:
        center = QPointF(rect.left(), rect.center().y());
        break;
    case CSizeHandleRect::RightBottom:
        center = rect.topLeft();
        break;
    case CSizeHandleRect::Bottom:
        center = QPointF(rect.center().x(), rect.top());
        break;
    case CSizeHandleRect::LeftBottom:
        center = rect.topRight();
        break;
    case CSizeHandleRect::Left:
        center = QPointF(rect.right(), rect.center().y());
        break;
    case CSizeHandleRect::Rotation:
        center = rect.center();
        break;
    default:
        center = rect.center();
        break;
    }
    return center;
}

void CGraphicsItemGroup::operatingBegin(CGraphItemEvent *event)
{
    for (CGraphicsItem *pItem : m_listItems) {
        QTransform thisToItem = this->itemTransform(pItem);
        CGraphItemEvent *childEvent = event->creatTransDuplicate(thisToItem, pItem->rect().size());
        childEvent->setItem(pItem);
        pItem->operatingBegin(childEvent);
        delete childEvent;
    }
    CGraphicsItem::operatingBegin(event);
}

void CGraphicsItemGroup::operating(CGraphItemEvent *event)
{
    bool accept = testOpetating(event);
    if (accept) {
        for (CGraphicsItem *pItem : m_listItems) {
            //得到将自身坐标系映射到其他图元pItem坐标系的矩阵
            QTransform thisToItem = this->itemTransform(pItem);
            CGraphItemEvent *childEvent = event->creatTransDuplicate(thisToItem, pItem->rect().size());
            childEvent->setItem(pItem);
            //将自身要做的转换矩阵映射到pItem上
            QTransform childDoTrans = thisToItem.inverted() * event->trans() * thisToItem;
            childEvent->setTrans(childDoTrans);
            pItem->operating(childEvent);
            delete childEvent;
        }
        CGraphicsItem::operating(event);
    }
}

bool CGraphicsItemGroup::testOpetating(CGraphItemEvent *event)
{
    //1.先判断自身是否能接受这个操作事件
    bool accept = CGraphicsItem::testOpetating(event);
    if (accept) {
        //2.再判断孩子们是否能接受这个操作事件
        if (event->type() == CGraphItemEvent::EScal) {
            auto doItems = items(true);
            for (CGraphicsItem *pItem : m_listItems) {
                QTransform thisToItem = this->itemTransform(pItem);
                CGraphItemEvent *childEvent = event->creatTransDuplicate(thisToItem, pItem->rect().size());
                childEvent->setItem(pItem);
                childEvent->setDriverEvent(event);
                QTransform childDoTrans = thisToItem.inverted() * event->trans() * thisToItem;
                childEvent->setTrans(childDoTrans);
                bool isItemAccept = pItem->testOpetating(childEvent);
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

bool CGraphicsItemGroup::testScaling(CGraphItemScalEvent *event)
{
    //当组合大小是无效时(隐藏时),那么默认绕过组合的判定,比如单选直线的情况,因为直线拥有自己的节点,那么就会隐藏选择框,隐藏的方式就是设置rect为无效
    if (!rect().isValid())
        return true;

    bool accept = true;
    QTransform trans = event->trans();
    QRectF rct = this->rect();
    QPointF pos1 = trans.map(rct.topLeft());
    QPointF pos4 = trans.map(rct.bottomRight());

    QRectF wantedRect(pos1, pos4);
    event->setMayResultPolygon(this->mapToScene(wantedRect));
    accept = wantedRect.isValid();
    return accept;
}

void CGraphicsItemGroup::operatingEnd(CGraphItemEvent *event)
{
    for (CGraphicsItem *pItem : m_listItems) {
        QTransform thisToItem = this->itemTransform(pItem);
        CGraphItemEvent *childEvent = event->creatTransDuplicate(thisToItem, pItem->rect().size());
        childEvent->setItem(pItem);
        childEvent->setTrans(thisToItem.inverted() * event->trans() * thisToItem);
        pItem->operatingEnd(childEvent);
        delete childEvent;
    }
    CGraphicsItem::operatingEnd(event);
}

void CGraphicsItemGroup::doScaling(CGraphItemScalEvent *event)
{
    QTransform trans = event->trans();
    QRectF rct = this->rect();
    QPointF pos1 = trans.map(rct.topLeft());
    QPointF pos4 = trans.map(rct.bottomRight());
    setRect(QRectF(pos1, pos4));
    updateHandlesGeometry();
}

void CGraphicsItemGroup::doScalEnd(CGraphItemScalEvent *event)
{
    Q_UNUSED(event)
    if (groupType() == ENormalGroup) {
        QRectF unitRect;
        for (int i = 0; i < m_listItems.size(); ++i) {
            auto item = m_listItems.at(i);
            auto rectInGroup = item->mapRectToItem(this, item->rect());
            if (!rect().adjusted(-1, -1, 1, 1).contains(rectInGroup)) {
                updateBoundingRect(true);
                return;
            }
            unitRect = unitRect.united(rectInGroup);
        }
        if (unitRect != rect())
            updateBoundingRect(true);

    } else {
        updateBoundingRect(true);
    }
}

QRectF CGraphicsItemGroup::rect() const
{
    return m_boundingRect;
}

void CGraphicsItemGroup::loadGraphicsUnit(const CGraphicsUnit &data)
{
    if (data.data.pGroup != nullptr) {
        setName(data.data.pGroup->name);
        setGroupType(EGroupType(data.data.pGroup->groupType));
        setCancelable(data.data.pGroup->isCancelAble);
    }
    setRect(data.head.rect);
    //loadHeadData(data.head);
    this->setPen(data.head.pen);
    this->setBrush(data.head.brush);

    _roteAgnel = data.head.rotate;
    this->setPos(data.head.pos);
    this->setZValue(data.head.zValue);
    _blurInfos = data.head.blurInfos;
    this->setTransform(data.head.trans);

    if (isCached()) {
        updateBlurPixmap(true);
        resetCachePixmap();
    }
}

CGraphicsUnit CGraphicsItemGroup::getGraphicsUnit(EDataReason reson) const
{
    CGraphicsUnit unit;

    unit.reson = reson;

    unit.head.dataType = this->type();
    unit.head.dataLength = sizeof(SGraphicsRectUnitData);
    unit.head.pen = this->pen();
    unit.head.brush = this->brush();
    unit.head.pos = this->pos();
    unit.head.rotate = this->drawRotation();
    unit.head.zValue = this->zValue();
    unit.head.trans  = this->transform();
    unit.head.rect   = this->rect();

    unit.data.pGroup = new SGraphicsGroupUnitData;
    unit.data.pGroup->name = this->name();
    unit.data.pGroup->isCancelAble = this->_isCancelable;
    unit.data.pGroup->groupType = this->_type;


    return unit;
}

void CGraphicsItemGroup::setNoContent(bool b, bool children)
{
    setFlag(ItemHasNoContents, b);

    if (children) {
        QList<CGraphicsItem *> chidren =  items(true);
        foreach (QGraphicsItem *pItem, chidren) {
            pItem->setFlag(ItemHasNoContents, b);
        }
    }
}

void CGraphicsItemGroup::setRecursiveScene(PageScene *scene)
{
    for (auto p : m_listItems) {
        if (p->isBzGroup()) {
            CGraphicsItemGroup *pGp = static_cast<CGraphicsItemGroup *>(p);
            pGp->setRecursiveScene(scene);
        }
        p->setScene(scene);
    }
    this->setScene(scene);
}

qreal CGraphicsItemGroup::getMinZ() const
{
    auto p = minZItem();
    if (p != nullptr) {
        return p->zValue();
    }
    return 0;
}

CGraphicsItem *CGraphicsItemGroup::minZItem() const
{
    auto list = getBzItems(true);
    if (list.isEmpty())
        return nullptr;
    return PageScene::returnSortZItems(list).last();
}

CGraphicsItem *CGraphicsItemGroup::getLogicFirst() const
{
    CGraphicsItem *pBaseItem = nullptr;
    if (this->addType() == EOneByOne) {
        auto list = items();
        if (!list.isEmpty()) {

            auto pFirst = list.first();
            if (pFirst->isBzGroup())
                pBaseItem = static_cast<CGraphicsItemGroup *>(pFirst)->minZItem();
            else {
                pBaseItem = pFirst;
            }
        }
    } else {
        pBaseItem = minZItem();
    }
    return pBaseItem;
}

void CGraphicsItemGroup::setAddType(CGraphicsItemGroup::EAddType tp)
{
    _addTp = tp;
}

CGraphicsItemGroup::EAddType CGraphicsItemGroup::addType()const
{
    return _addTp;
}

void CGraphicsItemGroup::setRect(const QRectF rct)
{
    prepareGeometryChange();
    m_boundingRect = rct;
    m_boundingRectTrue = rct;
}

void CGraphicsItemGroup::rasterToSelfLayer(bool deleteSelf)
{
    CGraphicsItem::paintInteractBorderLine = false;
    auto items = getBzItems(true);
    PageScene::sortZ(items, PageScene::EAesSort);
    foreach (auto item, items) {
        item->rasterToSelfLayer(deleteSelf);
    }
    CGraphicsItem::paintInteractBorderLine = true;
}

void CGraphicsItemGroup::updateHandlesGeometry()
{
    const QRectF &geom = this->boundingRect();

    //如果大小无效了那么就没有显示的必要了
    if (geom.isValid()) {
        show();
    } else {
        hide();
        return;
    }

    qreal qRoty = 0;
    const Handles::iterator hend =  m_handles.end();
    QPointF pos;
    for (Handles::iterator it = m_handles.begin(); it != hend; ++it) {
        CSizeHandleRect *hndl = *it;
        qreal w = hndl->boundingRect().width();
        qreal h = hndl->boundingRect().height();
        switch (hndl->dir()) {
        case CSizeHandleRect::LeftTop:
            hndl->move(geom.x() - w / 2, geom.y() - h / 2);
            break;
        case CSizeHandleRect::Top:
            hndl->move(geom.x() + geom.width() / 2 - w / 2, geom.y() - h / 2);
            break;
        case CSizeHandleRect::RightTop:
            hndl->move(geom.x() + geom.width() - w / 2, geom.y() - h / 2);
            break;
        case CSizeHandleRect::Right:
            pos = QPointF(geom.x() + geom.width() - w / 2, geom.y() + geom.height() / 2 - h / 2);
            hndl->move(geom.x() + geom.width() - w / 2, geom.y() + geom.height() / 2 - h / 2);
            break;
        case CSizeHandleRect::RightBottom:
            hndl->move(geom.x() + geom.width() - w / 2, geom.y() + geom.height() - h / 2);
            break;
        case CSizeHandleRect::Bottom:
            hndl->move(geom.x() + geom.width() / 2 - w / 2, geom.y() + geom.height() - h / 2);
            break;
        case CSizeHandleRect::LeftBottom:
            hndl->move(geom.x() - w / 2, geom.y() + geom.height() - h / 2);
            break;
        case CSizeHandleRect::Left:
            hndl->move(geom.x() - w / 2, geom.y() + geom.height() / 2 - h / 2);
            break;
        case CSizeHandleRect::Rotation: {
            if (m_listItems.count() > 1) {
                hndl->hide();
            } else {
                hndl->show();
                qRoty = geom.y() - h - h / 2;
                hndl->move(geom.x() + geom.width() / 2 - w / 2, qRoty);
            }
            break;
        }
        default:
            break;
        }
    }

    setHandleVisible(true, CSizeHandleRect::InRect);
    setHandleVisible(m_listItems.count() == 1, CSizeHandleRect::Rotation);
    if (m_listItems.count() == 1) {
        CGraphicsItem *pItem = m_listItems.first();
        if (pItem->type() == TextType) {
            //自动调整大小的情况不需要显示节点
            bool visble = !dynamic_cast<CGraphicsTextItem *>(pItem)->isAutoAdjustSize();
            this->setHandleVisible(visble, CSizeHandleRect::LeftTop);
            this->setHandleVisible(visble, CSizeHandleRect::Top);
            this->setHandleVisible(visble, CSizeHandleRect::RightTop);
            this->setHandleVisible(visble, CSizeHandleRect::RightBottom);
            this->setHandleVisible(visble, CSizeHandleRect::Bottom);
            this->setHandleVisible(visble, CSizeHandleRect::LeftBottom);
            this->setHandleVisible(visble, CSizeHandleRect::Rotation);
        } else if (pItem->type() == BlurType) {
            this->setHandleVisible(false, CSizeHandleRect::Rotation);
        }
    }
}

void CGraphicsItemGroup::updateAttributes(bool showTitle)
{
    Q_UNUSED(showTitle)
    if (groupType() != ESelectGroup)
        return;
    //　选中图元进行属性刷新
    if (drawScene() != nullptr)
        drawScene()->updateAttribution();
}

void CGraphicsItemGroup::setHandleVisible(bool visble, CSizeHandleRect::EDirection dirHandle)
{
    for (Handles::iterator it = m_handles.begin(); it != m_handles.end(); ++it) {
        CSizeHandleRect *hndl = *it;
        if (hndl->dir() == dirHandle || dirHandle == CSizeHandleRect::InRect) {
            hndl->setVisible(visble);

            if (hndl->type() == dirHandle)
                break;
        }
    }
}

void CGraphicsItemGroup::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
//    Q_UNUSED(option)
//    Q_UNUSED(widget)
//    if (groupType() == ENormalGroup) {
//        if (_zIsDirty) {
//            updateZValue();
//        }
//        paintMutBoundingLine(painter, option);

//        return;
//    }

//    bool paintBorder = isSelected() || groupType() == ESelectGroup;
//    if (paintBorder) {
//        painter->setClipping(false);
//        QPen pen;

//        // painter->setRenderHint(QPainter::Antialiasing, true);
//        pen.setWidthF(1 / option->levelOfDetailFromTransform(painter->worldTransform()));
//        pen.setColor(QColor("#BBBBBB"));

//        painter->setPen(pen);
//        painter->setBrush(QBrush(Qt::NoBrush));
//        painter->drawRect(this->boundingRect());
//        painter->setClipping(true);
//    }
    CGraphicsItem::paint(painter, option, widget);
}

void CGraphicsItemGroup::paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    Q_UNUSED(option)
    //Q_UNUSED(widget)
    if (groupType() == ENormalGroup) {
        if (_zIsDirty) {
            updateZValue();
        }
        paintMutBoundingLine(painter, option);

        return;
    }

    bool paintBorder = isSelected() || groupType() == ESelectGroup;
    if (paintBorder) {
        painter->setClipping(false);
        QPen pen;

        // painter->setRenderHint(QPainter::Antialiasing, true);
        pen.setWidthF(1 / option->levelOfDetailFromTransform(painter->worldTransform()));
        pen.setColor(QColor("#BBBBBB"));

        painter->setPen(pen);
        painter->setBrush(QBrush(Qt::NoBrush));
        painter->drawRect(this->boundingRect());
        painter->setClipping(true);
    }
}

void CGraphicsItemGroup::initHandle()
{
    clearHandle();

    m_handles.reserve(CSizeHandleRect::None);

    if (groupType() == ESelectGroup) {
        for (int i = CSizeHandleRect::LeftTop; i <= CSizeHandleRect::Rotation; ++i) {
            CSizeHandleRect *shr = nullptr;
            if (i == CSizeHandleRect::Rotation) {
                shr = new CSizeHandleRect(this, static_cast<CSizeHandleRect::EDirection>(i),
                                          QString(":/theme/light/images/mouse_style/icon_rotate.svg"));

            } else {
                shr = new CSizeHandleRect(this, static_cast<CSizeHandleRect::EDirection>(i));
            }
            m_handles.push_back(shr);
        }
    }

    updateBoundingRect();

    this->setFlag(QGraphicsItem::ItemIsMovable, true);
    this->setFlag(QGraphicsItem::ItemIsSelectable, true);
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    this->setAcceptHoverEvents(true);
}
