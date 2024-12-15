// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "drawtool.h"
#include "pagescene.h"
#include "pageview.h"
#include "pagecontext.h"
#include "cundoredocommand.h"
#include "pageitem.h"
#include "drawtoolfactory.h"
#include "attributemanager.h"
#include "toolsceneevent_p.h"


#include <QApplication>
#include <QDebug>
#include <QKeyEvent>
#include <QGraphicsSceneEvent>
#include <QGraphicsProxyWidget>
#include <QTimer>
#include <QToolButton>



class DrawTool::DrawToolBase_private
{
public:
    explicit DrawToolBase_private(DrawTool *q): _q(q),
        _button(new QToolButton),
        _cursor(Qt::ArrowCursor)
    {
        _button->setCheckable(true);
        _button->hide();
    }
    ~DrawToolBase_private()
    {
        s_timerForDoubleClike->deleteLater();
        s_timerForDoubleClike = nullptr;
    }
    int activePointsCount()
    {
        int count = 0;
        foreach (auto e, events) {
            if (e.eventlife == 2) {
                ++count;
            }
        }
        return count;
        //return std::count_if(events.begin(), events.end(), [ = ](QMap<int, S_ToolSceneEvents>::iterator e) {return (e.value().eventlife == 2);});
    }

    DrawTool *_q;

    DrawBoardToolMgr *_mgr = nullptr;
    QAbstractButton  *_button = nullptr;

    EStatus _status = EIdle;
    QCursor _cursor;
    bool    _isTouchSensitive = false;
    int     _touchSensitiveRadius = 10;
    int     _maxAllowTouchPoint = 10;
    bool    isContinue = false;
    bool    clearOnActived = false;
    bool    showAttriOnActived = true;

    //enum EEventLifeTo {EDoNotthing, EDoQtCoversion, ENormal};
    struct S_ToolSceneEvents {
        ToolSceneEvent pressEvent;
        ToolSceneEvent lastEvent;
        int  eventlife = 2;
        bool moved = false;
    };
    QMap<int, S_ToolSceneEvents> events;

    QTimer *getTimerForDoubleCliked()
    {
        if (s_timerForDoubleClike == nullptr) {
            s_timerForDoubleClike = new QTimer;
            s_timerForDoubleClike->setSingleShot(true);
        }
        return s_timerForDoubleClike;
    }
    QTimer *s_timerForDoubleClike = nullptr;

    friend class DrawTool;
};

DrawTool::DrawTool(QObject *parent): QObject(parent),
    DrawToolBase_d(new DrawToolBase_private(this))

{
    connect(this, &DrawTool::statusChanged, this, [ = ](EStatus oldStatus, EStatus nowStatus) {
        if (currentPage() != nullptr) {
            if ((oldStatus == EIdle && nowStatus == EReady)) {
                auto view = this->currentPage()->view();
                if (view != nullptr) {
                    if (view->activeProxDrawItem() != nullptr) {
                        auto activeItem = view->activeProxItem();
                        auto activeParentPageItem = view->activeProxDrawItem();
                        bool oldSelected = activeParentPageItem->isItemSelected();
                        activeItem->clearFocus();
                        bool nowSelected = activeParentPageItem->isItemSelected();

                        if (nowSelected) {
                            if (isSelectionClearOnActived()) {
                                view->pageScene()->clearSelections();
                            }
                        } else {
                            if (oldSelected) {
                                if (!isSelectionClearOnActived()) {
                                    QMetaObject::invokeMethod(view->pageScene(), [ = ]() {
                                        view->pageScene()->selectPageItem(activeParentPageItem);
                                    }, Qt::QueuedConnection);
                                }
                            }
                        }
                    } else  {
                        if (isSelectionClearOnActived())
                            view->pageScene()->clearSelections();
                    }
                }
            }
        }
        if (isAttributionsShowOnActived() && (oldStatus == EIdle && nowStatus == EReady)) {
            if (drawBoard() != nullptr) {
                drawBoard()->showAttributions(attributions(), AttributionManager::ToolActived);
            }
        }
        onStatusChanged(oldStatus, nowStatus);
    });
    connect(this, &SelectTool::toolManagerChanged, this, [ = ](DrawBoardToolMgr * old, DrawBoardToolMgr * cur) {
        Q_UNUSED(old)
        auto board = cur->drawBoard();
        connect(board, QOverload<Page *>::of(&DrawBoard::currentPageChanged), this, [ = ](Page * cur) {
            Q_UNUSED(cur)

            if (board->currentTool_p() == this) {
                //切换页面时，更新属性栏
                board->showAttributions(attributions());
            }

        });
    });
    connect(this, &DrawTool::toolManagerChanged, this, &DrawTool::onToolManagerChanged);
}

DrawTool::~DrawTool()
{
}

void DrawTool::doubleClickOnScene(ToolSceneEvent *event)
{
    Q_UNUSED(event)
}

void DrawTool::toolPressOnScene(ToolSceneEvent *event)
{
    if (event->isPressed()) {
        if (event->isNormalPressed() && tryConvertToDoubleClicked(event)) {
            return;
        }
        DrawToolBase_private::S_ToolSceneEvents info;
        event->itemsUnderPressedPos();
        event->topPageItemUnderPressedPos();
        event->topItemUnderPressedPos();
        info.pressEvent = *event;
        info.lastEvent = *event;
        if (d_DrawToolBase()->activePointsCount() >= maxTouchPoint()) {
            event->setAccepted(true);
            //超过可支持的点数了
            info.eventlife = ToolSceneEvent::EDoNotthing;
            d_DrawToolBase()->events.insert(event->uuid(), info);
        } else if (isPressEventHandledByQt(event)) {
            event->setAccepted(false);
            info.eventlife = ToolSceneEvent::EDoQtCoversion;
            qDebug() << "toolDoStart rInfo.eventLife = EDoQtCoversion id " << event->uuid();
            //只有真实的鼠标事件的调用才需要备份该事件信息,之后会调用toolDoUpdate,toolDoFinished，从而在其中通过UUID找到该事件信息以决定再将该事件传递给mouseEvent
            //触控操作在这里会被转成qt的鼠标事件直接去调用mouseEvent，不会再进入toolDoStart，toolDoUpdate,toolDoFinished，所以不需要备份保存信息
            //if (event->eventType() == ToolSceneEvent::EMouseEvent)
            {
                d_DrawToolBase()->events.insert(event->uuid(), info);
            }
        } else {
            d_DrawToolBase()->events.insert(event->uuid(), info);
            event->d_ToolSceneEvent()->_leftEventCount = d_DrawToolBase()->events.size();
            event->d_ToolSceneEvent()->_firstEvent = event;
            event->d_ToolSceneEvent()->_lastEvent = event;
            if (event->isFirstOne()) {
                changeStatusFlagTo(EWorking);
            }
            pressOnScene(event);
        }
    }
}

void DrawTool::toolMoveOnScene(ToolSceneEvent *event)
{
    auto findEvt = d_DrawToolBase()->events.find(event->uuid());
    if (findEvt != d_DrawToolBase()->events.end()) {
        DrawToolBase_private::S_ToolSceneEvents &es = findEvt.value();
        event->d_ToolSceneEvent()->_firstEvent = &(es.pressEvent);
        event->d_ToolSceneEvent()->_lastEvent = &(es.lastEvent);

        //判定是否移动过(根据工具不同移动的最小距离值不同可重载minMoveUpdateDistance)
        if (!es.moved) {
            int constDis = minMoveUpdateDistance();

            QPointF offset = event->pos(ToolSceneEvent::EViewportPos) -
                             es.pressEvent.pos(ToolSceneEvent::EViewportPos);
            int curDis = qRound(offset.manhattanLength());
            es.moved = (curDis >= constDis);
        }

        if (es.eventlife == ToolSceneEvent::EDoNotthing) {
            event->setAccepted(true);
        } else if (es.eventlife == ToolSceneEvent::EDoQtCoversion) {
            //0.如果开始事件是不被接受的（传递给了Qt框架），那么Update也应该不被接受（也应该传递给Qt框架）
            event->setAccepted(false);
        } else {
            if (es.moved) {
                event->d_ToolSceneEvent()->moved = true;
                moveOnScene(event);
            }
        }
        //记录这次事件
        DrawToolBase_private::S_ToolSceneEvents &events = d_DrawToolBase()->events[event->uuid()];
        events.lastEvent = *event;
    } else {
        if (!event->isPressed()) {
            setCursor(cursor());
            event->d_ToolSceneEvent()->moved = true;
            moveOnScene(event);
        }
    }
}

void DrawTool::toolReleaseOnScene(ToolSceneEvent *event)
{
    auto findEvt = d_DrawToolBase()->events.find(event->uuid());
    if (findEvt != d_DrawToolBase()->events.end()) {
        auto es = findEvt.value();
        event->d_ToolSceneEvent()->_firstEvent = &(findEvt.value().pressEvent);
        event->d_ToolSceneEvent()->_lastEvent = &(findEvt.value().lastEvent);
        event->d_ToolSceneEvent()->_leftEventCount = d_DrawToolBase()->events.count();
        event->d_ToolSceneEvent()->moved = findEvt.value().moved;

        if (es.eventlife == ToolSceneEvent::EDoQtCoversion) {
            //0.如果开始事件是不被接受的（传递给了Qt框架），那么Update也应该不被接受（也应该传递给Qt框架）
            event->setAccepted(false);
        } else {
            releaseOnScene(event);
            if (event->isFinalOne()) {
                changeStatusFlagTo(EReady);
                if (!isContinued()) {
                    if (currentPage() != nullptr && toolType() != selection) {
                        currentPage()->setCurrentTool(selection);
                    }
                }
            }
        }
        d_DrawToolBase()->events.erase(findEvt);
    }
}

DrawTool::EStatus DrawTool::status() const
{
    return d_DrawToolBase()->_status;
}

bool DrawTool::isEnable() const
{
    return d_DrawToolBase()->_status != EDisAbled;
}

int DrawTool::minMoveUpdateDistance()
{
    return qApp->startDragDistance();
}

DrawBoard *DrawTool::drawBoard() const
{
    if (d_DrawToolBase()->_mgr != nullptr)
        return d_DrawToolBase()->_mgr->drawBoard();
    return nullptr;
}

//void DrawToolBase::setDrawBoard(DrawBoard *board)
//{
//    if (board != _drawBoard) {
//        auto old = _drawBoard;
//        _drawBoard = board;
//        emit boardChanged(old, _drawBoard);
//    }
//}

Page *DrawTool::currentPage() const
{
    if (drawBoard() != nullptr) {
        return drawBoard()->currentPage();
    }
    return nullptr;
}

QCursor DrawTool::cursor() const
{
    return d_DrawToolBase()->_cursor;
}

void DrawTool::setCursor(const QCursor &curosr)
{
    d_DrawToolBase()->_cursor = curosr;

    if (currentPage() != nullptr) {
        currentPage()->setDrawCursor(curosr);
    }
}

int DrawTool::maxTouchPoint() const
{
    return d_DrawToolBase()->_maxAllowTouchPoint;
}

void DrawTool::setMaxTouchPoint(int num)
{
    d_DrawToolBase()->_maxAllowTouchPoint = qMax(qMin(num, 10), 0);
}

QList<int> DrawTool::currentActivedPointIds() const
{
    QList<int> list;
    for (auto e = d_DrawToolBase()->events.begin(); e != d_DrawToolBase()->events.end(); ++e) {
        list.append(e.key());
    }
    return list;
}

bool DrawTool::isContinued() const
{
    return d_DrawToolBase()->isContinue;
}

void DrawTool::setContinued(bool b)
{
    d_DrawToolBase()->isContinue = b;
}

DrawBoardToolMgr *DrawTool::toolManager() const
{
    return d_DrawToolBase()->_mgr;
}
void DrawTool_SetToolManagerHelper(DrawTool *tool, DrawBoardToolMgr *pManager)
{
    if (pManager == nullptr)
        return;

    if (tool->d_DrawToolBase()->_mgr == pManager) {
        return;
    }
    auto old = tool->d_DrawToolBase()->_mgr;
    tool->d_DrawToolBase()->_mgr = pManager;
    emit tool->toolManagerChanged(old, tool->d_DrawToolBase()->_mgr);
}
void DrawTool::setToolManager(DrawBoardToolMgr *pManager)
{
    if (pManager == nullptr)
        return;

    if (d_DrawToolBase()->_mgr == pManager) {
        return;
    }

    pManager->addTool(this);

//    auto old = d_DrawToolBase()->_mgr;

//    if (d_DrawToolBase()->_mgr != nullptr) {
//        //_mgr->removeTool(this);
//    }

//    d_DrawToolBase()->_mgr = pManager;

//    if (d_DrawToolBase()->_mgr != nullptr) {
//        //_mgr->addTool(this);
//    }

//    emit toolManagerChanged(old, d_DrawToolBase()->_mgr);
}

QAbstractButton *DrawTool::toolButton() const
{
    return d_DrawToolBase()->_button;
}

void DrawTool::onStatusChanged(EStatus oldStatus, EStatus nowStatus)
{

}

void DrawTool::onToolManagerChanged(DrawBoardToolMgr *old, DrawBoardToolMgr *now)
{
    Q_UNUSED(old)
    if (old != nullptr) {
        auto board = old->drawBoard();
        if (board != nullptr)
            disconnect(board, QOverload<Page *>::of(&DrawBoard::currentPageChanged), this, &DrawTool::onCurrentPageChanged);
    }
    if (now != nullptr) {
        auto board = now->drawBoard();
        if (board != nullptr) {
            connect(board, QOverload<Page *>::of(&DrawBoard::currentPageChanged), this, &DrawTool::onCurrentPageChanged);
            onCurrentPageChanged(board->currentPage());
        }
    }
}

void DrawTool::setEnable(bool b)
{
    toolButton()->setEnabled(b);
    changeStatusFlagTo(b ? EIdle : EDisAbled);
}

bool DrawTool::activeTool()
{
    toolButton()->setChecked(true);
    return true;
}

//QAbstractButton *DrawTool::initToolButton()
//{
//    return nullptr;
//}

SAttrisList DrawTool::attributions()
{
    return QList<int>() << 0;
}

void DrawTool::setAttributionVar(int attri, const QVariant &var, int phase, bool autoCmdStack)
{
    Q_UNUSED(attri)
    Q_UNUSED(var)
    Q_UNUSED(phase)
    Q_UNUSED(autoCmdStack)
}

bool DrawTool::blockPageBeforeOutput(Page *page)
{
    return false;
}

void DrawTool::clearPointRecording()
{
    d_DrawToolBase()->events.clear();
}

void DrawTool::onCurrentPageChanged(Page *newPage)
{
    Q_UNUSED(newPage)
}

bool DrawTool::isWorking()
{
    return !d_DrawToolBase()->events.isEmpty();
}

void DrawTool::interrupt()
{
    clearPointRecording();
    if (status() == EWorking)
        changeStatusFlagTo(EReady);
}

void DrawTool::setTouchSensitiveRadius(int sensitiveRadius)
{
    d_DrawToolBase()->_touchSensitiveRadius = sensitiveRadius;
}

int DrawTool::touchSensitiveRadius() const
{
    return d_DrawToolBase()->_touchSensitiveRadius;
}

void DrawTool::setClearSelectionOnActived(bool b)
{
    d_DrawToolBase()->clearOnActived = b;
}

bool DrawTool::isSelectionClearOnActived() const
{
    return d_DrawToolBase()->clearOnActived;
}

void DrawTool::setAttributionShowOnActived(bool b)
{
    d_DrawToolBase()->showAttriOnActived = b;
}

bool DrawTool::isAttributionsShowOnActived() const
{
    return d_DrawToolBase()->showAttriOnActived;
}

SAttri DrawTool::defaultAttriVar(int attri) const
{
    return SAttri(attri, drawBoard()->pageAttriVariant(drawBoard()->currentPage(), attri));
}

void DrawTool::drawMore(QPainter *painter, const QRectF &rect, PageScene *scene)
{
    //注意painter是在viewport的坐标系,绘制时需要转换
    Q_UNUSED(painter)
    Q_UNUSED(rect)
    Q_UNUSED(scene)
}

void DrawTool::changeStatusFlagTo(EStatus status)
{
    auto oldStatus = this->status();
    d_DrawToolBase()->_status = status;
    if (oldStatus != d_DrawToolBase()->_status) {
        emit statusChanged(oldStatus, d_DrawToolBase()->_status);
    }
}

bool DrawTool::isPressEventHandledByQt(ToolSceneEvent *event)
{
    //1.节点node不用传递给qgraphics的qt框架 自己处理调整图元大小
    auto itemsUnderPos = event->itemsUnderPressedPos();
    QGraphicsItem *pItem = itemsUnderPos.isEmpty() ? nullptr : itemsUnderPos.first();
    if (event->scene()->isHandleNode(pItem)) {
        return false;
    }

    //2.如果不是节点，那么如果是代理的widget那么传递给qgraphics的qt框架，将事件传递给这个widget
    return (event->view()->activeProxWidget() != nullptr);
}

void DrawTool::contextMenuEvent(ToolSceneEvent *event)
{
    Q_UNUSED(event)
}

bool DrawTool::tryConvertToDoubleClicked(ToolSceneEvent *event)
{
    static const int intervalMs = 250;
    if (event->eventType() == ToolSceneEvent::ETouchEvent) {
        static QPointF prePos;
        //qWarning() << "1111111111111111111";
        //touch触控判定是否是双击
        if (!d_DrawToolBase()->getTimerForDoubleCliked()->isActive()) {
            //qWarning() << "22222222222222222222";
            prePos = event->pos();
            d_DrawToolBase()->getTimerForDoubleCliked()->start(intervalMs);
        } else {
            //qWarning() << "33333333333333333333";
            //判定移动的幅度很小
            QRectF rectf(prePos - QPointF(10, 10), QSizeF(20, 20));
            if (rectf.contains(event->pos())) {
                //qWarning() << "44444444444444444444";
                DrawToolBase_private::S_ToolSceneEvents info;
//                event->itemsUnderPos();
//                event->topPageItem();
                info.pressEvent = *event;
                d_DrawToolBase()->events.insert(event->uuid(), info);
                event->d_ToolSceneEvent()->_firstEvent = event;
                //event->d_ToolSceneEvent()->_lastEvent = event;
                if (isPressEventHandledByQt(event)) {
                    //qWarning() << "55555555555555555555";
                    //触控事件的双击如果需要传递给Qt去处理,那么需要清理掉事件
                    event->setAccepted(false);
                    interrupt();
                } else {
                    //qWarning() << "66666666666666666666";
                    doubleClickOnScene(event);
                }
                //qWarning() << "77777777777777777777";
                d_DrawToolBase()->getTimerForDoubleCliked()->stop();

                return true;
            }
        }
    } else if (event->eventType() == ToolSceneEvent::EMouseEvent) {
        QEvent::Type tp = event->orgQtEvent()->type();
        if ((tp == QEvent::MouseButtonDblClick || tp == QEvent::GraphicsSceneMouseDoubleClick) && event->mouseButtons() == Qt::LeftButton) {

            DrawToolBase_private::S_ToolSceneEvents info;
            event->itemsUnderPressedPos();
            event->topPageItemUnderPressedPos();
            event->topItemUnderPressedPos();
            info.pressEvent = *event;
            d_DrawToolBase()->events.insert(event->uuid(), info);
            event->d_ToolSceneEvent()->_firstEvent = event;
            //event->d_ToolSceneEvent()->_lastEvent = event;
            if (isPressEventHandledByQt(event)) {
                event->setAccepted(false);
                d_DrawToolBase()->events[event->uuid()].eventlife = ToolSceneEvent::EDoQtCoversion;
            } else {
                doubleClickOnScene(event);
            }
            return true;
        }
    }
    return false;
}
