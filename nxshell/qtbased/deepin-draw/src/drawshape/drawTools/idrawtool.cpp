// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "idrawtool.h"
#include "cdrawscene.h"
#include "frame/cviewmanagement.h"
#include "frame/cgraphicsview.h"
#include "drawshape/cdrawparamsigleton.h"
#include "application.h"
#include "frame/cundoredocommand.h"
#include "cgraphicsitem.h"
#include "cdrawtoolfactory.h"



#include <QDebug>
#include <QKeyEvent>
#include <QGraphicsSceneEvent>
#include <QGraphicsProxyWidget>
#include <QTimer>
#include <QToolButton>

QTimer *IDrawTool::s_timerForDoubleClike = nullptr;

IDrawTool::IDrawTool(EDrawToolMode mode)
    : QObject(nullptr)
    , m_bMousePress(false)
    , m_sPointPress(0, 0)
    , m_sLastPress(0, 0)
    , m_sPointRelease(0, 0)
    , m_bShiftKeyPress(false)
    , m_bAltKeyPress(false)
    , m_noShiftSelectItem(nullptr)
    , m_mode(mode)
    , m_RotateCursor(QPixmap(":/theme/light/images/mouse_style/rotate_mouse.svg").scaled(24, 24, Qt::IgnoreAspectRatio, Qt::SmoothTransformation))
    , m_LeftTopCursor(QPixmap(":/theme/light/images/mouse_style/icon_drag_leftup.svg").scaled(24, 24, Qt::IgnoreAspectRatio, Qt::SmoothTransformation))
    , m_RightTopCursor(QPixmap(":/theme/light/images/mouse_style/icon_drag_rightup.svg").scaled(24, 24, Qt::IgnoreAspectRatio, Qt::SmoothTransformation))
    , m_LeftRightCursor(QPixmap(":/theme/light/images/mouse_style/icon_drag_left.svg").scaled(24, 24, Qt::IgnoreAspectRatio, Qt::SmoothTransformation))
    , m_UpDownCursor(QPixmap(":/theme/light/images/mouse_style/icon_drag_up.svg").scaled(24, 24, Qt::IgnoreAspectRatio, Qt::SmoothTransformation))
{
    QMetaObject::invokeMethod(this, [ = ]() {
        if (!_registedWidgets) {
            //registerAttributionWidgets();
            _registedWidgets = true;
        }
    }, Qt::QueuedConnection);


    connect(this, &IDrawTool::statusChanged, this, [ = ](EStatus oldStatus, EStatus newStatus) {
//        if (newStatus == EReady) {
//            qWarning() << "status changed to EReady, tool type is: " << getDrawToolMode();
//        }
        onStatusChanged(oldStatus, newStatus);
    });
}

IDrawTool::EStatus IDrawTool::status()
{
    return _status;
}

void IDrawTool::setEnable(bool b)
{
//    if (b) {
//        if (isEnable(CURRENTVIEW)) {
//            toolButton()->setEnabled(true);
//            _status = EIdle;
//        } else {
//            qWarning() << "tool can not set to enable because virtual function \"isEnable\" return false in current view!";
//        }
//    } else {
//        toolButton()->setEnabled(false);
//        _status = EDisAbled;
//    }

    toolButton()->setEnabled(b);
    changeStatusFlagTo(b ? EIdle : EDisAbled);
}

bool IDrawTool::activeTool()
{
    //return CDrawToolFactory::setCurrentTool(getDrawToolMode(), false);
    toolButton()->setChecked(true);
    return true;
}

QAbstractButton *IDrawTool::toolButton()
{
    if (_pToolButton == nullptr) {
        _pToolButton = initToolButton();
    }
    return _pToolButton;
}

QCursor IDrawTool::cursor() const
{
    return QCursor(Qt::ArrowCursor);
}

QAbstractButton *IDrawTool::initToolButton()
{
    return nullptr;
}

DrawAttribution::SAttrisList IDrawTool::attributions()
{
    return QList<int>() << 0;
}

void IDrawTool::setAttributionVar(int attri, const QVariant &var, int phase, bool autoCmdStack)
{
    Q_UNUSED(attri)
    Q_UNUSED(var)
    Q_UNUSED(phase)
    Q_UNUSED(autoCmdStack)
}

void IDrawTool::registerAttributionWidgets()
{

}

void IDrawTool::onStatusChanged(EStatus oldStatus, EStatus nowStatus)
{
    if (currentPage() == nullptr)
        return;

    if (oldStatus == EIdle && nowStatus == EReady) {
        if (this->currentPage()->view() != nullptr)
            this->currentPage()->view()->drawScene()->clearSelectGroup();
    }
}

IDrawTool::~IDrawTool()
{

}

void IDrawTool::mousePressEvent(QGraphicsSceneMouseEvent *event, PageScene *scene)
{
    //1.如果由qt将触控事件转成的鼠标事件那么不要再调用toolDoStart（因为在scene的event中已经处理过）
//    if (event->source() == Qt::MouseEventSynthesizedByQt) {
//        return scene->mouseEvent(event);
//    }
    CDrawToolEvent::CDrawToolEvents e = CDrawToolEvent::fromQEvent(event, scene);

    toolDoStart(&e.first());

    if (!e.first().isAccepted()) {
        scene->mouseEvent(event);
    }
}

void IDrawTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, PageScene *scene)
{
    //1.如果由qt将触控事件转成的鼠标事件那么不要再调用toolDoUpdate（因为在scene的event中已经处理过）
//    if (event->source() == Qt::MouseEventSynthesizedByQt) {
//        return scene->mouseEvent(event);
//    }

    CDrawToolEvent::CDrawToolEvents e = CDrawToolEvent::fromQEvent(event, scene);

    toolDoUpdate(&e.first());

    //已经处理完成，不用传递到框架,否则传递给框架继续处理
    if (!e.first().isAccepted()) {
        scene->mouseEvent(event);
    }
}

void IDrawTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event, PageScene *scene)
{
    //1.如果由qt将触控事件转成的鼠标事件那么不要再调用toolDoFinish（因为在scene的event中已经处理过）
//    if (event->source() == Qt::MouseEventSynthesizedByQt) {
//        return scene->mouseEvent(event);
//    }

    CDrawToolEvent::CDrawToolEvents e = CDrawToolEvent::fromQEvent(event, scene);

    toolDoFinish(&e.first());

    //已经处理完成，不用传递到框架,否则传递给框架继续处理
    if (!e.first().isAccepted()) {
        scene->mouseEvent(event);
    }
}
void IDrawTool::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event, PageScene *scene)
{
    //1.如果由qt将触控事件转成的鼠标事件那么不要再调用（因为在scene的event中已经处理过）
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        return scene->mouseEvent(event);
    }

    CDrawToolEvent::CDrawToolEvents e = CDrawToolEvent::fromQEvent(event, scene);

    dueTouchDoubleClickedStart(&e.first());

    if (!e.first().isAccepted()) {
        scene->mouseEvent(event);
    }
}

void IDrawTool::enterEvent(CDrawToolEvent *event)
{
    Q_UNUSED(event)
}

void IDrawTool::leaveEvent(CDrawToolEvent *event)
{
    Q_UNUSED(event)
}

bool IDrawTool::isWorking()
{
    return !_allITERecordInfo.isEmpty();
}

void IDrawTool::interrupt()
{
    clearITE();
    if (status() == EWorking)
        changeStatusFlagTo(EReady);
}

DrawBoard *IDrawTool::drawBoard() const
{
    return _drawBoard;
}

void IDrawTool::setDrawBoard(DrawBoard *board)
{
    if (board != _drawBoard) {
        auto old = _drawBoard;
        _drawBoard = board;
        emit boardChanged(old, _drawBoard);
    }
}

Page *IDrawTool::currentPage() const
{
    if (drawBoard() != nullptr) {
        return drawBoard()->currentPage();
    }
    return nullptr;
}

void IDrawTool::setTouchSensitiveRadius(int sensitiveRadius)
{
    _touchSensitiveRadius = sensitiveRadius;
}

//int IDrawTool::touchSensitiveRadius() const
//{
//    return _touchSensitiveRadius;
//}

void IDrawTool::toolDoStart(CDrawToolEvent *event)
{
    //event->scene()->clearHighlight();
    if (event->mouseButtons() == Qt::LeftButton || event->eventType() == CDrawToolEvent::ETouchEvent) {

        if (dueTouchDoubleClickedStart(event)) {
            return;
        }
        int incW = drawBoard()->touchFeelingEnhanceValue();
        ITERecordInfo info;

        info._prePos       = event->pos();
        info._startPos     = event->pos();
        info.startPosItems = event->eventType() == CDrawToolEvent::ETouchEvent ?
                             event->scene()->items(QRectF(event->pos() - QPoint(incW, incW), QSizeF(2 * incW, 2 * incW))) :
                             event->scene()->items(event->pos());
        info.startPosTopBzItem = event->scene()->topBzItem(event->pos(), true,
                                                           event->eventType() == CDrawToolEvent::ETouchEvent ? drawBoard()->touchFeelingEnhanceValue() : 0);
        info._isvaild  = true;
        info._curEvent = *event;
        info._startEvent = *event;
        info._scene    = event->scene();
        info.getTimeHandle()->restart();

        if (getCurVaildActivedPointCount() >= allowedMaxTouchPointCount()) {
            event->setAccepted(true);
            //超过可支持的点数了
            info.eventLife = EDoNotthing;
            qDebug() << "toolDoStart rInfo.eventLife = EDoNotthing id " << event->uuid();
            _allITERecordInfo.insert(event->uuid(), info);
        } else if (isPressEventHandledByQt(event, &info)) {
            event->setAccepted(false);
            info.eventLife = EDoQtCoversion;
            qDebug() << "toolDoStart rInfo.eventLife = EDoQtCoversion id " << event->uuid();
            //只有真实的鼠标事件的调用才需要备份该事件信息,之后会调用toolDoUpdate,toolDoFinished，从而在其中通过UUID找到该事件信息以决定再将该事件传递给mouseEvent
            //触控操作在这里会被转成qt的鼠标事件直接去调用mouseEvent，不会再进入toolDoStart，toolDoUpdate,toolDoFinished，所以不需要备份保存信息
            if (event->eventType() == CDrawToolEvent::EMouseEvent)
                _allITERecordInfo.insert(event->uuid(), info);
        } else {
            info.businessItem  = creatItem(event, &info);
            //设置默认属性
            if (info.businessItem != nullptr) {
                auto attrisList = this->attributions();
                foreach (auto attri, attrisList) {
                    info.businessItem->setAttributionVar(attri.attri, attri.var, EChanged);
                }
            }

            info.eventLife     = ENormal;
            qDebug() << "toolDoStart rInfo.eventLife = ENormal id " << event->uuid();

            _allITERecordInfo.insert(event->uuid(), info);

            if (info.businessItem != nullptr) {

                //工具开始创建图元应该清理当前选中情况
                event->scene()->clearSelection();

                toolCreatItemStart(event, &info);

            } else {
                toolStart(event, &info);
            }
        }
    }
}

void IDrawTool::toolDoUpdate(CDrawToolEvent *event)
{
    if (!_allITERecordInfo.isEmpty()) {
        auto it = _allITERecordInfo.find(event->uuid());
        if (it != _allITERecordInfo.end()) {
            ITERecordInfo &rInfo = it.value();
            rInfo._preEvent = rInfo._curEvent;
            rInfo._curEvent = *event;

            //判定是否移动过(根据工具不同移动的最小距离值不同可重载minMoveUpdateDistance)
            if (!rInfo.hasMoved()) {
                int constDis = minMoveUpdateDistance();

                QPointF offset = event->pos(CDrawToolEvent::EViewportPos) -
                                 rInfo._startEvent.pos(CDrawToolEvent::EViewportPos);
                int curDis = qRound(offset.manhattanLength());
                rInfo._moved = (curDis >= constDis);
            }

            if (rInfo.eventLife == EDoNotthing) {
                event->setAccepted(true);
            } else if (rInfo.eventLife == EDoQtCoversion) {
                //0.如果开始事件是不被接受的（传递给了Qt框架），那么Update也应该不被接受（也应该传递给Qt框架）
                event->setAccepted(false);
            } else {
                if (!rInfo.haveDecidedOperateType) {
                    //判定应该做什么
                    if (rInfo.businessItem != nullptr) {
                        if (rInfo.hasMoved()) {
                            rInfo._opeTpUpdate = EToolCreatItemMove;
                            rInfo.haveDecidedOperateType = true;
                        }
                    } else {
                        bool doDecide = true;
                        bool isTouch = (event->eventType() == CDrawToolEvent::ETouchEvent);
                        if (isTouch) {
                            if (_touchSensitiveRadius != 0) {
                                QRectF rectf(event->view()->mapFromScene(rInfo._startPos) - QPointF(_touchSensitiveRadius, _touchSensitiveRadius), QSizeF(2 * _touchSensitiveRadius, 2 * _touchSensitiveRadius));
                                doDecide = !rectf.contains(event->pos(CDrawToolEvent::EViewportPos));
                            }
                        }
                        if (doDecide) {
                            QTime *elTi = rInfo.getTimeHandle();
                            rInfo._elapsedToUpdate = (elTi == nullptr ? -1 : elTi->elapsed());
                            rInfo._opeTpUpdate = decideUpdate(event, &rInfo);

                            if (rInfo._opeTpUpdate > 0) {
                                sendToolEventToItem(event, &rInfo, EChangedBegin);
                            }

                            rInfo.haveDecidedOperateType = true;
                        } else {
                            event->setPosXAccepted(false);
                            event->setPosYAccepted(false);
                        }
                    }
                }
                //2.执行操作
                if (rInfo.haveDecidedOperateType) {
                    if (rInfo._opeTpUpdate == EToolCreatItemMove) {
                        changeStatusFlagTo(EWorking);
                        toolCreatItemUpdate(event, &rInfo);
                    } else if (rInfo._opeTpUpdate > EToolDoNothing) {
                        changeStatusFlagTo(EWorking);
                        toolUpdate(event, &rInfo);
                    }
                }
            }
            //qDebug() << "event->isPosXAccepted() = " << event->isPosXAccepted() << "event->isPosYAccepted() = " << event->isPosYAccepted();
            if (event->isPosXAccepted())
                rInfo._prePos.setX(event->pos().x());
            if (event->isPosYAccepted())
                rInfo._prePos.setY(event->pos().y());
        }
    } else {
        mouseHoverEvent(event);
    }
}

void IDrawTool::toolDoFinish(CDrawToolEvent *event)
{
    qDebug() << "toolDoFinish ==== " << event->uuid();
    if (!_allITERecordInfo.isEmpty()) {
        auto it = _allITERecordInfo.find(event->uuid());
        if (it != _allITERecordInfo.end()) {
            ITERecordInfo &rInfo = it.value();
            rInfo._prePos = event->pos();
            rInfo._preEvent = rInfo._curEvent;
            rInfo._curEvent = *event;
            qDebug() << "toolDoFinish rInfo.eventLife = " << rInfo.eventLife;

            CGraphicsItem *pCreatedItem = nullptr;
            if (rInfo.eventLife == EDoNotthing) {
                event->setAccepted(true);
            } else if (rInfo.eventLife == EDoQtCoversion) {
                event->setAccepted(false);
            } else {
                //1.根据操作类型决定要做的事情
                if (rInfo.businessItem != nullptr) {
                    toolCreatItemFinish(event, &rInfo);
                    pCreatedItem = rInfo.businessItem;
                    if (rInfo.businessItem != nullptr) {
                        if (rInfo.businessItem->scene() == event->scene()) {
                            if (rInfo.businessItem->layer() != nullptr && rasterItemToLayer(event, &rInfo)) {
                                rInfo.businessItem->rasterToSelfLayer(true);
                                rInfo.businessItem = nullptr;
                                pCreatedItem = nullptr;
                            } else {
                                if (autoSupUndoForCreatItem())
                                    CCmdBlock block(event->scene(), CSceneUndoRedoCommand::EItemAdded, rInfo.businessItem);
                            }
                        }
                    }
                } else {
                    if (rInfo._opeTpUpdate > EToolDoNothing)
                        sendToolEventToItem(event, &rInfo, EChangedFinished);
                    toolFinish(event, &rInfo);
                }
                // 保证恢复到正常绘制
                if (event->view()->isCacheEnabled() && _allITERecordInfo.count() == 1) {
                    event->view()->setCacheEnable(false);
                }
            }

            qDebug() << "finished uuid ===== " << event->uuid() << "allITERecordInfo count = " << _allITERecordInfo.count();

            //2.是否要回到select工具模式下去(多个触控时即将为空才判断)
            if (_allITERecordInfo.count() == 1) {

                if (returnToSelectTool(event, &rInfo)) {
                    if (pCreatedItem != nullptr) {
                        event->scene()->selectItem(pCreatedItem);
                        event->scene()->setFocusItem(pCreatedItem);
                    }
                    changeStatusFlagTo(EReady);
                    drawBoard()->setCurrentTool(selection);
                } else {
                    changeStatusFlagTo(EReady);
                }
            }
            _allITERecordInfo.erase(it);
        }
    } else {
        event->setAccepted(false);
    }

    event->view()->setFocus();
}

bool IDrawTool::isEnable(PageView *pView)
{
    Q_UNUSED(pView)
    return true;
}

void IDrawTool::refresh()
{
    auto curView = currentPage() != nullptr ? currentPage()->view() : nullptr;
    if (curView != nullptr) {
        auto viewPos  = curView->viewport()->mapFromGlobal(QCursor::pos());
        auto scenePos = curView->mapToScene(viewPos);
        curView->viewport()->setCursor(cursor());
        if (qApp->activePopupWidget() != nullptr) {
            CDrawToolEvent event(QPointF(), QPointF(), QPointF(), curView->drawScene());
            mouseHoverEvent(&event);
            return;
        }
        CDrawToolEvent event(viewPos, scenePos, QCursor::pos(), curView->drawScene());
        mouseHoverEvent(&event);
    }
}

DrawAttribution::SAttri IDrawTool::defaultAttriVar(int attri) const
{
    return DrawAttribution::SAttri(attri, drawBoard()->defaultAttriVar(drawBoard()->currentPage(), attri));
}

bool IDrawTool::dueTouchDoubleClickedStart(CDrawToolEvent *event)
{
    static const int intervalMs = 250;
    if (event->eventType() == CDrawToolEvent::ETouchEvent) {
        static QPointF prePos;
        //touch触控判定是否是双击
        if (!getTimerForDoubleCliked()->isActive()) {
            prePos = event->pos();
            getTimerForDoubleCliked()->start(intervalMs);
        } else {
            //判定移动的幅度很小
            QRectF rectf(prePos - QPointF(10, 10), QSizeF(20, 20));
            if (rectf.contains(event->pos())) {
                m_bMousePress = true;

                ITERecordInfo info;

                info._prePos = event->pos();
                info._startPos = event->pos();
                info.startPosItems = event->scene()->items(event->pos());
                info.startPosTopBzItem = event->scene()->topBzItem(event->pos());
                info._isvaild = true;
                info._curEvent = *event;
                info._scene = event->scene();

                _allITERecordInfo.insert(event->uuid(), info);

                if (isPressEventHandledByQt(event, &info)) {
                    //触控事件的双击如果需要传递给Qt去处理,那么需要清理掉事件
                    event->setAccepted(false);
                    interrupt();
                } else {
                    toolDoubleClikedEvent(event, &info);
                }
                getTimerForDoubleCliked()->stop();

                return true;
            }
        }
    } else if (event->eventType() == CDrawToolEvent::EMouseEvent) {
        m_bMousePress = true;
        QEvent::Type tp = event->orgQtEvent()->type();
        if ((tp == QEvent::MouseButtonDblClick || tp == QEvent::GraphicsSceneMouseDoubleClick) && event->mouseButtons() == Qt::LeftButton) {
            ITERecordInfo info;

            info._prePos = event->pos();
            info._startPos = event->pos();
            info.startPosItems = event->scene()->items(event->pos());
            info.startPosTopBzItem = event->scene()->topBzItem(event->pos());
            info._isvaild = true;
            info._curEvent = *event;
            info._scene = event->scene();

            _allITERecordInfo.insert(event->uuid(), info);

            if (isPressEventHandledByQt(event, &info)) {
                event->setAccepted(false);
                _allITERecordInfo[event->uuid()].eventLife = EDoQtCoversion;
            } else {
                toolDoubleClikedEvent(event, &info);
            }
            return true;
        }
    }
    return false;
}

void IDrawTool::toolStart(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    Q_UNUSED(pInfo)

    //默认不传递事件给框架
    event->setAccepted(true);
}

void IDrawTool::toolUpdate(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    Q_UNUSED(pInfo)

    //默认传递事件给框架
    event->setAccepted(true);
}

void IDrawTool::toolFinish(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    Q_UNUSED(pInfo)

    //默认传递事件给框架
    event->setAccepted(true);

    event->scene()->update();
}

int IDrawTool::decideUpdate(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo)
{
    Q_UNUSED(event)
    Q_UNUSED(pInfo)

    //默认-2是 刷新新创造的item的路径 的标记（只是必要条件 不充分）
    return -2;
}

void IDrawTool::toolCreatItemStart(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo)
{
    Q_UNUSED(pInfo)

    //默认不传递事件给框架
    event->setAccepted(true);

    event->scene()->update();
}

void IDrawTool::toolCreatItemUpdate(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo)
{
    Q_UNUSED(pInfo)

    //默认不传递事件给框架
    event->setAccepted(true);

    event->scene()->update();
}

void IDrawTool::toolCreatItemFinish(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo)
{
    Q_UNUSED(pInfo)

    //默认不传递事件给框架
    event->setAccepted(true);

    event->scene()->update();
}

void IDrawTool::clearITE()
{
    _allITERecordInfo.clear();
}

void IDrawTool::setViewToSelectionTool(PageView *pView)
{
    Q_UNUSED(pView)
    auto pSelectTool = CDrawToolManagerSigleton::GetInstance()->getDrawTool(selection);
    pSelectTool->activeTool();

}

CGraphicsItem *IDrawTool::creatItem(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    Q_UNUSED(event)
    Q_UNUSED(pInfo)
    return nullptr;
}

int IDrawTool::minMoveUpdateDistance()
{
    return dApp->startDragDistance();
}

bool IDrawTool::blockPageBeforeOutput(Page *page)
{
    Q_UNUSED(page)
    return false;
}

void IDrawTool::mouseHoverEvent(CDrawToolEvent *event)
{
    // 只有鼠标才存在hover事件
    event->scene()->setCursor(cursor());
}

void IDrawTool::toolDoubleClikedEvent(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    Q_UNUSED(event)
    Q_UNUSED(pInfo)
    qDebug() << "IDrawTool::toolDoubleClikedEvent == " << event->pos() << "tp = " << event->eventType();
}

void IDrawTool::drawMore(QPainter *painter, const QRectF &rect, PageScene *scene)
{
    //注意painter是在viewport的坐标系,绘制时需要转换
    Q_UNUSED(painter)
    Q_UNUSED(rect)
    Q_UNUSED(scene)
}

EDrawToolMode IDrawTool::getDrawToolMode() const
{
    return m_mode;
}

QCursor IDrawTool::getCursor(CSizeHandleRect::EDirection dir, bool bMouseLeftPress, char toolType)
{
    Q_UNUSED(dir)
    Q_UNUSED(bMouseLeftPress)
    Q_UNUSED(toolType)
#if 0
    CGraphicsView *pView = CManageViewSigleton::GetInstance()->getCurView();
    if (pView != nullptr) {
        if (pView->isKeySpacePressed()) {
            return (*qApp->overrideCursor());
        }
    }

    Qt::CursorShape result;
    QCursor resultCursor;
    QMatrix matrix;
    QPixmap pixmap;
    switch (dir) {
    case CSizeHandleRect::Right:
        if (bMouseLeftPress) {
            result =  Qt::ClosedHandCursor;
            resultCursor = QCursor(result);
        } else {
            resultCursor = m_LeftRightCursor;
            matrix.rotate(getCursorRotation());
            pixmap = resultCursor.pixmap().transformed(matrix, Qt::SmoothTransformation);
            resultCursor = QCursor(pixmap);
        }
        break;
    case CSizeHandleRect::RightTop:
        if (bMouseLeftPress) {
            result =  Qt::ClosedHandCursor;
            resultCursor = QCursor(result);
        } else {
            resultCursor = m_RightTopCursor;
            matrix.rotate(getCursorRotation());
            pixmap = resultCursor.pixmap().transformed(matrix, Qt::SmoothTransformation);
            resultCursor = QCursor(pixmap);
        }
        break;
    case CSizeHandleRect::RightBottom:
        if (bMouseLeftPress) {
            result =  Qt::ClosedHandCursor;
            resultCursor = QCursor(result);
        } else {
            resultCursor = m_LeftTopCursor;
            matrix.rotate(getCursorRotation());
            pixmap = resultCursor.pixmap().transformed(matrix, Qt::SmoothTransformation);
            resultCursor = QCursor(pixmap);
        }
        break;
    case CSizeHandleRect::LeftBottom:
        if (bMouseLeftPress) {
            result =  Qt::ClosedHandCursor;
            resultCursor = QCursor(result);
        } else {
            resultCursor = m_RightTopCursor;
            matrix.rotate(getCursorRotation());
            pixmap = resultCursor.pixmap().transformed(matrix, Qt::SmoothTransformation);
            resultCursor = QCursor(pixmap);
        }
        break;
    case CSizeHandleRect::Bottom:
        if (bMouseLeftPress) {
            result =  Qt::ClosedHandCursor;
            resultCursor = QCursor(result);
        } else {
            resultCursor = m_UpDownCursor;
            matrix.rotate(getCursorRotation());
            pixmap = resultCursor.pixmap().transformed(matrix, Qt::SmoothTransformation);
            resultCursor = QCursor(pixmap);
        }
        break;
    case CSizeHandleRect::LeftTop:
        if (bMouseLeftPress) {
            result =  Qt::ClosedHandCursor;
            resultCursor = QCursor(result);
        } else {
            resultCursor = m_LeftTopCursor;
            matrix.rotate(getCursorRotation());
            pixmap = resultCursor.pixmap().transformed(matrix, Qt::SmoothTransformation);
            resultCursor = QCursor(pixmap);
        }
        break;
    case CSizeHandleRect::Left:
        if (bMouseLeftPress) {
            result =  Qt::ClosedHandCursor;
            resultCursor = QCursor(result);
        } else {
            resultCursor = m_LeftRightCursor;
            matrix.rotate(getCursorRotation());
            pixmap = resultCursor.pixmap().transformed(matrix, Qt::SmoothTransformation);
            resultCursor = QCursor(pixmap);
        }
        break;
    case CSizeHandleRect::Top:
        if (bMouseLeftPress) {
            result =  Qt::ClosedHandCursor;
            resultCursor = QCursor(result);
        } else {
            resultCursor = m_UpDownCursor;
            matrix.rotate(getCursorRotation());
            pixmap = resultCursor.pixmap().transformed(matrix, Qt::SmoothTransformation);
            resultCursor = QCursor(pixmap);
        }
        break;

    case CSizeHandleRect::Rotation: {
        resultCursor = m_RotateCursor;
        matrix.rotate(getCursorRotation());
        pixmap = resultCursor.pixmap().transformed(matrix, Qt::SmoothTransformation);
        resultCursor = QCursor(pixmap);
    }
    break;
    case CSizeHandleRect::InRect:
        if (toolType == 0) {
            if (bMouseLeftPress) {
                result =  Qt::ClosedHandCursor;
            } else {
                result =  Qt::OpenHandCursor;
            }
            resultCursor = QCursor(result);
        }
        break;
    case CSizeHandleRect::None:
        result =  Qt::ArrowCursor;
        resultCursor = QCursor(result);
        break;
//    default:
//        result =  Qt::ArrowCursor;
//        resultCursor = QCursor(result);
//        break;
    }

    return resultCursor;
#endif
    return  QCursor();
}

//qreal IDrawTool::getCursorRotation()
//{
//#if 0
//    qreal angle = 0;
//    CGraphicsView *pView = CManageViewSigleton::GetInstance()->getCurView();
//    if (pView != nullptr) {
//        CDrawScene *scene = static_cast<CDrawScene *>(pView->scene());
//        QList<QGraphicsItem *> allSelectItems = scene->selectedItems();
//        for (int i = allSelectItems.size() - 1; i >= 0; i--) {
//            if (allSelectItems.at(i)->zValue() == 0.0) {
//                allSelectItems.removeAt(i);
//                continue;
//            }
//            if (allSelectItems[i]->type() <= QGraphicsItem::UserType || allSelectItems[i]->type() >= EGraphicUserType::MgrType) {
//                allSelectItems.removeAt(i);
//            }
//        }

//        if (allSelectItems.size() >= 1) {
//            angle = allSelectItems.at(0)->rotation();
//        }
//    }
//    return  angle;
//#endif
//    return qreal();
//}

int IDrawTool::allowedMaxTouchPointCount()
{
    return 1;
}

bool IDrawTool::returnToSelectTool(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    Q_UNUSED(event)
    Q_UNUSED(pInfo)
    return true;
}

bool IDrawTool::autoSupUndoForCreatItem()
{
    return true;
}

bool IDrawTool::rasterItemToLayer(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo)
{
    Q_UNUSED(pInfo)

    return false;
}

void IDrawTool::sendToolEventToItem(CDrawToolEvent *event,
                                    ITERecordInfo *info,
                                    EChangedPhase phase)
{
    Q_UNUSED(event)
    Q_UNUSED(info)
    Q_UNUSED(phase)
}

bool IDrawTool::isFirstEvent()
{
    return _allITERecordInfo.count() == 1;
}

bool IDrawTool::isFinalEvent()
{
    return _allITERecordInfo.count() == 1;
}

int IDrawTool::getCurVaildActivedPointCount()
{
    int result = 0;
    foreach (auto it, _allITERecordInfo.values()) {
        if (it.eventLife != EDoNotthing) {
            ++result;
        }
    }
    return result;
}

void IDrawTool::changeStatusFlagTo(EStatus status)
{
    auto oldStatus = this->status();
    _status = status;
    if (oldStatus != _status) {
        emit statusChanged(oldStatus, _status);
    }
}

bool IDrawTool::isPressEventHandledByQt(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo)
{
    //1.节点node不用传递给qgraphics的qt框架 自己处理调整图元大小
    QGraphicsItem *pItem = pInfo->startPosItems.isEmpty() ? nullptr : pInfo->startPosItems.first();
    if (event->scene()->isBussizeHandleNodeItem(pItem)) {
        return false;
    }

    //2.如果不是节点，那么如果是代理的widget那么传递给qgraphics的qt框架，将事件传递给这个widget
    return (event->view()->activeProxWidget() != nullptr);
}

QTimer *IDrawTool::getTimerForDoubleCliked()
{
    if (s_timerForDoubleClike == nullptr) {
        s_timerForDoubleClike = new QTimer;
        s_timerForDoubleClike->setSingleShot(true);
    }
    return s_timerForDoubleClike;
}



bool IDrawTool::ITERecordInfo::isVaild() const
{
    return _isvaild;
}

bool IDrawTool::ITERecordInfo::hasMoved() const
{
    //return (_prePos != _startPos);

    //return (_prePos - _startPos).manhattanLength() > dApp->startDragDistance();

    return _moved;
}

QTime *IDrawTool::ITERecordInfo::getTimeHandle()
{
    return &_elapsedToUpdateTimeHandle;
}
