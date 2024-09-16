// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "drawtool.h"
#include "pagescene.h"
#include "pagecontext.h"
#include "globaldefine.h"

#include "cgraphicsproxywidget.h"
#include "pageview.h"
#include "ctextedit.h"
#include "cundoredocommand.h"
#include "drawboardtoolmgr.h"
#include "attributemanager.h"
#include "layeritem.h"
#include "pageitem_p.h"
#include "selectionitem.h"
#include "cgraphicsitemevent.h"

#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QRect>
#include <QGraphicsView>
#include <QPainterPath>
#include <QtMath>
#include <QScrollBar>
#include <QWindow>
#include <QApplication>
#include <QDesktopWidget>

#ifdef USE_DTK
#include <DGuiApplicationHelper>
DWIDGET_USE_NAMESPACE
#endif

class TrashBinItem: public PageItem
{
public:
    enum Type {TrashBin = TrashBinItemType};
    explicit TrashBinItem(PageScene *s)
    {
        s->addItem(this);
        this->setVisible(false);
    }
    int type()const {return TrashBin;}
    QRectF itemRect()const
    {
        return QRectF(0, 0, 0, 0);
    }
};
class PageScene::PageScene_private
{
public:
    explicit PageScene_private(PageScene *qq): q(qq)
    {
    }
    void initScene()
    {
        q->clear();
        //获取屏幕分辨率
        QDesktopWidget *desktopWidget = QApplication::desktop();
        QRect screenRect = desktopWidget->screenGeometry();
        //需要乘以系统缩放系数才是最终的大小
        screenRect = QRect(0, 0, qRound(screenRect.width() * desktopWidget->devicePixelRatioF()),
                           qRound(screenRect.height() * desktopWidget->devicePixelRatioF()));

        q->setSceneRect(screenRect);
        resetSceneBackgroundBrush();

        selectionItem = new SelectionItem(q);

        LayerItem *pLayer = new LayerItem(q);

        q->insertTopLayer(pLayer);
        q->setCurrentTopLayer(pLayer);

        trashBin = new TrashBinItem(q);
    }
    void resetSceneBackgroundBrush()
    {
        int themeType = 1;
#ifdef USE_DTK
        themeType = DGuiApplicationHelper::instance()->themeType();
#endif
        if (themeType == 1) {
            q->setBackgroundBrush(QColor(248, 248, 251));
        } else if (themeType == 2) {
            q->setBackgroundBrush(QColor(35, 35, 35));
        }
        q->setBgColor(Qt::white);
    }
    void mouseEvent(QGraphicsSceneMouseEvent *mouseEvent)
    {
        switch (mouseEvent->type()) {
        case QEvent::GraphicsSceneMousePress:
            qDebug() << "qt to do SceneMousePress-----";
            q->QGraphicsScene::mousePressEvent(mouseEvent);
            break;
        case QEvent::GraphicsSceneMouseMove:
            //qDebug() << "qt to do SceneMouseMove----- press = " << mouseEvent->buttons();
            q->QGraphicsScene::mouseMoveEvent(mouseEvent);
            break;
        case QEvent::GraphicsSceneMouseRelease:
            qDebug() << "qt to do SceneMouseRelease-----";
            q->QGraphicsScene::mouseReleaseEvent(mouseEvent);
            break;
        case QEvent::GraphicsSceneMouseDoubleClick:
            qDebug() << "qt to do SceneMouseDoubleClick-----";
            q->QGraphicsScene::mouseDoubleClickEvent(mouseEvent);
            break;
        default:
            break;
        }
    }
    bool gestureEvent(QGestureEvent *event)
    {
        if (QGesture *pinch = event->gesture(Qt::PinchGesture))
            pinchTriggered(static_cast<QPinchGesture *>(pinch));
        return true;
    }

    void pinchTriggered(QPinchGesture *gesture)
    {
        QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags();
        if (changeFlags & QPinchGesture::RotationAngleChanged) {
            qreal rotationDelta = gesture->rotationAngle() - gesture->lastRotationAngle();
            Q_UNUSED(rotationDelta);
        }
        if (changeFlags & QPinchGesture::ScaleFactorChanged) {
            q->firstPageView()->scaleWithCenter(gesture->scaleFactor(), q->firstPageView()->viewport()->mapFromGlobal(gesture->centerPoint().toPoint()));
        }
        if (gesture->state() == Qt::GestureFinished) {
        }
        q->firstPageView()->update();
    }

    PageScene *q;

    LayerItem *m_currentLayer = nullptr;
    SelectionItem *selectionItem = nullptr;
    TrashBinItem *trashBin = nullptr;
    QList<LayerItem *>    topLayers;

    bool blockZAssign = false;
    bool blockSel     = false;
    bool blocked = false;
    bool bDrawForeground = true;
    bool paintSelectedBorderLine = true;

    QColor _bgColor = QColor(Qt::white);
};

PageItem *_pageSceneSelectItem(PageScene *scene)
{
    return scene->d_PageScene()->selectionItem;
}

PageScene::PageScene(PageContext *pageCxt)
    : QGraphicsScene(pageCxt), PageScene_d(new PageScene_private(this))
{
    d_PageScene()->initScene();

#ifdef USE_DTK
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this,
    [ = ](DGuiApplicationHelper::ColorType themeType) {
        d_PageScene()->resetSceneBackgroundBrush();
    });
#endif
}

PageScene::~PageScene()
{
}

PageContext *PageScene::pageContext() const
{
    return qobject_cast<PageContext *>(parent());
}

Page *PageScene::page() const
{
    if (this->firstPageView() == nullptr)
        return nullptr;

    return this->firstPageView()->page();
}

SAttrisList PageScene::currentAttris() const
{
    SAttrisList attris = d_PageScene()->selectionItem->attributions();

    return attris;
}

void PageScene::insertTopLayer(LayerItem *pLayer, int index)
{
    Q_UNUSED(index)
    if (pLayer == nullptr)
        return;

    if (pLayer->layer() != nullptr) {
        return;
    }

    if (pLayer->parentPageItem() != nullptr) {
        pLayer->setParentPageItem(nullptr);
    }

//    if (!d_PageScene()->topLayers.contains(pLayer)) {
//        d_PageScene()->topLayers.insert(index, pLayer);
//        addPageItem(pLayer, nullptr);
//    }
    addPageItem(pLayer, nullptr);
}

void PageScene::removeTopLayer(LayerItem *pLayer)
{
    removePageItem(pLayer);
}

void PageScene::setCurrentTopLayer(LayerItem *pLayer)
{
    d_PageScene()->m_currentLayer = pLayer;
}

LayerItem *PageScene::currentTopLayer() const
{
    return d_PageScene()->m_currentLayer;
}

int PageScene::topLayerCount() const
{
    return d_PageScene()->topLayers.count();
}

QList<LayerItem *> PageScene::topLayers() const
{
    return d_PageScene()->topLayers;
}

void PageScene::selectPageItem(PageItem *item)
{
    if (d_PageScene()->blockSel)
        return;

    d_PageScene()->selectionItem->select(item);
}

void PageScene::selectPageItem(const QList<PageItem *> &items)
{
    if (d_PageScene()->blockSel)
        return;
    d_PageScene()->selectionItem->select(items);
}

void PageScene::notSelectPageItem(PageItem *item)
{
    if (d_PageScene()->blockSel)
        return;
    d_PageScene()->selectionItem->notSelect(item);
}

void PageScene::notSelectPageItem(const QList<PageItem *> &items)
{
    if (d_PageScene()->blockSel)
        return;
    d_PageScene()->selectionItem->notSelect(items);
}

void PageScene::selectAll()
{
    if (d_PageScene()->blockSel)
        return;

//    if (currentTopLayer() != nullptr) {
//        d_PageScene()->selectionItem->select(currentTopLayer()->items());
//        return;
//    }
    d_PageScene()->selectionItem->select(switchListClassTo<LayerItem, PageItem>(topLayers()));
}

void PageScene::clearSelections()
{
    if (d_PageScene()->blockSel)
        return;
    d_PageScene()->selectionItem->clear();
}

void PageScene::setPageItemSelected(PageItem *item, bool b)
{
    b ? selectPageItem(item) : notSelectPageItem(item);
}

void PageScene::setAttributionVar(int attri, const QVariant &var, int phase, bool autoCmdStack)
{
    //只记录开始和结束的变化，用于撤销恢复
    if (autoCmdStack && (phase == EChangedBegin || phase == EChanged))
        UndoStack::recordUndo(selectedPageItems());

    d_PageScene()->selectionItem->setAttributionVar(attri, var, phase);

    if (autoCmdStack && (phase == EChangedFinished || phase == EChanged)) {
        UndoStack::recordRedo(selectedPageItems());
        UndoStack::finishRecord(firstPageView()->stack());
    }
}

QColor PageScene::bgColor() const
{
    return d_PageScene()->_bgColor;
}

void PageScene::setBgColor(const QColor &c)
{
    d_PageScene()->_bgColor = c;
    update();
}

PageView *PageScene::firstPageView() const
{
    auto views = this->views();
    if (views.isEmpty())
        return nullptr;
    return qobject_cast<PageView *>(views.first());
}

void PageScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsScene::drawBackground(painter, rect);
    //painter->fillRect(sceneRect(), bgColor());
}

void PageScene::setCursor(const QCursor &cursor)
{
    if (page() != nullptr) {
        page()->setDrawCursor(cursor);
    }
}

void PageScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (isSceneBlocked())
        return;

    DrawTool *pTool =  page()->currentTool_p();
    if (nullptr != pTool) {

        ToolSceneEvent::CDrawToolEvents e = ToolSceneEvent::fromQEvent(mouseEvent, this);

        pTool->toolPressOnScene(&e.first());

        if (!e.first().isAccepted()) {
            d_PageScene()->mouseEvent(mouseEvent);
        }
    }
}

void PageScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (isSceneBlocked())
        return;

    DrawTool *pTool =  page()->currentTool_p();
    if (nullptr != pTool) {

        ToolSceneEvent::CDrawToolEvents e = ToolSceneEvent::fromQEvent(mouseEvent, this);

        pTool->toolMoveOnScene(&e.first());

        //已经处理完成，不用传递到框架,否则传递给框架继续处理
        if (!e.first().isAccepted()) {
            d_PageScene()->mouseEvent(mouseEvent);
        }
    }
}

void PageScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (isSceneBlocked())
        return;

    DrawTool *pTool =  page()->currentTool_p();
    if (nullptr != pTool) {
//        if (pTool->isWorking() && mouseEvent->button() != Qt::LeftButton) {
//            return;
//        }

        ToolSceneEvent::CDrawToolEvents e = ToolSceneEvent::fromQEvent(mouseEvent, this);

        pTool->toolReleaseOnScene(&e.first());

        //已经处理完成，不用传递到框架,否则传递给框架继续处理
        if (!e.first().isAccepted()) {
            d_PageScene()->mouseEvent(mouseEvent);
        }
    }
}

void PageScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (isSceneBlocked())
        return;

    DrawTool *pTool =  page()->currentTool_p();
    if (nullptr != pTool) {
        ToolSceneEvent::CDrawToolEvents e = ToolSceneEvent::fromQEvent(mouseEvent, this);

        pTool->tryConvertToDoubleClicked(&e.first());

        if (!e.first().isAccepted()) {
            d_PageScene()->mouseEvent(mouseEvent);
        }
    }
}

void PageScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    event->ignore();

    // Send the event to all items at this position until one item accepts the
    // event.

//    auto items = selectedPageItems();
//    //items.clear();
//    //items << d_PageScene()->selectionItem;
//    foreach (auto item, items) {
//        event->setPos(item->mapFromScene(event->pos()));
//        event->accept();
//        if (!sendEvent(item, event))
//            break;

//        if (event->isAccepted())
//            break;
//    }

    //QGraphicsScene::contextMenuEvent(event);

    DrawTool *pTool =  page()->currentTool_p();

    ToolSceneEvent::CDrawToolEvents e = ToolSceneEvent::fromQEvent(event, this);

    pTool->contextMenuEvent(&e.first());
}

bool PageScene::event(QEvent *event)
{
    QEvent::Type evType = event->type();
    if (evType == QEvent::TouchBegin || evType == QEvent::TouchUpdate || evType == QEvent::TouchEnd) {

        if (isSceneBlocked())
            return true;

        QTouchEvent *touchEvent = dynamic_cast<QTouchEvent *>(event);

        QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();

        DrawTool *pTool =  page()->currentTool_p();

        if (nullptr == pTool || (touchPoints.count() > 1 && pTool->toolType() == selection)) {
            //是一个手势，那么中断当前的updating操作
            if (pTool != nullptr)
                pTool->interrupt();
            return QGraphicsScene::event(event);
        }

        //解决触屏后鼠标隐藏但还可能鼠标的位置还是高亮的问题
        if (evType == QEvent::TouchBegin) {
            QAbstractButton *pBtn = pTool->toolButton();
            if (pBtn != nullptr) {
                pBtn->setAttribute(Qt::WA_UnderMouse, false);
                pBtn->update();
            }
        }

        bool accept = true;
        foreach (const QTouchEvent::TouchPoint tp, touchPoints) {
            ToolSceneEvent e = ToolSceneEvent::fromTouchPoint(tp, this, touchEvent);
            switch (tp.state()) {
            case Qt::TouchPointPressed:
                //表示触碰按下
                pTool->toolPressOnScene(&e);
                break;
            case Qt::TouchPointMoved:
                //触碰移动
                pTool->toolMoveOnScene(&e);
                break;
            case Qt::TouchPointReleased:
                //触碰离开
                pTool->toolReleaseOnScene(&e);
                break;
            default:
                break;
            }
            if (!e.isAccepted()) {
                accept = false;
            }
        }
        event->accept();

        if (evType == QEvent::TouchEnd)
            pTool->interrupt();//pTool->clearITE();

        if (accept)
            return true;

    } else if (event->type() == QEvent::Gesture) {
        int currentMode = page()->currentTool();
        if (currentMode == selection) {
            return d_PageScene()->gestureEvent(static_cast<QGestureEvent *>(event));
        }
    }
    return QGraphicsScene::event(event);
}

void PageScene::drawItems(QPainter *painter, int numItems, QGraphicsItem *items[], const QStyleOptionGraphicsItem options[], QWidget *widget)
{
    painter->setClipping(true);
    painter->setClipRect(sceneRect());

    QGraphicsScene::drawItems(painter, numItems, items, options, widget);

//    QStyleOptionGraphicsItem option;
//    for (int i = 0; i < _topLayers.count(); ++i) {
//        auto item = _topLayers[i];

//        //if (item->itemRect().isValid())
//        {
//            // Draw the item
//            painter->save();
//            painter->setTransform(item->sceneTransform(), true);
//            item->d_PageItem()->initStyleOption(&option, painter->worldTransform(), QRegion(), true);
//            PageItem::paintItemHelper(item, painter, &option, widget);
//            painter->restore();
//        }
//    }

//    if (selectionItem->itemRect().isValid()) {
//        painter->save();
//        painter->setTransform(selectionItem->sceneTransform(), true);
//        selectionItem->d_PageItem()->initStyleOption(&option, painter->worldTransform(), QRegion(), true);
//        PageItem::paintItemHelper(selectionItem, painter, &option, widget);
//        painter->restore();
//    }
}

void PageScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    //禁止绘制时返回
    if (!d_PageScene()->bDrawForeground)
        return;

    //绘制额外的前景显示，如框选等

    DrawTool *pTool = page()->currentTool_p();

    if (pTool != nullptr) {
        pTool->drawMore(painter, rect, this);
    }

    QGraphicsScene::drawForeground(painter, rect);
}

//GroupItem *PageScene::loadUnitItemTree(const UnitItemTree &info)
//{
//    //如果是顶层根管理类型(顶层) 那么先清空当前场景内的组合情况
//    if (info.gpType == GroupItem::EVirRootGroup)
//        destoryAllGroup();

//    GroupItem *pGroup = info.groupItem;

//    QList<PageItem *> items = info.bzItems;

//    foreach (auto it, info.childTrees) {
//        GroupItem *pChildGroup = loadUnitItemTree(it);
//        if (pChildGroup != nullptr)
//            items.append(pChildGroup);
//    }

//    if (!items.isEmpty()) {
//        if (pGroup == nullptr)
//            pGroup = creatGroup(items, info.gpType);
//        else {
//            pGroup->addToGroup(items);
//            pGroup->setRecursiveScene(this);
//            pGroup->updateBoundingRect();
//            if (!m_pGroups.contains(pGroup))
//                m_pGroups.append(pGroup);
//            selectItem(pGroup);
//        }
//        if (info.gpUnit.data.data.isValid())
//            pGroup->loadUnit(info.gpUnit);
//    }
//    return pGroup;
//}

//UnitTree_Comp PageScene::getGroupTreeInfo(GroupItem *pGroup, int reson,
//                                          bool processEvent)
//{
//    UnitTree_Comp info;

//    //为nullptr表示收集当前场景下的组合情况
//    if (pGroup == nullptr) {
//        foreach (auto pGroup, m_pGroups) {
//            if (processEvent)
//                qApp->processEvents();
//            if (pGroup->isTopGroup()) {
//                info.childTrees.append(getGroupTreeInfo(pGroup));
//            }
//        }
//        if (reson == ESaveToDDf) {
//            auto bzItems = getBzItems();
//            foreach (auto p, bzItems) {
//                if (p->pageGroup() == nullptr) {
//                    info.bzItems.append(p->getUnit(reson));
//                }
//            }
//        }
//        //info.setGroupType(GroupItem::EVirRootGroup);
//        info.treeUnit.head.dataType = GroupItemType;
//        return info;
//    }

//    info.treeUnit = pGroup->getUnit(reson);
//    info.setGroupType(pGroup->groupType());

//    //只允许有一个选择组合图元
//    if (info.groupType() == GroupItem::ESelectGroup) {
//        info.setGroupType(GroupItem::EVirRootGroup);
//        info.treeUnit.data.data.clear();
//    }

//    //按照升序进行保存,这样在被加载回来的时候z值可以从低到高正确
//    auto groups  = pGroup->getGroupItems();
//    auto fSortGroup = [ = ](GroupItem * gp1, GroupItem * gp2) {
//        return gp1->getMinZ() <= gp2->getMinZ();
//    };
//    // 调用了过时的函数“ qSort”，改用'std :: sort'
//    std::sort(groups.begin(), groups.end(), fSortGroup);

//    auto bzItems = pGroup->getNotGroupItems();
//    auto fSortItems = [ = ](PageItem * it1, PageItem * it2) {
//        return it1->zValue() <= it2->zValue();
//    };
//    std::sort(bzItems.begin(), bzItems.end(), fSortItems);

//    foreach (auto gp, groups) {
//        if (processEvent)
//            qApp->processEvents();
//        info.childTrees.append(getGroupTreeInfo(gp));
//    }
//    foreach (auto it, bzItems) {
//        if (processEvent)
//            qApp->processEvents();
//        info.bzItems.append(it->getUnit(reson));
//    }
//    return info;
//}


//GroupItem *PageScene::loadGroupTreeInfo(const UnitTree_Comp &info, bool notClear)
//{
//    //如果是非常规组合 那么先清空当前场景内的组合情况
//    if (info.groupType() != GroupItem::ENormalGroup && !notClear)
//        destoryAllGroup();

//    GroupItem *pGroup = nullptr;

//    QList<Unit> utItems = info.bzItems;
//    QList<PageItem *> items;
//    foreach (auto ut, utItems) {
//        PageItem *pItem = PageItem::creatItemInstance(ut.head.dataType, ut);
//        if (pItem != nullptr)
//            items.append(pItem);
//    }

//    foreach (auto it, info.childTrees) {
//        GroupItem *pChildGroup = loadGroupTreeInfo(it);
//        if (pChildGroup != nullptr)
//            items.append(pChildGroup);
//    }

//    if (!items.isEmpty()) {
//        pGroup = creatGroup(items, info.groupType());
//        if (info.treeUnit.data.data.isValid() && pGroup != nullptr)
//            pGroup->loadUnit(info.treeUnit);
//    }

//    return pGroup;
//}
bool isItemsZDownable(const QList<PageItem *> &items, int step,
                      QList<PageItem *> &siblings, QList<PageItem *> &moveItems, int &bottomItemIndexInSiblings)
{
    if (!PageScene::isPageItemsSibling(items))
        return false;

    moveItems = items;

    //1.首先判断图元们是否是一个父对象中,要移动的图元需要是处于一个父对象
    PageItem *parentItem = PageScene::samePageItemParent(moveItems);
    PageScene *scene = items.first()->pageScene();
    if (scene == nullptr && parentItem == nullptr)
        return false;

    //2.获取会受到影响的图元(按照z升序排列,即从z值小到大排序)
    siblings = parentItem == nullptr ? switchListClassTo<LayerItem, PageItem>(scene->topLayers()) : parentItem->childPageItems();

    //先排序，按照z升序排列,即从z值小到大排序，这样方便从下往上遍历一个一个向下移动(就不会相互影响到)
    sortZ(siblings, EAesSort);
    sortZ(moveItems, EAesSort);

    bottomItemIndexInSiblings = siblings.indexOf(moveItems.first());

    if (bottomItemIndexInSiblings <= 0) {
        //想要向下移动的图元中，最下面的一个图元已经是在已经在兄弟们的最下面了,所以这个图元不能再向下移动了，根据需求这种情况就不用考虑其他图元了，直接返回
        return false;
    }
    return true;
}
void moveItemsZDown(const QList<PageItem *> &items, int step, QHash<PageItem *, qreal> &invokedItems)
{
    QList<PageItem *> siblings;
    QList<PageItem *> moveItems;
    int bottomItemIndexInSiblings = 0;

    if (!isItemsZDownable(items, step, siblings, moveItems, bottomItemIndexInSiblings))
        return;

    //-1表示想要移动到最下层
    if (step == -1) {
        step = bottomItemIndexInSiblings;
    }

    //想要移动的最下层图元在兄弟图元中的索引决定了能向下移动的最大层数
    step = qMin(bottomItemIndexInSiblings, step);

    //QSet<PageItem *> invokedItems;
    qreal pagezvaluemax = 0;
    qreal pagezvalue = 0;
    for (int i = 0; i <  moveItems.size(); ++i) {
        if (i == 0) {
            int index = siblings.indexOf(moveItems[i]);

            int wantedBotIndex = index - step;
            PageItem *wantedBotItem = siblings.at(wantedBotIndex);

            pagezvaluemax = wantedBotItem->pageZValue();
            pagezvalue = pagezvaluemax;
            moveItems[i]->setPageZValue(pagezvalue);
            invokedItems.insert(moveItems[i], pagezvalue);
        } else {
            pagezvalue++;
            moveItems[i]->setPageZValue(pagezvalue);
            invokedItems.insert(moveItems[i], pagezvalue);
        }
    }
    for (int i = 0 ; i < siblings.size(); ++i) {
        if (siblings[i]->pageZValue() < pagezvaluemax) {
            invokedItems.insert(siblings[i], siblings[i]->pageZValue());
            continue;
        }
        if (!moveItems.contains(siblings[i])) {
            pagezvalue++;
            siblings[i]->setPageZValue(pagezvalue);
            invokedItems.insert(siblings[i], pagezvalue);
        }
    }
}
bool isItemsZUpable(const QList<PageItem *> &items, int step,
                    QList<PageItem *> &siblings, QList<PageItem *> &moveItems, int &topItemIndexInSiblings)
{
    if (!PageScene::isPageItemsSibling(items))
        return false;

    if (items.isEmpty())
        return false;

    moveItems = items;

    //1.首先判断图元们是否是一个组合的,要移动的图元需要是处于一个组合中的(或者都不处于组合)
    PageItem *parentItem = PageScene::samePageItemParent(moveItems);
    PageScene *scene = items.first()->pageScene();
    if (scene == nullptr && parentItem == nullptr)
        return false;

    //2.获取可能会受到影响的图元，即兄弟图元
    siblings = parentItem == nullptr ? switchListClassTo<LayerItem, PageItem>(scene->topLayers()) : parentItem->childPageItems();

    //先排序，按照z降序排列,即从z值大到小排序，这样方便从上往下遍历一个一个向上移动(就不会相互影响到)
    sortZ(siblings, EDesSort);
    sortZ(moveItems, EDesSort);

    topItemIndexInSiblings = siblings.indexOf(moveItems.first());

    if (topItemIndexInSiblings == 0) {
        //想要向上移动的图元中，最上面的一个图元已经是在已经在兄弟们的最上面了,所以这个图元不能再向上移动了，根据需求这种情况就不用考虑其他图元了，直接返回
        return false;
    }
    return true;
}
void moveItemsZUp(const QList<PageItem *> &items, int step, QHash<PageItem *, qreal> &invokedItems)
{
    QList<PageItem *> siblings;
    QList<PageItem *> moveItems;
    int topItemIndexInSiblings = 0;

    if (!isItemsZUpable(items, step, siblings, moveItems, topItemIndexInSiblings))
        return;

    //-1表示想要移动到最下层
    if (step == -1) {
        step = topItemIndexInSiblings;
    }

    //想要移动的最上层图元在兄弟图元中的索引决定了能向上移动的最大层数
    step = qMin(topItemIndexInSiblings, step);

    //QSet<PageItem *> invokedItems;
    qreal pagezvaluemax = 0;
    qreal pagezvalue = 0;
    for (int i = 0; i <  moveItems.size(); ++i) {
        if (i == 0) {
            int index = siblings.indexOf(moveItems[i]);

            int wantedBotIndex = index - step;
            PageItem *wantedBotItem = siblings.at(wantedBotIndex);

            pagezvaluemax = wantedBotItem->pageZValue();
            pagezvalue = pagezvaluemax;
            moveItems[i]->setPageZValue(pagezvalue);
            invokedItems.insert(moveItems[i], pagezvalue);
        } else {
            pagezvalue--;
            moveItems[i]->setPageZValue(pagezvalue);
            invokedItems.insert(moveItems[i], pagezvalue);
        }
    }
    for (int i = 0 ; i < siblings.size(); ++i) {
        if (siblings[i]->pageZValue() > pagezvaluemax) {
            invokedItems.insert(siblings[i], siblings[i]->pageZValue());
            continue;
        }
        if (!moveItems.contains(siblings[i])) {
            pagezvalue--;
            siblings[i]->setPageZValue(pagezvalue);
            invokedItems.insert(siblings[i], pagezvalue);
        }
    }
}


void PageScene::addPageItem(PageItem *pItem)
{
    addPageItem(pItem, d_PageScene()->m_currentLayer);
}

void PageScene::addPageItem(PageItem *pItem, LayerItem *toLayer)
{
    if (pItem == nullptr)
        return;

    //场景一样并且不是在这个场景的垃圾桶里面才返回(垃圾桶图元负责进行item的内存回收)
    //如果pItem处于这个场景的垃圾桶里面，那么依然可以向下执行图元所处parent的变化
    if (!pItem->isIdle())
        return;

    bool success = true;
    //1.如果要添加到的图层不为空，那么添加到这个图层中去
    //  如果pItem是一个图层图元，那么加入到场景作为一个顶层图元，否则添加失败
    auto ly = toLayer;
    if (ly != nullptr) {
        if (ly->pageScene() == this) {
            if (pItem->type() == RasterItemType &&  static_cast<RasterItem *>(pItem)->rasterType()
                    == static_cast<RasterItem *>(pItem)->ERasterType::EImageType) { //加入图片图元需要保存命令
                UndoRecorder block(ly, LayerUndoCommand::ChildItemAdded,
                                   pItem);
            }
            ly->addItem(pItem);
        }
    } else {
        if (pItem->type() == LayerItemType) {
            addItem(pItem);
            auto lyItem = static_cast<LayerItem *>(pItem);
            if (!d_PageScene()->topLayers.contains(lyItem)) {
                d_PageScene()->topLayers.insert(-1, lyItem);
                if (d_PageScene()->topLayers.count() == 1) {
                    setCurrentTopLayer(lyItem);
                }
            }
        } else {
            success = false;
            qWarning() << "page item should add to one layer!!";
        }
    }

    //2.通知pageScene变动
    if (success) {
        pItem->d_PageItem()->notifyItemAndChildrenPageSceneChanged(this);
    }
}

void PageScene::removePageItem(PageItem *pItem)
{
    if (pItem == nullptr)
        return;

    if (pItem->pageScene() != this)
        return;

    //1 step. 取消选择
    pItem->setItemSelected(false);

    //2 step 如果删除的是一个顶层layer，那么更新一下toplayer列表
    if (pItem->type() == LayerItemType && pItem->isRootPageItem()) {
        auto layer = static_cast<LayerItem *>(pItem);
        int index = d_PageScene()->topLayers.indexOf(layer);
        d_PageScene()->topLayers.removeAt(index);
        if (layer == d_PageScene()->m_currentLayer) {
            if (d_PageScene()->topLayers.count() > index && index >= 0) {
                setCurrentTopLayer(d_PageScene()->topLayers[index]);
            } else {
                setCurrentTopLayer(nullptr);
            }
        }
    }
    removeItem(pItem);

    //从场景中删除的话，将图元放到垃圾桶里去，避免内存泄漏
    d_PageScene()->trashBin->addChild(pItem);

    //3 step 通知pageScene变动
    pItem->d_PageItem()->notifyItemAndChildrenPageSceneChanged(nullptr);
}

void PageScene::clearAll()
{
    foreach (auto ly, d_PageScene()->topLayers) {
        removeTopLayer(ly);
    }
}

void PageScene::blockAssignZValue(bool b)
{
    d_PageScene()->blockZAssign = b;
}

bool PageScene::isAssignZBlocked() const
{
    return d_PageScene()->blockZAssign;
}

void PageScene::setSelectionsVisible(bool b)
{
    d_PageScene()->selectionItem->setVisible(b);
    d_PageScene()->paintSelectedBorderLine = b;
    PageItem::setPaintSelectingBorder(b);
    this->update();
}

bool PageScene::isSelectionsVisible() const
{
    return d_PageScene()->paintSelectedBorderLine;
}

bool PageScene::isPageItem(QGraphicsItem *pItem)
{
    if (pItem == nullptr)
        return false;

    return (dynamic_cast<PageItem *>(pItem) != nullptr);
}

bool PageScene::isHandleNode(QGraphicsItem *pItem)
{
    if (pItem == nullptr)
        return false;
    //CSizeHandleRect的父类QGraphicsSvgItem的类型就是13
    if (pItem->type() == QGraphicsSvgItem::Type) {
        HandleNode *pHandleItem = dynamic_cast<HandleNode *>(pItem);
        if (pHandleItem != nullptr) {
            return true;
        }
    }

    return false;
}

bool PageScene::isGroupItem(QGraphicsItem *pItem)
{
    if (pItem == nullptr)
        return false;

    return pItem->type() == GroupItemType;
}

PageItem *PageScene::associatedTopItem(QGraphicsItem *pItem)
{
    if (pItem == nullptr)
        return nullptr;

    if (isPageItem(pItem)) {
        return static_cast<PageItem *>(pItem);
    }

    if (isHandleNode(pItem)) {
        HandleNode *pHandle = static_cast<HandleNode *>(pItem);
        auto pageItem = pHandle->parentPageItem();
        if (pageItem->type() == SelectionItemType) {
            pageItem = static_cast<SelectionItem *>(pageItem)->singleSelectedItem();
        }
        return pageItem;
    }
    return nullptr;
}

void PageScene::blockSelect(bool b)
{
    d_PageScene()->blockSel = b;
}

QList<PageItem *> PageScene::selectedPageItems() const
{
    return d_PageScene()->selectionItem->selectedItems();
}

int PageScene::selectedItemCount() const
{
    return d_PageScene()->selectionItem->selectedCount();
}

QRectF PageScene::selectionBoundingRect() const
{
    return d_PageScene()->selectionItem->mapRectToScene(d_PageScene()->selectionItem->itemRect());
}

//PageItem *PageScene::selectionPageItem() const
//{
//    return d_PageScene()->selectionItem;
//}

void PageScene::selectItemsByRect(const QRectF &rect, bool replace)
{
    if (d_PageScene()->blockSel)
        return;

    if (replace)
        clearSelections();

    auto items = allRootPageItems(rect);
    selectPageItem(items);
}

QList<PageItem *> PageScene::filterOutPageItems(const QList<QGraphicsItem *> &items)
{
    QList<QGraphicsItem *> lists = items;

    QList<PageItem *> result;
    for (int i = 0; i < lists.count(); ++i) {
        QGraphicsItem *pItem = lists[i];
        if (isPageItem(pItem)) {
            result.append(static_cast<PageItem *>(pItem));
        }
    }
    return result;
}

QList<PageItem *> PageScene::allPageItems(ESortItemTp tp) const
{
    return filterOutPageItems(returnSortZItems(this->items(tp == EDesSort ? Qt::DescendingOrder : Qt::AscendingOrder), tp));
}

QList<PageItem *> PageScene::allPageItems(const QPointF &pos, ESortItemTp tp) const
{
    return filterOutPageItems(returnSortZItems(this->items(pos, Qt::IntersectsItemShape, tp == EDesSort ? Qt::DescendingOrder : Qt::AscendingOrder), tp));
}

QList<PageItem *> PageScene::allPageItems(const QRectF &rect, ESortItemTp tp) const
{
    return filterOutPageItems(returnSortZItems(this->items(rect, Qt::IntersectsItemShape, tp == EDesSort ? Qt::DescendingOrder : Qt::AscendingOrder), tp));
}

QList<PageItem *> PageScene::allRootPageItems(ESortItemTp tp) const
{
    return filterOutRootPageItems(allPageItems(tp));
}

QList<PageItem *> PageScene::allRootPageItems(const QPointF &pos, ESortItemTp tp) const
{
    return filterOutRootPageItems(allPageItems(pos, tp));
}

QList<PageItem *> PageScene::allRootPageItems(const QRectF &rect, ESortItemTp tp) const
{
    return filterOutRootPageItems(allPageItems(rect, tp));
}

QList<PageItem *> PageScene::filterOutRootPageItems(const QList<PageItem *> &items)
{
    QList<PageItem *> result;
    foreach (auto p, items) {
        if (p->parentPageItem() == nullptr) {
            result.append(p);
        }
    }
    return result;
}

void PageScene::movePageItemsZValue(const QList<PageItem *> &items,
                                    MoveItemZType tp, int step,
                                    bool pushToUndoStack)
{
    QHash<PageItem *, qreal> invokedItem;
    switch (tp) {
    case DownItemZ: {
        moveItemsZDown(items, step, invokedItem);
        break;
    }
    case UpItemZ: {
        moveItemsZUp(items, step, invokedItem);
        break;
    }
    }
    if (!invokedItem.isEmpty() && pushToUndoStack) {
        auto view = invokedItem.begin().key()->pageView();
        if (view != nullptr) {
            UndoStack::clearRecordings();
            for (auto it = invokedItem.begin(); it != invokedItem.end(); ++it) {
                auto p = it.key();
                auto z = it.value();
                UndoStack::recordUndo(UndoKey(p, PageItemObject, ItemUndoCommand::EZValueChanged), PageVariantList() << z);
                UndoStack::recordRedo(UndoKey(p, PageItemObject, ItemUndoCommand::EZValueChanged), PageVariantList() << p->pageZValue());
            }
            UndoStack::finishRecord(view->stack());
        }
    }
}

bool PageScene::isItemsZMovable(const QList<PageItem *> &items, MoveItemZType tp, int step)
{
    bool result = false;
    switch (tp) {
    case DownItemZ: {
        QList<PageItem *> siblings;
        QList<PageItem *> moveItems;
        int temp = 0;
        result = isItemsZDownable(items, step, siblings, moveItems, temp);
        break;
    }
    case UpItemZ: {
        QList<PageItem *> siblings;
        QList<PageItem *> moveItems;
        int temp = 0;
        result =  isItemsZUpable(items, step, siblings, moveItems, temp);
        break;
    }
    }
    return result;
}

qreal PageScene::getMaxZValue()
{
    auto bzItems = allPageItems();
    return bzItems.isEmpty() ? -1 : bzItems.first()->zValue();
}

PageItem *PageScene::topPageItem(const QPointF &pos, bool penalgor, int IncW)
{
    return dynamic_cast<PageItem *>(topItem(pos, QList<QGraphicsItem *>(),
                                            true, penalgor, true, true, true, IncW));
}

QGraphicsItem *PageScene::topItem(const QPointF &pos,
                                  const QList<QGraphicsItem *> &itemsCus,
                                  bool isListDesSorted,
                                  bool penalgor,
                                  bool mustPageItem,
                                  bool seeHandleNodeAsParentPageItem,
                                  bool filterLayer,
                                  int incW)
{
    QList<QGraphicsItem *> items = itemsCus;

    if (itemsCus.isEmpty()) {
        if (incW == 0) {
            items = this->items(pos);
        } else {
            items = this->items(QRectF(pos - QPoint(incW, incW), 2 * QSize(incW, incW)), Qt::IntersectsItemShape);
        }
    }

    //类型黑名单
    QSet<int> blackTypeSet;
    blackTypeSet << PageItem::UserType << LayerItemType;
    if (filterLayer) {
        blackTypeSet << LayerItemType;
    }

//    //排序，保证handlenode在最前面
//    std::stable_sort(items.begin(), items.end(), [ = ](QGraphicsItem * info1, QGraphicsItem * info2) {
//        if (isHandleNode(info1))
//            return true;
//        else if (isHandleNode(info2)) {
//            return false;
//        }
//        return info1->zValue() > info2->zValue();
//    });

    //进行过滤
    for (int i = 0; i < items.count();) {
        QGraphicsItem *pItem = items[i];
        if (blackTypeSet.contains(pItem->type())) {
            items.removeAt(i);
            continue;
        }/* else if (isHandleNode(pItem)) {
            if (seeHandleNodeAsParentPageItem) {
                pItem = associatedTopItem(pItem);
                if (pItem != nullptr)
                    items.replace(i, pItem);
                else {
                    items.removeAt(i);
                    continue;
                }
            }
        }*/
        ++i;
    }


    //如果list没有排好序那么要进行先排序
    if (!isListDesSorted) {
        sortZ(items);
    }

    if (items.isEmpty())
        return nullptr;

    if (penalgor) {
        QGraphicsItem *pResultItem = nullptr;
        QList<PageItem *> allPenetrable;
        for (int i = 0; i < items.count(); ++i) {
            QGraphicsItem *pItem = items[i];
            if (isPageItem(pItem)) {
                PageItem *pBzItem = dynamic_cast<PageItem *>(pItem);
                bool isPenetrable = pBzItem->isPosPenetrable(pBzItem->mapFromScene(pos));
                if (incW != 0) {
                    QRectF rect(pBzItem->mapFromScene(pos) - QPoint(incW, incW), 2 * QSize(incW, incW));
                    isPenetrable = pBzItem->isRectPenetrable(rect);
                }
                if (!isPenetrable) {
                    //在该位置不透明,判定完成
                    pResultItem = pBzItem;
                    break;
                } else {
                    allPenetrable.append(pBzItem);
                }
            }/*else if(isHandleNode(pItem)){
                if (seeHandleNodeAsParentPageItem) {
                    pItem = associatedTopItem(pItem);
                    if (pItem != nullptr)
                        items.replace(i, pItem);
                    else {
                        items.removeAt(i);
                        continue;
                    }
                }
            } */
            else {
                if (pItem->flags() & QGraphicsItem::ItemHasNoContents) {
                    continue;
                }
                //非业务图元无认识不透明的 ，那么就证明找到了，判定完成
                pResultItem = pItem;

                //将非业务图元的节点图元看作业务图元时的情况
                if (isHandleNode(pItem)) {
                    if (seeHandleNodeAsParentPageItem) {
//                        HandleNode *pHandelItem = dynamic_cast<HandleNode *>(pItem);
//                        PageItem *pBzItem = dynamic_cast<PageItem *>(pHandelItem->parentItem());
//                        pResultItem = pBzItem;
                        pResultItem = associatedTopItem(pItem);
                    }
                }
                //必须返回业务图元的情况
                if (mustPageItem && !isPageItem(pResultItem)) {
                    pResultItem = nullptr;
                }
                break;
            }
        }
        if (pResultItem == nullptr) {
            if (!allPenetrable.isEmpty()) {
                QRectF ins = allPenetrable.first()->mapToScene(allPenetrable.first()->itemRect()).boundingRect();
                for (auto it : allPenetrable) {
                    QRectF rect = it->mapToScene(it->itemRect()).boundingRect();
                    ins = ins.intersected(rect);
                }
                qreal maxRadio = 0;
                for (auto it : allPenetrable) {
                    QRectF rect = it->mapToScene(it->itemRect()).boundingRect();
                    qreal radio = (ins.width() * ins.height()) / (rect.width() * rect.height());
                    if (radio > maxRadio) {
                        maxRadio = radio;
                        pResultItem = it;
                    }
                }
            }
        }
        return pResultItem;
    }
    QGraphicsItem *pRetItem = nullptr;
    if (mustPageItem) {
        foreach (auto item, items) {
            if (isPageItem(item)) {
                pRetItem = item;
                break;
            } else {
                if (seeHandleNodeAsParentPageItem && isHandleNode(item)) {
                    pRetItem = associatedTopItem(item);
                    break;
                }
            }
        }
    } else {
        pRetItem = items.isEmpty() ? nullptr : items.first();
    }
    return pRetItem;
}

// Cppcheck检测函数没有使用到
//QList<CGraphicsItem *> CDrawScene::findBzItems(const QPointF &pos,
//                                               bool seeNodeAsBzItem,
//                                               bool filterGroup,
//                                               int incW)
//{
//    QList<QGraphicsItem *> items;
//    if (incW == 0) {
//        items = this->items(pos);
//    } else {
//        items = this->items(QRectF(pos - QPoint(incW, incW), 2 * QSize(incW, incW)), Qt::IntersectsItemShape);
//    }
//    QList<CGraphicsItem *> resultItems;
//    for (auto p : items) {
//        if (isDrawItem(p)) {
//            if (filterGroup) {
//                if (!isNormalGroupItem(p)) {
//                    resultItems.append(static_cast<CGraphicsItem *>(p));
//                }
//            } else {
//                resultItems.append(static_cast<CGraphicsItem *>(p));
//            }
//        } else {
//            if (seeNodeAsBzItem && isBussizeHandleNodeItem(p)) {
//                auto pItem = getAssociatedBzItem(p);
//                if (filterGroup) {
//                    if (!isNormalGroupItem(pItem)) {
//                        resultItems.append(pItem);
//                    }
//                } else {
//                    resultItems.append(pItem);
//                }
//            }
//        }
//    }
//    return resultItems;
//}

void PageScene::blockScene(bool b)
{
    d_PageScene()->blocked = b;
}

bool PageScene::isSceneBlocked() const
{
    return d_PageScene()->blocked;
}

//void PageScene::recordUndo(LayerItem *layer, int exptype, const QList<PageItem *> &items)
//{
//    PageVariantList vars;

//    LayerUndoCommand::ChangedType tp = LayerUndoCommand::ChangedType(exptype);

//    //当前仅支持EGroupChanged的场景快照还原
//    switch (tp) {
//    case LayerUndoCommand::ChildGroupAdded: {
//        break;
//    }
//    default:
//        vars << items;
//        return;
//    }

//    UndoStack::recordUndo(UndoKey(layer, PageLayerObject, exptype),
//                          vars, true);
//}

//void PageScene::recordRedo(LayerItem *layer, int exptype, const QList<PageItem *> &items)
//{
//    PageVariantList vars;

//    LayerUndoCommand::ChangedType tp = LayerUndoCommand::ChangedType(exptype);

//    //当前仅支持EGroupChanged的场景快照还原
//    switch (tp) {
//    case LayerUndoCommand::ChildGroupAdded: {
//        break;
//    }
//    default:
//        vars << items;
//        return;
//    }

//    UndoStack::recordRedo(UndoKey(layer, PageLayerObject, exptype),
//                          vars);
//}

//void PageScene::recordUndo(PageItem *pItem, bool clearInfo)
//{
//    PageVariantList vars;
//    vars << pItem->getItemUnit(UnitUsage_Undo);
//    UndoStack::recordUndo(UndoKey(pItem, PageItemObject,
//                                  ItemUndoCommand::EUnitChanged), vars, clearInfo);
//}

//void PageScene::recordRedo(PageItem *pItem)
//{
//    PageVariantList vars;
//    vars << pItem->getItemUnit(UnitUsage_Undo);
//    UndoStack::recordRedo(UndoKey(pItem, PageItemObject,
//                                  ItemUndoCommand::EUnitChanged), vars);
//}

//void PageScene::recordUndo(const QList<PageItem *> &items, bool clearInfo)
//{
//    for (int i = 0; i < items.size(); ++i) {
//        PageItem *pItem = items[i];
//        recordUndo(pItem, clearInfo && (i == 0));
//    }
//}

//void PageScene::recordRedo(const QList<PageItem *> &items)
//{
//    for (int i = 0; i < items.size(); ++i) {
//        PageItem *pItem = items[i];
//        recordRedo(pItem);
//    }
//}

//void PageScene::finishRecord(UndoStack *stack, bool doRedoCmd)
//{
//    UndoStack::finishRecord(stack, doRedoCmd);
//}

bool PageScene::isGroupable(const QList<PageItem *> &pageItems) const
{
    //组合个数必须是大于1的
    if (pageItems.count() <= 1)
        return false;

    //必须是顶层图元
    foreach (auto p, pageItems) {
        if (p->parentPageItem() != nullptr && !p->parentPageItem()->isLayer())
            return false;
    }

    return true;
}

bool PageScene::isUnGroupable(const QList<PageItem *> &pBzItems) const
{
    bool bIsUngroup = false;

    foreach (auto pItem, pBzItems) {
        pItem = pItem->pageProxyItem(true);
        if (isGroupItem(pItem)) {
            bIsUngroup = true;
            break;
        }
    }

    return bIsUngroup;
}

GroupItem *PageScene::getCommonGroup(const QList<PageItem *> &pBzItems, bool top)
{
    if (pBzItems.isEmpty())
        return nullptr;

    GroupItem *pGroup = top ? pBzItems.first()->topPageGroup() : pBzItems.first()->pageGroup();
    for (int i = 1; i < pBzItems.size(); ++i) {
        PageItem *p = pBzItems[i];
        GroupItem *pTopGroupTemp = top ? p->topPageGroup() : p->pageGroup();
        if (pGroup != pTopGroupTemp) {
            pGroup = nullptr;
            break;
        }
    }
    return pGroup;
}

bool PageScene::isPageItemsSibling(const QList<PageItem *> &PageItems)
{
    if (PageItems.isEmpty()) {
        return false;
    }
    auto parent = PageItems.first()->parentPageItem();
    foreach (auto p, PageItems) {
        if (p->parentPageItem() != parent)
            return false;
    }
    return true;
}

PageItem *PageScene::samePageItemParent(const QList<PageItem *> &PageItems)
{
    if (PageItems.isEmpty()) {
        return nullptr;
    }
    auto parent = PageItems.first()->parentPageItem();
    foreach (auto p, PageItems) {
        if (p->parentPageItem() != parent)
            return nullptr;
    }
    return parent;
}

GroupItem *PageScene::creatGroup(const QList<PageItem *> &pageItems,
                                 PageItem *base)
{
    if (!isGroupable(pageItems)) {
        return nullptr;
    }

    if (pageItems.isEmpty())
        return nullptr;


    if (!pageItems.contains(base)) {
        //获取最小z值item
        qreal minvzvalue = pageItems[0]->pageZValue();
        base = pageItems.first();
        for (int i = 1; i < pageItems.size(); ++i) {
            if (minvzvalue > pageItems[i]->pageZValue()) {
                minvzvalue = pageItems[i]->pageZValue();
                base = pageItems[i];
            }
        }
    }

    auto group = new GroupItem;
    addPageItem(group);

    group->addToGroup(pageItems);

    group->setPageZValue(base->pageZValue());

    return group;
}

//GroupItem *PageScene::copyCreatGroup(GroupItem *pGroup)
//{
//    if (pGroup == nullptr || pGroup->pageScene() == nullptr)
//        return nullptr;

//    PageScene *pScene = pGroup->pageScene();
//    UnitTree_Comp itemsTreeInfo = pScene->getGroupTreeInfo(pGroup);
//    GroupItem *pNewGroup = pScene->loadGroupTreeInfo(itemsTreeInfo, true);
//    return pNewGroup;
//}

void PageScene::cancelGroup(GroupItem *pGroup)
{
    if (pGroup == nullptr)
        return;

    auto lists = pGroup->childPageItems();

    qreal basezvalue = pGroup->pageZValue();

    pGroup->clear();

    if (pGroup->pageScene() != nullptr) {
        pGroup->pageScene()->removePageItem(pGroup);
    }
    //解除组合设置图层
    for (int i = 0; i < lists.size(); ++i) {
        lists[i]->setZValue(basezvalue + i);
    }

    auto allpagelists = allPageItems(EAesSort);
    qreal pagevalue = lists.last()->pageZValue();
    for (int i = 0; i < allpagelists.size(); ++i) {
        //过滤其他图元，保留业务图元
        if (allpagelists[i]->type() <= NoType || allpagelists[i]->type() > RasterItemType)
            continue;
        //过滤垃圾桶里面的图元
        if (allpagelists[i]->parentItem() == d_PageScene()->trashBin)
            continue;
        if (allpagelists[i]->pageZValue() == pagevalue && !lists.contains(allpagelists[i])) {
            pagevalue++;
            allpagelists[i]->setZValue(pagevalue);
        }
    }
    selectPageItem(lists);
}

QColor PageScene::systemThemeColor() const
{
    QPalette pa = this->palette();
    QBrush selectBrush = pa.brush(QPalette::Active, QPalette::Highlight);
    return selectBrush.color();
}

QImage PageScene::renderToImage(const QColor &bgColor, const QSize &desImageSize)
{
    QImage image;

    auto scene = this;

    //render前屏蔽掉多选框和选中的边线显示(之后恢复)
    this->setSelectionsVisible(false);

    auto cachedColor = this->bgColor();
    auto cachedBrush = this->backgroundBrush();

    this->setBgColor(bgColor);
    this->setBackgroundBrush(Qt::transparent);

    image = QImage(desImageSize.isValid() ? desImageSize : scene->sceneRect().size().toSize(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    this->render(&painter, image.rect(), scene->sceneRect(), Qt::IgnoreAspectRatio);

    //render后恢复屏蔽掉的多选框和选中的边线显示
    this->setSelectionsVisible(true);

    this->setBgColor(cachedColor);
    this->setBackgroundBrush(cachedBrush);

    return image;
}

void PageScene::loadData(const PageVariant &ut)
{
    Q_UNUSED(ut)
}

PageVariant PageScene::getData(int use) const
{
    Q_UNUSED(use)
    return PageVariant();
}

void PageScene::rotateSelectItems(qreal angle)
{
    UndoStack::recordUndo(selectedPageItems());

    PageItemRotEvent event(angle - d_PageScene()->selectionItem->drawRotation(), d_PageScene()->selectionItem->itemRect().center());
    d_PageScene()->selectionItem->setBlockUpdateRect(true);
    d_PageScene()->selectionItem->operating(&event);
    d_PageScene()->selectionItem->setBlockUpdateRect(false);

    UndoStack::finishRecord(page()->view()->stack());
}

QRectF PageScene::currentTopLayerSceneRect()
{
    QRectF rt;
    if (nullptr != d_PageScene()->m_currentLayer) {
        rt = d_PageScene()->m_currentLayer->sceneBoundingRect();
    }
    return rt;
}
