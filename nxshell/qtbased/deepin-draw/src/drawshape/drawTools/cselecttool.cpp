// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cselecttool.h"
#include "cdrawscene.h"
#include "cgraphicsitem.h"
#include "cgraphicslineitem.h"
#include "cdrawparamsigleton.h"
#include "cgraphicstextitem.h"
#include "cgraphicsproxywidget.h"
#include "cgraphicscutitem.h"
#include "cgraphicsellipseitem.h"
#include "cgraphicstriangleitem.h"
#include "cgraphicspolygonitem.h"
#include "cgraphicspolygonalstaritem.h"
#include "widgets/ctextedit.h"
#include "bzItems/cpictureitem.h"
#include "cgraphicsitemselectedmgr.h"
#include "frame/cviewmanagement.h"
#include "frame/cgraphicsview.h"
#include "application.h"
#include "frame/cundoredocommand.h"
#include "cgraphicsitemevent.h"

#include <DApplication>
#include <QScrollBar>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QCursor>
#include <QtMath>
#include <QGraphicsTextItem>
#include <QPainterPath>
#include <QTextCursor>
#include <QPixmap>
#include <QGraphicsView>
#include <QTextEdit>
#include <QSvgGenerator>
#include <QtMath>
#include <DToolButton>

#include "cattributeitemwidget.h"
#include "cattributemanagerwgt.h"
using namespace DrawAttribution;

CSelectTool::CSelectTool()
    : IDrawTool(selection)
    , m_isItemMoving(false)
{
    connect(this, &CSelectTool::boardChanged, this, [ = ](DrawBoard * old, DrawBoard * cur) {
        Q_UNUSED(old)
        connect(cur, QOverload<Page *>::of(&DrawBoard::currentPageChanged), this, [ = ](Page * cur) {
            Q_UNUSED(cur)
            _hightLight = QPainterPath();
        });
    });
}

CSelectTool::~CSelectTool()
{

}

QCursor CSelectTool::cursor() const
{
    return QCursor(Qt::ArrowCursor);
}

QAbstractButton *CSelectTool::initToolButton()
{
    DToolButton *m_selectBtn = new DToolButton;
    m_selectBtn->setShortcut(QKeySequence(QKeySequence(Qt::Key_V)));
    setWgtAccesibleName(m_selectBtn, "Select tool button");
    m_selectBtn->setToolTip(tr("Select (V)"));
    m_selectBtn->setIconSize(QSize(48, 48));
    m_selectBtn->setFixedSize(QSize(37, 37));
    m_selectBtn->setCheckable(true);
    //m_selectBtn->setVisible(false);

    connect(m_selectBtn, &DToolButton::toggled, m_selectBtn, [ = ](bool b) {
        QIcon icon       = QIcon::fromTheme("ddc_choose tools_normal");
        QIcon activeIcon = QIcon::fromTheme("ddc_choose tools_active");
        m_selectBtn->setIcon(b ? activeIcon : icon);
    });
    m_selectBtn->setIcon(QIcon::fromTheme("ddc_choose tools_normal"));
    return m_selectBtn;
}

DrawAttribution::SAttrisList CSelectTool::attributions()
{
    DrawAttribution::SAttrisList result;
    return result;
}

void CSelectTool::setAttributionVar(int attri, const QVariant &var, int phase, bool autoCmdStack)
{
    if (drawBoard()->currentPage() != nullptr)
        drawBoard()->currentPage()->setAttributionVar(attri, var, phase, autoCmdStack);
}

void CSelectTool::registerAttributionWidgets()
{
    //注册自建的属性控件
    //0.注册标题显示控件
    _titleLabe = new QLabel;
    //不需要重新排列属性控件，所以设置为0
    _titleLabe->setProperty(AttriWidgetReWidth, 0);
    QString defaultText = tr("Unnamed");
    connect(drawBoard()->attributionWidget(), &CAttributeManagerWgt::updateWgt, this,
    [ = ](QWidget * pWgt, const QVariant & var) {
        if (pWgt == _titleLabe) {
            QString text = var.isValid() ? var.toString() : defaultText;
            //_titleLabe->setText(text);
            text = _titleLabe->fontMetrics().elidedText(text, Qt::ElideRight, drawBoard()->attributionWidget()->width());
            _titleLabe->setText(text);
        }
    });
    drawBoard()->attributionWidget()->installEventFilter(this);
    drawBoard()->attributionWidget()->installComAttributeWgt(ETitle, _titleLabe, defaultText);

    //14.组合/取消组合 设置控件
    auto group = new CGroupButtonWgt;

    connect(group, &CGroupButtonWgt::buttonClicked, this, [ = ](bool doGroup, bool doUngroup) {
        auto currentPage = this->drawBoard()->currentPage();
        if (currentPage == nullptr)
            return;

        auto currentScene = currentPage->scene();
        if (doGroup) {

            CGraphicsItem *pBaseItem = currentScene->selectGroup()->getLogicFirst();
            currentScene->creatGroup(QList<CGraphicsItem *>(), CGraphicsItemGroup::ENormalGroup,
                                     true, pBaseItem, true);
        }
        if (doUngroup) {
            currentScene->cancelGroup(nullptr, true);
        }
        drawBoard()->attributionWidget()->hideExpWindow();
        //另外需要将焦点转移到text
        auto pView = this->currentPage()->view();
        pView->captureFocus();
    });
    connect(drawBoard()->attributionWidget(), &CAttributeManagerWgt::updateWgt, this,
    [ = ](QWidget * pWgt, const QVariant & var) {
        if (pWgt == group) {
            bool canGroup = false;
            bool canUnGroup = false;
            QList<QVariant> bools = var.toList();
            if (bools.count() == 2) {
                canGroup   = bools[0].toBool();
                canUnGroup = bools[1].toBool();
            }
            group->setGroupFlag(canGroup, canUnGroup);
        }
    });
    drawBoard()->attributionWidget()->installComAttributeWgt(EGroupWgt, group);
}

void CSelectTool::toolStart(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    _hightLight = QPainterPath();

    CGraphicsItem *pStartPosTopBzItem = pInfo->startPosTopBzItem;

    QGraphicsItem *pFirstItem = pInfo->startPosItems.isEmpty() ? nullptr : pInfo->startPosItems.first();
    bool isMrNodeItem = event->scene()->isBussizeHandleNodeItem(pFirstItem) && (event->scene()->getAssociatedBzItem(pFirstItem)->type() == MgrType);

    bool clearBeforeSelect = true;

    if (event->keyboardModifiers() == Qt::ShiftModifier) {
        // 点住shift那么不用清除
        clearBeforeSelect = false;

        //首先应该要满足当前鼠标点击下的图元不是节点
        if (!isMrNodeItem) {
            //如果点击的图元当前是选中的,那么在按住shift的情况下应该是清除它的选中
            if (pStartPosTopBzItem != nullptr) {
                CGraphicsItem *pProxItem = pStartPosTopBzItem->thisBzProxyItem(true);
                if (pProxItem->isSelected()) {
                    if (event->scene()->selectGroup()->count() > 1) {
                        event->scene()->notSelectItem(pProxItem);
                        return;
                    }
                }
            }
        }
    } else {
        if (isMrNodeItem) {
            clearBeforeSelect = false;
        } else if (pStartPosTopBzItem != nullptr) {
            CGraphicsItem *pProxItem = pStartPosTopBzItem->thisBzProxyItem(true);
            if (pProxItem->isSelected()) {
                //其代理图元(自身或者其顶层组合图元)被选中那么不用清除
                clearBeforeSelect = false;
            }
        }
    }
    //清理当前选中
    if (clearBeforeSelect) {
        event->scene()->clearSelectGroup();
    }

    if (pStartPosTopBzItem != nullptr) {
        if (!isMrNodeItem) {
            event->scene()->selectItem(pStartPosTopBzItem->thisBzProxyItem());
        }
    } else {
        if (!isMrNodeItem) {
            //点击处是空白的那么清理所有选中
            event->scene()->clearSelectGroup();
        }
    }
}

void CSelectTool::toolUpdate(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    event->setAccepted(false);

    //根据要做的类型去执行相应的操作
    switch (pInfo->_opeTpUpdate) {
    case ERectSelect: {
        event->scene()->update();
        break;
    }
    case EDragSceneMove: {
        //拖拽画布移动
        QPointF pos0 = pInfo->_startPos;
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
        processItemsMove(event, pInfo, EChangedUpdate);
        break;
    }
    case EResizeMove: {
        //交给图元去完成
        processItemsScal(event, pInfo, EChangedUpdate);
        break;
    }
    case ERotateMove: {
        if (!pInfo->etcItems.isEmpty()) {
            QGraphicsItem *pItem = !pInfo->etcItems.isEmpty() ? pInfo->etcItems.first() : nullptr;
            CGraphicsItem *pMrItem = dynamic_cast<CGraphicsItem *>(pItem);
            processItemsRot(event, pInfo, EChangedUpdate);
            event->scene()->setCursor(pMrItem->handleNode()->getCursor());
        }
        break;
    }
    default:
        break;
    }
}

void CSelectTool::toolFinish(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    bool doUndoFinish = true;
    switch (pInfo->_opeTpUpdate) {
    case ERectSelect: {
        QPointF pos0 = pInfo->_startPos;
        QPointF pos1 = event->pos();

        QPointF topLeft(qMin(pos0.x(), pos1.x()), qMin(pos0.y(), pos1.y()));
        QPointF bomRight(qMax(pos0.x(), pos1.x()), qMax(pos0.y(), pos1.y()));

        event->scene()->selectItemsByRect(QRectF(topLeft, bomRight));

        event->scene()->update();

        doUndoFinish = false;
        break;
    }
    case EDragMove: {
        event->scene()->recordItemsInfoToCmd(event->scene()->selectGroup()->items(true), RedoVar);
        m_isItemMoving = false;
        break;
    }
    case EResizeMove: {
        //记录Redo点
        doUndoFinish = (event->view()->activeProxWidget() == nullptr);
        if (doUndoFinish)
            event->scene()->recordItemsInfoToCmd(event->scene()->selectGroup()->items(true), RedoVar);

        break;
    }
    case ECopyMove: {
        event->scene()->recordSecenInfoToCmd(CSceneUndoRedoCommand::EGroupChanged, RedoVar, event->scene()->selectGroup()->items());
        m_isItemMoving = false;
        break;
    }
    case ERotateMove: {
        event->scene()->recordItemsInfoToCmd(event->scene()->selectGroup()->items(true), RedoVar);
        break;
    }
    default:
        return;
    }

    QList<CGraphicsItem *> items = event->scene()->selectGroup()->getBzItems(true);
    if (!items.isEmpty()) {
        QGraphicsItem *pItem = items.first();
        if (pItem != nullptr && pItem->type() == PictureType) {
            event->scene()->selectGroup()->updateAttributes();
            event->scene()->updateAttribution();
        }
    }

    //入栈
    if (doUndoFinish)
        CUndoRedoCommand::finishRecord();

    mouseHoverEvent(event);

    IDrawTool::toolFinish(event, pInfo);
}

void CSelectTool::toolDoubleClikedEvent(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo)
{
    IDrawTool::toolDoubleClikedEvent(event, pInfo);

    //修复双击形成多选
    event->scene()->clearSelectGroup();

    if (pInfo->startPosTopBzItem != nullptr) {
        if (pInfo->startPosTopBzItem->type() == TextType) {
            CGraphicsTextItem *pTextItem = dynamic_cast<CGraphicsTextItem *>(pInfo->startPosTopBzItem);

            //如果是非编辑状态那么应该进入编辑状态
            if (pTextItem->textState() == CGraphicsTextItem::EReadOnly) {
                pTextItem->setTextState(CGraphicsTextItem::EInEdit, true);
            }
        }
    }
    event->setAccepted(true);
}

int CSelectTool::decideUpdate(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo)
{
    int tpye = ENothingDo;
    if (isWorking()) {
        QGraphicsItem *pStartPosTopQtItem = event->scene()->firstItem(pInfo->_startPos,
                                                                      pInfo->startPosItems,
                                                                      true, true);
        if (pStartPosTopQtItem == nullptr) {
            if (event->eventType() == CDrawToolEvent::ETouchEvent) {
                if (pInfo->elapsedFromStartToUpdate() > 200) {
                    tpye = ERectSelect;
                } else {
                    tpye = EDragSceneMove;
                }
            } else {
                tpye = ERectSelect;
            }
        } else {
            if (event->scene()->isBussizeItem(pStartPosTopQtItem)) {
                // 业务图元可执行移动可执行复制
                if (event->keyboardModifiers() == Qt::AltModifier) {
                    //复制移动
                    tpye = ECopyMove;
                    if (event->scene()->selectGroup()->count() > 0) {

                        //0.复制前得到当前场景的组合快照(用于Undo撤销)
                        event->scene()->recordSecenInfoToCmd(CSceneUndoRedoCommand::EGroupChanged, UndoVar);

                        //1.复制出一样的一个组合图元(这里复制的是选择组合图元,之后应该删掉它)
                        CGraphicsItemGroup *pNewGroup = PageScene::copyCreatGroup(event->scene()->selectGroup());

                        QList<CGraphicsItem *> needSelectItems;
                        if (pNewGroup != nullptr) {
                            //2.获取到拷贝出来的图元
                            QList<CGraphicsItem *> outItems = pNewGroup->items();
                            QList<CGraphicsItem *> newBzItems = pNewGroup->getBzItems(true);

                            //3.删掉复制出来的临时用于选择管理的组合,即pNewGroup
                            event->scene()->destoryGroup(pNewGroup, true);
                            delete pNewGroup;
                            pNewGroup = nullptr;

                            //4.记录增加的基本业务图元(用于撤销还原)
                            QList<QVariant> vars;
                            vars << reinterpret_cast<long long>(event->scene());
                            for (auto p : newBzItems) {
                                vars << reinterpret_cast<long long>(p);
                            }
                            CUndoRedoCommand::recordUndoCommand(CUndoRedoCommand::ESceneChangedCmd,
                                                                CSceneUndoRedoCommand::EItemAdded, vars, false, true);

                            needSelectItems = outItems;
                        }

                        //5.复制完成后记录场景当前组合的快照(用于还原)这要在toolFinished再调用
                        //event->scene()->recordSecenInfoToCmd(CSceneUndoRedoCommand::EGroupChanged, RedoVar);

                        //6.选中被复制出来的图元
                        event->scene()->clearSelectGroup();
                        for (auto p : needSelectItems) {
                            event->scene()->selectItem(p, true, false, false);
                        }
                        event->scene()->selectGroup()->updateBoundingRect();
                        event->scene()->selectGroup()->updateAttributes();
                        pInfo->etcItems.append(event->scene()->selectGroup());
                    }
                } else {
                    tpye = EDragMove;
                    pInfo->etcItems.append(event->scene()->selectGroup());
                    QList<CGraphicsItem *> lists = event->scene()->selectGroup()->items(true);
                    event->scene()->recordItemsInfoToCmd(lists, UndoVar);
                }
                m_isItemMoving = true;
            } else if (event->scene()->isBussizeHandleNodeItem(pStartPosTopQtItem)) {
                CSizeHandleRect *pHandle = dynamic_cast<CSizeHandleRect *>(pStartPosTopQtItem);
                pInfo->_etcopeTpUpdate = pHandle->dir();
                pInfo->etcItems.clear();

                pInfo->etcItems.append(event->scene()->selectGroup());

                //记录undo点
                if (event->view()->activeProxWidget() == nullptr)
                    event->scene()->recordItemsInfoToCmd(event->scene()->selectGroup()->items(true), UndoVar, true);

                tpye = (pHandle->dir() != CSizeHandleRect::Rotation ? EResizeMove : ERotateMove);
            }
        }
    }
    return tpye;
}

void CSelectTool::mouseHoverEvent(CDrawToolEvent *event)
{
    //处理高亮，鼠标样式变化等问题
    processHightLight(event);

    processCursor(event);
}

void CSelectTool::leaveEvent(CDrawToolEvent *event)
{
    Q_UNUSED(event);
    _hightLight = QPainterPath();
    event->scene()->update();
}

void CSelectTool::drawMore(QPainter *painter,
                           const QRectF &rect,
                           PageScene *scene)
{
    //注意painter是在scene的坐标系

    Q_UNUSED(rect)
    painter->save();

    if (_allITERecordInfo.isEmpty()) {
        painter->setBrush(Qt::NoBrush);
        QColor selectColor = scene->systemThemeColor();
        QPen p(selectColor);
        p.setWidthF(2.0);
        painter->setPen(p);
        if (!_hightLight.isEmpty() && CGraphicsItem::paintInteractBorderLine) {
            painter->drawPath(_hightLight);
        }
    }
    foreach (auto info, _allITERecordInfo.values()) {
        if (info._opeTpUpdate == ERectSelect) {
            QPointF pos0 = info._startPos;
            QPointF pos1 = info._curEvent.pos();

            QPointF topLeft(qMin(pos0.x(), pos1.x()), qMin(pos0.y(), pos1.y()));
            QPointF bomRight(qMax(pos0.x(), pos1.x()), qMax(pos0.y(), pos1.y()));

            QPen pen;
            pen.setWidthF(0.5);
            QBrush selectBrush = scene->systemThemeColor();
            QColor selectColor = selectBrush.color();
            selectColor.setAlpha(20);
            selectBrush.setColor(selectColor);
            selectColor.setAlpha(100);
            pen.setColor(selectColor);

            painter->setPen(pen);
            painter->setBrush(selectBrush);
            painter->drawRect(QRectF(topLeft, bomRight));
        } else if (info._opeTpUpdate == ERotateMove) {
            //功能：将旋转角度绘制到视口上

            painter->save();
            painter->setClipping(false);

            //重置所有变换，从而保证绘制时是视口的坐标系
            painter->resetTransform();

            QPoint  posInView  = scene->drawView()->viewport()->mapFromGlobal(QCursor::pos());

            QPointF paintPos = posInView + QPointF(50, 0);

            qreal rote = scene->selectGroup()->count() == 0 ? 0 : scene->selectGroup()->items().first()->drawRotation();
            QString angle = QString("%1°").arg(QString::number(rote, 'f', 1));
            QFont f;
            f.setPixelSize(14);

            QFontMetrics fontMetrics(f);
            int width = fontMetrics.width(angle) + 6;
            QRectF rotateRect(paintPos, paintPos + QPointF(width, fontMetrics.height()));

            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor("#E5E5E5"));
            painter->drawRoundRect(rotateRect);
            painter->setFont(f);
            painter->setPen(Qt::black);
            painter->drawText(rotateRect, Qt::AlignCenter, angle);
            painter->restore();
        }
    }
    painter->restore();
}

void CSelectTool::onStatusChanged(EStatus oldStatus, EStatus nowStatus)
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

bool CSelectTool::returnToSelectTool(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    Q_UNUSED(event)
    Q_UNUSED(pInfo)
    return false;
}

void CSelectTool::sendToolEventToItem(CDrawToolEvent *event,
                                      ITERecordInfo *info,
                                      EChangedPhase phase)
{
    CGraphItemEvent::EItemType tp = CGraphItemEvent::EUnKnow;
    switch (info->_opeTpUpdate) {
    case EDragMove:
    case ECopyMove:
        tp = CGraphItemEvent::EMove;
        break;
    case EResizeMove:
        tp = CGraphItemEvent::EScal;
        break;
    case ERotateMove:
        tp = CGraphItemEvent::ERot;
        break;
    default:
        tp = CGraphItemEvent::EUnKnow;
        break;
    }

    switch (tp) {
    case CGraphItemEvent::EMove: {
        processItemsMove(event, info, phase);
        break;
    }
    case CGraphItemEvent::EScal: {
        processItemsScal(event, info, phase);
        break;
    }
    case CGraphItemEvent::ERot: {
        processItemsRot(event, info, phase);
        break;
    }
    default:
        break;
    }
}

void CSelectTool::processItemsScal(CDrawToolEvent *event, IDrawTool::ITERecordInfo *info, EChangedPhase phase)
{
    CGraphItemScalEvent scal(CGraphItemEvent::EScal);
    scal.setEventPhase(phase);
    scal.setToolEventType(info->_opeTpUpdate);
    scal.setPressedDirection(info->_etcopeTpUpdate);
    scal._scenePos = event->pos();
    scal._oldScenePos = info->_prePos;
    scal._sceneBeginPos = info->_startPos;

    bool xBlock = false;
    bool yBlock = false;
    CSizeHandleRect::EDirection dir = CSizeHandleRect::EDirection(info->_etcopeTpUpdate);
    CSizeHandleRect::getTransBlockFlag(dir, xBlock, yBlock);
    scal.setXTransBlocked(xBlock);
    scal.setYTransBlocked(yBlock);

    bool xNegitiveOffset = false;
    bool yNegitiveOffset = false;
    CSizeHandleRect::getTransNegtiveFlag(dir, xNegitiveOffset, yNegitiveOffset);
    scal.setXNegtiveOffset(xNegitiveOffset);
    scal.setYNegtiveOffset(yNegitiveOffset);
    scal.setKeepOrgRadio(event->keyboardModifiers() & Qt::ShiftModifier);

    //分发事件
    for (auto item : info->etcItems) {
        if (PageScene::isDrawItem(item) || item == event->scene()->selectGroup()) {
            CGraphicsItem *pBzItem = static_cast<CGraphicsItem *>(item);

            scal.setPos(pBzItem->mapFromScene(event->pos()));
            scal.setOldPos(pBzItem->mapFromScene(info->_prePos));
            scal.setOrgSize(pBzItem->rect().size());
            scal.setCenterPos((event->keyboardModifiers() & Qt::AltModifier) ? pBzItem->rect().center() :
                              CSizeHandleRect::transCenter(dir, pBzItem));
            scal._sceneCenterPos = pBzItem->mapToScene(scal.centerPos());

            if (phase == EChangedBegin) {
                pBzItem->operatingBegin(&scal);
            } else if (phase == EChangedUpdate) {
                pBzItem->operating(&scal);
            } else if (phase == EChangedFinished) {
                pBzItem->operatingEnd(&scal);
            }
        }
    }
    event->view()->viewport()->update();
}

void CSelectTool::processItemsRot(CDrawToolEvent *event, IDrawTool::ITERecordInfo *info, EChangedPhase phase)
{
    CGraphItemRotEvent rot(CGraphItemEvent::ERot);
    rot.setEventPhase(phase);
    rot.setToolEventType(info->_opeTpUpdate);
    rot.setPressedDirection(info->_etcopeTpUpdate);
    rot._scenePos = event->pos();
    rot._oldScenePos = info->_prePos;
    rot._sceneBeginPos = info->_startPos;

    //分发事件
    for (auto item : info->etcItems) {
        if (PageScene::isDrawItem(item) || item == event->scene()->selectGroup()) {
            CGraphicsItem *pBzItem = static_cast<CGraphicsItem *>(item);
            rot.setPos(pBzItem->mapFromScene(event->pos()));
            rot.setOldPos(pBzItem->mapFromScene(info->_prePos));
            rot.setOrgSize(pBzItem->boundingRectTruly().size());
            rot.setCenterPos(pBzItem->boundingRectTruly().center());
            rot._sceneCenterPos = pBzItem->mapToScene(rot.centerPos());
            if (phase == EChangedBegin) {
                pBzItem->operatingBegin(&rot);
            } else if (phase == EChangedUpdate) {
                pBzItem->operating(&rot);
            } else if (phase == EChangedFinished) {
                pBzItem->operatingEnd(&rot);
            }
        }
    }
    event->view()->viewport()->update();
}

void CSelectTool::processHightLight(CDrawToolEvent *event)
{
    auto hlPath = QPainterPath();

    if (event->view()->activeProxWidget() == nullptr) {
        QGraphicsItem *pItem = event->scene()->firstItem(event->pos(), QList<QGraphicsItem *>(),
                                                         true, true, true, true);
        if (pItem != nullptr) {
            CGraphicsItem *pBzItem = static_cast<CGraphicsItem *>(pItem);
            hlPath = pBzItem->mapToScene(pBzItem->getHighLightPath());
        }
    }
    if (hlPath != _hightLight) {
        _hightLight = hlPath;
        event->scene()->update();
    }
}

void CSelectTool::processCursor(CDrawToolEvent *event)
{
    QPointF scenePos = event->pos();

    QList<QGraphicsItem *> items = event->scene()->items(scenePos);

    QGraphicsItem *pItem = event->scene()->firstItem(scenePos, items, true, true, false, false, false);
    if (pItem != nullptr) {
        CSizeHandleRect *pRectItem = dynamic_cast<CSizeHandleRect *>(pItem);
        if (pRectItem != nullptr) {
            event->scene()->setCursor(pRectItem->getCursor());
        } else {
            event->scene()->setCursor(pItem->cursor());
        }
    } else {
        event->scene()->setCursor(cursor());
    }
}

bool CSelectTool::eventFilter(QObject *o, QEvent *e)
{
    auto borad = drawBoard();
    if (borad != nullptr && borad->currentPage() != nullptr && borad->currentPage()->currentTool() == selection) {
        if (o == borad->attributionWidget() && (e->type() == QEvent::Resize || e->type() == QEvent::Show)) {
            auto text = borad->currentPage()->name();
            text = _titleLabe->fontMetrics().elidedText(text, Qt::ElideRight, borad->attributionWidget()->width());
            _titleLabe->setText(text);
        }
    }
    return IDrawTool::eventFilter(o, e);
}

void CSelectTool::processItemsMove(CDrawToolEvent *event,
                                   ITERecordInfo *info,
                                   EChangedPhase phase)
{
    CGraphItemMoveEvent mov(CGraphItemEvent::EMove);
    mov.setEventPhase(phase);
    mov.setToolEventType(info->_opeTpUpdate);
    mov.setPressedDirection(info->_etcopeTpUpdate);
    mov._scenePos = event->pos();
    mov._oldScenePos = info->_prePos;
    mov._sceneBeginPos = info->_startPos;

    //分发事件
    foreach (auto item, info->etcItems) {
        if (PageScene::isDrawItem(item) || item == event->scene()->selectGroup()) {
            CGraphicsItem *pBzItem = static_cast<CGraphicsItem *>(item);
            mov.setPos(pBzItem->mapFromScene(event->pos()));
            mov.setOldPos(pBzItem->mapFromScene(info->_prePos));
            mov.setOrgSize(pBzItem->boundingRectTruly().size());
            mov.setCenterPos(pBzItem->boundingRectTruly().center());
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
}
