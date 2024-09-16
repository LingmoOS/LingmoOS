// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "selecttool.h"
#include "pagescene.h"
#include "pagecontext.h"
#include "pageview.h"
#include "pageitem.h"
#include "textitem.h"
#include "cundoredocommand.h"
#include "cgraphicsitemevent.h"
#include "layeritem.h"
#include "attributemanager.h"

#include <QScrollBar>
#include <QDebug>
#include <QCursor>
#include <QtMath>
#include <QPainterPath>
#include <QTextCursor>
#include <QtMath>
#include <QToolButton>
#include <QGraphicsSceneContextMenuEvent>

extern PageItem *_pageSceneSelectItem(PageScene *scene);

class SelectTool::SelectTool_private
{
public:
    explicit SelectTool_private(SelectTool *qq): q(qq) {}

    // 通过框选工具触发点角度变更信号，用于更新外部的属性栏等控件
    void internalAngleUpdate();

    SelectTool *q;

    /* 当前有item正在被拖拽移动 */
    bool m_isItemMoving = false;

    /* 高亮路径 */
    QPainterPath _hightLight;


    int _extraTp = -1;

    PageItem *_extraItem = nullptr;


    HandleNode *handleNode = nullptr;

    QPointF _selectionFirPos;
    QPointF _selectionSecPos;

    QMap<int, QRectF>  _selections;
    QMap<int, QPointF> _activeRotIdEvents;
};

/**
 * @brief 当使用框选工具更新图元角度时，通过信号更新至其它界面控件
 */
void SelectTool::SelectTool_private::internalAngleUpdate()
{
    // 获取当前勾选图元的实际角度
    PageScene *scene = q->drawBoard()->currentPage()->context()->scene();
    auto selectedItems = scene->selectedPageItems();
    qreal rote = selectedItems.count() == 0 ? 0 : selectedItems.first()->drawRotation();
    // 向外广播属性变更信号
    emit q->drawBoard()->attributionManager()->helper()->internalAttibutionUpdate(ERotProperty, rote, EChangedUpdate);
}


SelectTool::SelectTool(QObject *parent)
    : DrawFunctionTool(parent), SelectTool_d(new SelectTool_private(this))
{
    setClearSelectionOnActived(false);
    //setAttributionShowOnActived(false);

    setCursor(QCursor(Qt::ArrowCursor));

    auto m_selectBtn = toolButton();
    m_selectBtn->setShortcut(QKeySequence(QKeySequence(Qt::Key_V)));
    setWgtAccesibleName(m_selectBtn, "Select tool button");
    m_selectBtn->setToolTip(tr("Select (V)"));
    m_selectBtn->setIconSize(TOOL_ICON_RECT);
    m_selectBtn->setFixedSize(TOOL_BUTTON_RECT);
    m_selectBtn->setCheckable(true);
    m_selectBtn->setIcon(QIcon::fromTheme("arrow_normal"));
    connect(m_selectBtn, &QToolButton::toggled, m_selectBtn, [ = ](bool b) {
        QIcon icon       = QIcon::fromTheme("arrow_normal");
        QIcon activeIcon = QIcon::fromTheme("arrow_highlight");
        m_selectBtn->setIcon(b ? activeIcon : icon);
    });
}

SelectTool::~SelectTool()
{

}

int SelectTool::toolType() const
{
    return selection;
}

SAttrisList SelectTool::attributions()
{
//    SAttrisList result;
//    return result;
    if (currentPage() != nullptr) {
        return currentPage()->currentAttris();
    }
    return SAttrisList();
}

void SelectTool::setAttributionVar(int attri, const QVariant &var, int phase, bool autoCmdStack)
{
    if (drawBoard()->currentPage() != nullptr)
        drawBoard()->currentPage()->setAttributionVar(attri, var, phase, autoCmdStack);
}

void SelectTool::funcStart(ToolSceneEvent *event)
{
    if (!event->isNormalPressed())
        return;
    d_SelectTool()->_hightLight = QPainterPath();

    d_SelectTool()->_selections.insert(event->uuid(), QRectF(event->pos(), event->pos()));

    auto itemUnderPos = event->topPageItemUnderPressedPos();

    if (itemUnderPos == nullptr) {
        auto graphicsItem = event->scene()->topItem(event->pos());
        if (!PageScene::isHandleNode(graphicsItem))
            event->scene()->clearSelections();
    } else {
        if (event->keyboardModifiers() == Qt::ShiftModifier) {
            if (event->scene()->selectedItemCount() > 1) {
                itemUnderPos->setItemSelected(!itemUnderPos->isItemSelected());
            } else {
                itemUnderPos->setItemSelected(true);
            }
        } else {
            if (itemUnderPos->isItemSelected()) {

            } else {
                event->scene()->clearSelections();
                itemUnderPos->setItemSelected(true);
            }
        }
    }
}

int SelectTool::funcDecide(ToolSceneEvent *event)
{
    int type = ENothingDo;
    if (isWorking()) {
        auto graphicsItem = event->firstEvent()->topItemUnderPressedPos();/*event->scene()->topItem(event->pos(), event->firstEvent()->itemsUnderPos(), true, true)*/;
        if (graphicsItem == nullptr) {
            if (event->eventType() == ToolSceneEvent::ETouchEvent) {
                if (elapsedFromBeginToDecide(event->uuid()) > 200) {
                    type = ERectSelect;
                } else {
                    type = EDragSceneMove;
                }
            } else {
                type = ERectSelect;
            }
        } else {
            if (PageScene::isPageItem(graphicsItem)) {
                d_SelectTool()->_extraItem = _pageSceneSelectItem(event->scene());
                if (event->keyboardModifiers() == Qt::AltModifier) {
                    type = ECopyMove;
                    auto waitCopyItems = returnSortZItems(event->scene()->selectedPageItems(), EAesSort);
                    QList<PageItem *> copyedItems;
                    foreach (auto p, waitCopyItems) {
                        auto cpItem = PageItem::creatItemInstance(p->type(), p->getItemUnit(UnitUsage_Copy));
                        event->scene()->addPageItem(cpItem);
                        copyedItems.append(cpItem);
                    }
                    event->scene()->clearSelections();
                    event->scene()->selectPageItem(copyedItems);
                } else {
                    type = EDragMove;
                    UndoStack::recordUndo(event->scene()->selectedPageItems());
                }
            } else if (PageScene::isHandleNode(graphicsItem)) {

                HandleNode *pHandle = dynamic_cast<HandleNode *>(graphicsItem);

                d_SelectTool()->_extraItem = pHandle->parentPageItem();

                //记录undo点
                if (event->view()->activeProxWidget() == nullptr)
                    UndoStack::recordUndo(event->scene()->selectedPageItems());

                d_SelectTool()->handleNode = pHandle;

                d_SelectTool()->handleNode->pressBegin(toolType(), event);

                type = (pHandle->nodeType() != HandleNode::Rotation ? EResizeMove : ERotateMove);
                if (type == ERotateMove) {
                    d_SelectTool()->_activeRotIdEvents.insert(event->uuid(), event->pos());
                }
            }
        }
    }

    if (type > ENothingDo) {
        //event->scene()->update();
        sendToolEventToItem(event, EChangedBegin, type);
    }
    return type;
}

void SelectTool::funcUpdate(ToolSceneEvent *event, int decidedTp)
{
    event->setAccepted(false);

    //根据要做的类型去执行相应的操作
    switch (decidedTp) {
    case ERectSelect: {
        QPointF pos0 = event->firstEvent()->pos();
        QPointF pos1 = event->pos();

        QPointF topLeft(qMin(pos0.x(), pos1.x()), qMin(pos0.y(), pos1.y()));
        QPointF bomRight(qMax(pos0.x(), pos1.x()), qMax(pos0.y(), pos1.y()));
        d_SelectTool()->_selections[event->uuid()] = QRectF(topLeft, bomRight);
        event->scene()->update();
        break;
    }
    case EDragSceneMove: {
        //拖拽画布移动
        QPointF pos0 = event->firstEvent()->pos();
        QPointF pos1 = event->pos();
        QPointF mov = pos1 - pos0;

        QScrollBar *horBar = event->view()->horizontalScrollBar();
        int horValue = horBar->value() - qRound(mov.x());
        horBar->setValue(qMin(qMax(horBar->minimum(), horValue), horBar->maximum()));

        QScrollBar *verBar = event->view()->verticalScrollBar();
        int verValue = verBar->value() - qRound(mov.y());
        verBar->setValue(qMin(qMax(verBar->minimum(), verValue), verBar->maximum()));

        break;
    }
    case EDragMove:
    case ECopyMove: {
        //执行移动操作
        processItemsMove(event, EChangedUpdate);
        break;
    }
    case EResizeMove: {
        //交给图元去完成
        d_SelectTool()->handleNode->pressMove(toolType(), event);
        break;
    }
    case ERotateMove: {
        d_SelectTool()->handleNode->pressMove(toolType(), event);
        d_SelectTool()->_activeRotIdEvents.insert(event->uuid(), event->pos());

        // 当使用框选工具更新图元角度时，通过信号更新至其它界面控件
        d_SelectTool()->internalAngleUpdate();
        break;
    }
    default:
        break;
    }
}

void SelectTool::funHover(ToolSceneEvent *event)
{
    //处理高亮，鼠标样式变化等问题
    processHightLight(event);

    processCursor(event);
}

void SelectTool::funcFinished(ToolSceneEvent *event, int decidedTp)
{
    if (decidedTp > ENothingDo) {
        sendToolEventToItem(event, EChangedFinished, decidedTp);
    }
    bool doUndoFinish = true;
    switch (decidedTp) {
    case ERectSelect: {
        //event->scene()->selectItemsByRect(_selections[event->uuid()]);

        auto currentLayer = event->scene()->currentTopLayer();

        //qWarning() << "rect ======= " << d_SelectTool()->_selections[event->uuid()] << currentLayer->mapRectFromScene(d_SelectTool()->_selections[event->uuid()])
        //        << currentLayer->itemRect() << currentLayer->items().count();
        auto items = currentLayer->pageItems(currentLayer->mapRectFromScene(d_SelectTool()->_selections[event->uuid()]), EAesSort);

        event->scene()->selectPageItem(items);

        d_SelectTool()->_selections.remove(event->uuid());

        event->scene()->update();

        doUndoFinish = false;
        break;
    }
    case EDragMove: {
        UndoStack::recordRedo(event->scene()->selectedPageItems());
        d_SelectTool()->m_isItemMoving = false;
        break;
    }
    case EResizeMove: {
        d_SelectTool()->handleNode->pressRelease(toolType(), event);
        //记录Redo点
        doUndoFinish = (event->view()->activeProxWidget() == nullptr);
        if (doUndoFinish)
            UndoStack::recordRedo(event->scene()->selectedPageItems());

        break;
    }
    case ECopyMove: {
        UndoRecorder recordUndo(event->scene()->currentTopLayer(), LayerUndoCommand::ChildItemAdded,
                                event->scene()->selectedPageItems());
        d_SelectTool()->m_isItemMoving = false;
        break;
    }
    case ERotateMove: {
        d_SelectTool()->handleNode->pressRelease(toolType(), event);
        doUndoFinish = (event->view()->activeProxWidget() == nullptr);
        if (doUndoFinish)
            UndoStack::recordRedo(event->scene()->selectedPageItems());
        d_SelectTool()->_activeRotIdEvents.remove(event->uuid());
        break;
    }
    default:
        return;
    }

    //入栈
    if (doUndoFinish)
        UndoStack::finishRecord(event->view()->stack());


    d_SelectTool()->handleNode = nullptr;

    funHover(event);
}

void SelectTool::doubleClickOnScene(ToolSceneEvent *event)
{
    DrawFunctionTool::doubleClickOnScene(event);

    //修复双击形成多选
    event->scene()->clearSelections();

    PageItem *pItem = event->topPageItemUnderPressedPos();
    if (pItem != nullptr) {
        //qWarning() << "pIme type ===== " << pItem->type();
        if (pItem->type() == TextType) {
            TextItem *pTextItem = dynamic_cast<TextItem *>(pItem);
            //如果是非编辑状态那么应该进入编辑状态
            if (!pTextItem->isEditing()) {
                pTextItem->setEditing(true, true);
                funHover(event);
            }
        }
    }
    event->setAccepted(true);
}

void SelectTool::contextMenuEvent(ToolSceneEvent *event)
{
    PageItem *item = _pageSceneSelectItem(event->scene());
    QGraphicsSceneContextMenuEvent *contextEvent = static_cast<QGraphicsSceneContextMenuEvent *>(event->orgQtEvent());
    contextEvent->setPos(item->mapFromScene(event->pos()));
    event->scene()->sendEvent(item, contextEvent);
}

void SelectTool::leaveSceneEvent(ToolSceneEvent *event)
{
    //qWarning() << "SelectTool::leaveEvent -------------- ";
    d_SelectTool()->_hightLight = QPainterPath();
    DrawFunctionTool::leaveSceneEvent(event);
    event->scene()->update();
}

void SelectTool::drawMore(QPainter *painter,
                          const QRectF &rect,
                          PageScene *scene)
{
    //注意painter是在scene的坐标系

    Q_UNUSED(rect)
    painter->save();

    if (status() != EWorking) {
        painter->setBrush(Qt::NoBrush);
        QColor selectColor = scene->systemThemeColor();
        QPen p(selectColor);
        p.setWidthF(2.0);
        painter->setPen(p);
        if (!d_SelectTool()->_hightLight.isEmpty()) {
            painter->drawPath(d_SelectTool()->_hightLight);
        }
    } else {
        foreach (auto rect, d_SelectTool()->_selections) {

            QPen pen;
            pen.setWidthF(0.5);
#ifdef USE_DTK
            QBrush selectBrush = scene->systemThemeColor();
#else
            QBrush selectBrush = QColor(0, 0, 255);
#endif
            QColor selectColor = selectBrush.color();
            selectColor.setAlpha(20);
            selectBrush.setColor(selectColor);
            selectColor.setAlpha(100);
            pen.setColor(selectColor);

            painter->setPen(pen);
            painter->setBrush(selectBrush);
            painter->drawRect(rect);

        }
        foreach (auto rot, d_SelectTool()->_activeRotIdEvents) {
            //功能：将旋转角度绘制到视口上

            painter->save();
            painter->setClipping(false);

            //重置所有变换，从而保证绘制时是视口的坐标系
            painter->resetTransform();

            QPoint  posInView  = scene->firstPageView()->mapFromScene(rot);

            QPointF paintPos = posInView + QPointF(50, 0);

            auto selectedItems = scene->selectedPageItems();
            qreal rote = selectedItems.count() == 0 ? 0 : selectedItems.first()->drawRotation();
            QString angle = QString("%1°").arg(QString::number(rote, 'f', 1));
            QFont f;
            f.setPixelSize(14);

            QFontMetrics fontMetrics(f);
            int width = fontMetrics.width(angle) + 6;
            QRectF rotateRect(paintPos, paintPos + QPointF(width, fontMetrics.height()));

            painter->setPen(Qt::NoPen);
            //#E5E5E5
            painter->setBrush(QColor(245, 245, 245));
            painter->drawRoundRect(rotateRect);
            painter->setFont(f);
            painter->setPen(Qt::black);
            painter->drawText(rotateRect, Qt::AlignCenter, angle);
            painter->restore();
        }
    }

    painter->restore();
}

void SelectTool::onStatusChanged(EStatus oldStatus, EStatus nowStatus)
{
    Q_UNUSED(oldStatus)
    if (nowStatus == EIdle) {
        if (drawBoard()->currentPage() != nullptr) {
            auto w = drawBoard()->currentPage()->view()->activeProxWidget();
            if (w != nullptr) {
                w->clearFocus();
            }
        }
    }
}

//bool SelectTool::returnToSelectTool(ToolSceneEvent *event)
//{
//    Q_UNUSED(event)
//    return false;
//}

void SelectTool::sendToolEventToItem(ToolSceneEvent *event,

                                     EChangedPhase phase, int type)
{
    PageItemEvent::EItemType tp = PageItemEvent::EUnKnow;
    switch (type) {
    case EDragMove:
    case ECopyMove:
        tp = PageItemEvent::EMove;
        break;
    case EResizeMove:
        tp = PageItemEvent::EScal;
        break;
    case ERotateMove:
        tp = PageItemEvent::ERot;
        break;
    default:
        tp = PageItemEvent::EUnKnow;
        break;
    }

    switch (tp) {
    case PageItemEvent::EMove: {
        processItemsMove(event, phase);
        break;
    }
    case PageItemEvent::EScal: {
        processItemsScal(event, phase);
        break;
    }
    case PageItemEvent::ERot: {
        processItemsRot(event, phase);
        break;
    }
    default:
        break;
    }
}

void SelectTool::processItemsScal(ToolSceneEvent *event, EChangedPhase phase)
{
    PageItemScalEvent scal(PageItemEvent::EScal);
    scal.setEventPhase(phase);
    scal.setToolEventType(decideValue(event->uuid()));
    scal.setPressedDirection(d_SelectTool()->_extraTp);
    scal._scenePos = event->pos();
    scal._oldScenePos = event->lastEvent()->pos();
    scal._sceneBeginPos = event->firstEvent()->pos();

    bool xBlock = false;
    bool yBlock = false;
    HandleNode::EInnerType dir = HandleNode::EInnerType(d_SelectTool()->_extraTp);
    HandleNode::getTransBlockFlag(dir, xBlock, yBlock);
    scal.setXTransBlocked(xBlock);
    scal.setYTransBlocked(yBlock);

    bool xNegitiveOffset = false;
    bool yNegitiveOffset = false;
    HandleNode::getTransNegtiveFlag(dir, xNegitiveOffset, yNegitiveOffset);
    scal.setXNegtiveOffset(xNegitiveOffset);
    scal.setYNegtiveOffset(yNegitiveOffset);
    scal.setKeepOrgRadio(event->keyboardModifiers() & Qt::ShiftModifier);

    //分发事件
    if (PageScene::isPageItem(d_SelectTool()->_extraItem) /*|| _extraItem == event->scene()->selectGroup()*/) {
        PageItem *pBzItem = d_SelectTool()->_extraItem;

        scal.setPos(pBzItem->mapFromScene(event->pos()));
        scal.setOldPos(pBzItem->mapFromScene(event->lastEvent()->pos()));
        scal.setOrgSize(pBzItem->orgRect().size());
        scal.setCenterPos((event->keyboardModifiers() & Qt::AltModifier) ? pBzItem->orgRect().center() :
                          HandleNode::transCenter(dir, pBzItem));
        scal._sceneCenterPos = pBzItem->mapToScene(scal.centerPos());

        if (phase == EChangedBegin) {
            pBzItem->operatingBegin(&scal);
        } else if (phase == EChangedUpdate) {
            pBzItem->operating(&scal);
        } else if (phase == EChangedFinished) {
            pBzItem->operatingEnd(&scal);
        }
    }
    event->view()->viewport()->update();
}

void SelectTool::processItemsRot(ToolSceneEvent *event, EChangedPhase phase)
{
    if (d_SelectTool()->_extraItem == nullptr)
        return;

    PageItemRotEvent rot(PageItemEvent::ERot);
    rot.setEventPhase(phase);
    rot.setToolEventType(decideValue(event->uuid()));
    rot.setPressedDirection(d_SelectTool()->_extraTp);
    rot._scenePos = event->pos();
    rot._oldScenePos = event->lastEvent()->pos();
    rot._sceneBeginPos = event->firstEvent()->pos();

    //分发事件
    if (PageScene::isPageItem(d_SelectTool()->_extraItem) /*|| _extraItem == event->scene()->selectGroup()*/) {
        PageItem *pBzItem = d_SelectTool()->_extraItem;
        rot.setPos(pBzItem->mapFromScene(event->pos()));
        rot.setOldPos(pBzItem->mapFromScene(event->lastEvent()->pos()));
        rot.setOrgSize(pBzItem->orgRect().size());
        rot.setCenterPos(pBzItem->orgRect().center());
        rot._sceneCenterPos = pBzItem->mapToScene(rot.centerPos());
        if (phase == EChangedBegin) {
            pBzItem->operatingBegin(&rot);
        } else if (phase == EChangedUpdate) {
            pBzItem->operating(&rot);
        } else if (phase == EChangedFinished) {
            pBzItem->operatingEnd(&rot);
        }
    }
    event->view()->viewport()->update();
}

void SelectTool::processHightLight(ToolSceneEvent *event)
{
    auto hlPath = QPainterPath();

    if (event->view()->activeProxWidget() == nullptr) {
//        QGraphicsItem *pItem = event->scene()->topItem(event->pos(), QList<QGraphicsItem *>(),
//                                                       true, true, true, true);

        auto pItem = event->scene()->topPageItem(event->pos());
        if (pItem != nullptr) {
            PageItem *pBzItem = static_cast<PageItem *>(pItem);
            hlPath = pBzItem->mapToScene(pBzItem->highLightPath());
        }
    }
    if (hlPath != d_SelectTool()->_hightLight) {
        d_SelectTool()->_hightLight = hlPath;
        event->scene()->update();
    }
}

void SelectTool::processCursor(ToolSceneEvent *event)
{
//    static int i = 0;
//    qWarning() << __func__ << ++i;
    QPointF scenePos = event->pos();
    QList<QGraphicsItem *> items = event->scene()->items(scenePos);
    QGraphicsItem *pItem = event->scene()->topItem(scenePos, items, true, true, false, false, false);
    if (pItem != nullptr) {
        auto cusor = pItem->cursor();
        if (PageScene::isHandleNode(pItem)) {
            cusor = static_cast<HandleNode *>(pItem)->cursor();
        }
        event->scene()->setCursor(cusor);
    } else {
        event->scene()->setCursor(cursor());
    }
}

void SelectTool::processItemsMove(ToolSceneEvent *event,
                                  EChangedPhase phase)
{
    PageItemMoveEvent mov(PageItemEvent::EMove);
    mov.setEventPhase(phase);
    mov.setToolEventType(decideValue(event->uuid()));
    mov.setPressedDirection(d_SelectTool()->_extraTp);
    mov._scenePos = event->pos();
    mov._oldScenePos = event->lastEvent()->pos();
    mov._sceneBeginPos = event->firstEvent()->pos();

    if (PageScene::isPageItem(d_SelectTool()->_extraItem) /*|| _extraItem == event->scene()->selectGroup()*/) {
        PageItem *pBzItem = d_SelectTool()->_extraItem;
        mov.setPos(pBzItem->mapFromScene(event->pos()));
        mov.setOldPos(pBzItem->mapFromScene(event->lastEvent()->pos()));
        mov.setOrgSize(pBzItem->orgRect().size());
        mov.setCenterPos(pBzItem->orgRect().center());
        mov._sceneCenterPos = pBzItem->mapToScene(mov.centerPos());
        if (phase == EChangedBegin) {
            pBzItem->operatingBegin(&mov);
        } else if (phase == EChangedUpdate) {
            pBzItem->operating(&mov);
        } else if (phase == EChangedFinished) {
            pBzItem->operatingEnd(&mov);
        }
    }
}
