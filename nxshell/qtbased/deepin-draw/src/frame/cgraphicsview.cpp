// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cgraphicsview.h"
#include "drawshape/cdrawparamsigleton.h"
#include "drawshape/cshapemimedata.h"
#include "bzItems/cgraphicsitem.h"
#include "drawshape/globaldefine.h"
#include "drawshape/cdrawscene.h"
//#include "cundocommands.h"
#include "widgets/ctextedit.h"
#include "bzItems/cgraphicspolygonitem.h"
#include "bzItems/cgraphicspolygonalstaritem.h"
#include "drawshape/cdrawscene.h"
#include "bzItems/cgraphicsrectitem.h"
#include "bzItems/cgraphicsellipseitem.h"
#include "bzItems/cgraphicslineitem.h"
#include "bzItems/cgraphicstriangleitem.h"
#include "bzItems/cgraphicspolygonitem.h"
#include "bzItems/cgraphicspolygonalstaritem.h"
#include "bzItems/cgraphicstextitem.h"
#include "bzItems/cgraphicspenitem.h"
#include "bzItems/cpictureitem.h"
#include "drawTools/cpicturetool.h"
#include "drawItems/cgraphicsitemselectedmgr.h"
#include "frame/cviewmanagement.h"
#include "frame/cgraphicsview.h"
#include "drawTools/cdrawtoolmanagersigleton.h"
#include "../application.h"
#include "cundocommands.h"
#include "cundoredocommand.h"
#include "mainwindow.h"
#include "cgraphicsitemevent.h"
#include "progresslayout.h"
#include "filehander.h"

#include <QTimer>
#include <DMenu>
#include <DFileDialog>
#include <DDialog>

#include <QAction>
#include <QWheelEvent>
#include <QClipboard>
#include <QApplication>
#include <QDebug>
#include <QStandardPaths>
#include <QUndoStack>
#include <QRectF>
#include <QPainter>
#include <QDesktopWidget>
#include <QClipboard>
#include <QMessageBox>
#include <QWindow>
#include <QScreen>
#include <qscrollbar.h>
#include <QTouchEvent>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneWheelEvent>

//升序排列用
//static bool zValueSortASC(QGraphicsItem *info1, QGraphicsItem *info2)
//{
//    return info1->zValue() < info2->zValue();
//}

//水平等间距对齐升序排列
static bool xValueSortDES(QGraphicsItem *info1, QGraphicsItem *info2)
{
    return info1->sceneBoundingRect().left() < info2->sceneBoundingRect().left();
}

//垂直等间距对齐升序排列
static bool yValueSortDES(QGraphicsItem *info1, QGraphicsItem *info2)
{
    return info1->sceneBoundingRect().top() < info2->sceneBoundingRect().top();
}

extern bool adaptImgPosAndRect(PageScene *pScene, const QString &imgName, const QImage &img, QPointF &pos, QRectF &rect, int &choice);

PageView::PageView(Page *parentPage)
    : DGraphicsView(parentPage)
    , m_scale(1)
    , m_isShowContext(true)
    , m_isStopContinuousDrawing(false)
{
    setOptimizationFlags(IndirectPainting);
    m_pUndoStack = new QUndoStack(this);
    //设置撤销还原最大步数
    m_pUndoStack->setUndoLimit(30);
    CUndoRedoCommand::setUndoRedoStack(m_pUndoStack, this);

    initContextMenu();
    initContextMenuConnection();

    //文字右键菜单初始化
    initTextContextMenu();
    initTextContextMenuConnection();

    initConnection();

    viewport()->installEventFilter(this);

    //初始化后设置自身为焦点
    QTimer::singleShot(200, nullptr, [ = ]() {
        this->setFocus();
    });

    this->acceptDrops();
    this->setAttribute(Qt::WA_AcceptTouchEvents);
    viewport()->setAttribute(Qt::WA_AcceptTouchEvents);

    viewport()->grabGesture(Qt::PinchGesture);

    this->setAlignment(Qt::AlignCenter);
    this->setRenderHint(QPainter::Antialiasing);   //设置反走样

    //自动设置滚动条
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

PageView::~PageView()
{
}

Page *PageView::page() const
{
    return qobject_cast<Page *>(parent());
}

void PageView::zoomOut(EScaleCenter center, const QPoint &viewPos)
{
    //保证精度为小数点后两位
    qreal current_scale = qRound(m_scale * 100) / 100.0;

    qreal inc = 0.1;

    if (qFuzzyIsNull(current_scale - 20.0) || current_scale > 20.0) {
        inc = 0;
    } else {
        if (current_scale > 2.0 || qFuzzyIsNull(current_scale - 2.0)) {
            inc = 1.0;
        } else {
            if (current_scale > 1.0 || qFuzzyIsNull(current_scale - 1.0)) {
                inc = 0.1;
            } else {
                if (current_scale > 0.1 || qFuzzyIsNull(current_scale - 0.1)) {
                    inc = 0.01;
                } else {
                    inc = 0;
                }
            }
        }
    }
    current_scale += inc;

    scale(current_scale, center, viewPos);
}

void PageView::zoomIn(EScaleCenter center, const QPoint &viewPos)
{
    //保证精度为小数点后两位
    qreal current_scale = qRound(m_scale * 100) / 100.0;
    qreal inc = 0.1;
    if (qFuzzyIsNull(current_scale - 20.0) || current_scale > 20.0) {
        inc = 1.0;
    } else {
        if (current_scale > 2.0) {
            inc = 1.0;
        } else {
            if (current_scale > 1.0) {
                inc = 0.1;
            } else {
                if (current_scale > 0.1) {
                    inc = 0.01;
                } else {
                    inc = 0;
                }
            }
        }
    }
    current_scale -= inc;
    scale(current_scale, center, viewPos);
}

void PageView::scale(qreal scale, EScaleCenter center, const QPoint &viewPos)
{
    qreal multiple = scale / m_scale;

    QPoint centerViewPos = viewPos;
    switch (center) {
    case EViewCenter:
        centerViewPos = viewport()->rect().center();
        break;
    case ESceneCenter:
        centerViewPos = mapFromScene(sceneRect().center());
        break;
    case EMousePos:
        centerViewPos = viewport()->mapFromGlobal(QCursor::pos());
        break;
    default:
        centerViewPos = viewPos;
        break;
    }
    scaleWithCenter(multiple, centerViewPos);
}

qreal PageView::getScale()
{
    return m_scale;
}

void PageView::scaleWithCenter(qreal factor, const QPoint viewPos)
{
    qreal wantedTotalScaled = m_scale * factor;

    if (wantedTotalScaled < 0.1) {
        wantedTotalScaled = 0.1;
        factor = wantedTotalScaled / m_scale;
    } else if (wantedTotalScaled > 20) {
        wantedTotalScaled = 20;
        factor = wantedTotalScaled / m_scale;
    }


    //最最最完美的方案！！！
    QPoint centerViewPos = viewPos.isNull() ? viewport()->mapFromGlobal(QCursor::pos()) : viewPos;

    QPointF targetScenePos = mapToScene(centerViewPos);
    ViewportAnchor oldAnchor = this->transformationAnchor();
    setTransformationAnchor(QGraphicsView::NoAnchor);

    QTransform matrix = transform();
    matrix.translate(targetScenePos.x(), targetScenePos.y())
    .scale(factor, factor)
    .translate(-targetScenePos.x(), -targetScenePos.y());

    setTransform(matrix);

    setTransformationAnchor(oldAnchor);

    //保存缩放值
    m_scale *= factor;
    //getDrawParam()->setScale(m_scale);
    emit signalSetScale(m_scale);

    if (drawScene() != nullptr)
        drawScene()->selectGroup()->updateBoundingRect();
}

void PageView::wheelEvent(QWheelEvent *event)
{
    Q_UNUSED(event)
    if (activeProxItem() != nullptr) {
        auto proxyItem = activeProxItem();
        QPointF scenePos = this->mapToScene(this->viewport()->mapFromGlobal(QCursor::pos()));
        if (proxyItem->sceneBoundingRect().contains(scenePos)) {
            QPointF p = proxyItem->mapFromScene(scenePos);
            QGraphicsSceneWheelEvent gsEvent(QEvent::GraphicsSceneWheel);
            gsEvent.setPos(p);
            gsEvent.setScenePos(scenePos);
            gsEvent.setScreenPos(QCursor::pos());
            gsEvent.setButtons(event->buttons());
            gsEvent.setModifiers(event->modifiers());
            gsEvent.setDelta(event->delta());
            gsEvent.setOrientation(event->orientation());
            drawScene()->sendEvent(proxyItem, &gsEvent);
            return;
        }
    }
    PageView *pCurView   = this;
    int            delayValue = event->delta();
    if (pCurView != nullptr) {
        if (event->modifiers() == Qt::NoModifier) {
            //滚动view的垂直scrollbar
            int curValue = pCurView->verticalScrollBar()->value();
            pCurView->verticalScrollBar()->setValue(curValue - delayValue / 12);
        } else if (event->modifiers() == Qt::ShiftModifier) {
            //滚动view的水平scrollbar
            int curValue = pCurView->horizontalScrollBar()->value();
            pCurView->horizontalScrollBar()->setValue(curValue - delayValue / 12);
        } else if (event->modifiers()& Qt::ControlModifier) {
            //当前工具正在绘制时不进行放大缩小
            IDrawTool *currentTool = page()->currentTool_p();
            if (currentTool != nullptr && currentTool->status() == IDrawTool::EWorking)
                return;

            //如果按住CTRL那么就是放大缩小
            if (event->delta() > 0) {
                pCurView->zoomOut(PageView::EMousePos);
            } else {
                pCurView->zoomIn(PageView::EMousePos);
            }
        }
    }
    //QGraphicsView::wheelEvent(event);
}

void PageView::initContextMenu()
{
    m_contextMenu = new CMenu(this);

    //CMenu enterEvent激活全部action
    m_layerMenu = new DMenu(tr("Layer"), this);

    m_cutAct = new QAction(tr("Cut"), this);
    m_contextMenu->addAction(m_cutAct);
    m_cutAct->setShortcut(QKeySequence::Cut);
    this->addAction(m_cutAct);

    m_copyAct = new QAction(tr("Copy"), this);
    m_contextMenu->addAction(m_copyAct);
    m_copyAct->setShortcut(QKeySequence::Copy);
    this->addAction(m_copyAct);

    m_pasteAct = new QAction(tr("Paste"), this);
    m_pasteActShortCut = new QAction(this);
    m_contextMenu->addAction(m_pasteAct);
    m_pasteActShortCut->setShortcut(QKeySequence::Paste);
    this->addAction(m_pasteAct);
    this->addAction(m_pasteActShortCut);

    m_selectAllAct = new QAction(tr("Select All"), this);
    m_contextMenu->addAction(m_selectAllAct);
    m_selectAllAct->setShortcut(QKeySequence::SelectAll);
    this->addAction(m_selectAllAct);

    m_contextMenu->addSeparator();

    m_deleteAct = new QAction(tr("Delete"), this);
    m_contextMenu->addAction(m_deleteAct);
    m_deleteAct->setShortcut(QKeySequence::Delete);
    this->addAction(m_deleteAct);

    //m_undoAct = m_pUndoStack->createUndoAction(this, tr("Undo"));
    m_undoAct = new QAction(tr("Undo"), this);
    m_undoAct->setEnabled(m_pUndoStack->canUndo());
    connect(m_pUndoStack, SIGNAL(canUndoChanged(bool)),
            m_undoAct, SLOT(setEnabled(bool)));
    m_contextMenu->addAction(m_undoAct);
    m_undoAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));
    this->addAction(m_undoAct);

    //m_redoAct = m_pUndoStack->createRedoAction(this, tr("Redo"));
    m_redoAct = new QAction(tr("Redo"), this);
    m_redoAct->setEnabled(m_pUndoStack->canRedo());
    connect(m_pUndoStack, SIGNAL(canRedoChanged(bool)),
            m_redoAct, SLOT(setEnabled(bool)));
    m_contextMenu->addAction(m_redoAct);
    m_redoAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));
    this->addAction(m_redoAct);
    m_contextMenu->addSeparator();

    m_oneLayerUpAct = new QAction(tr("Raise Layer"), this);
    m_layerMenu->addAction(m_oneLayerUpAct);
    m_oneLayerUpAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_BracketRight));
    this->addAction(m_oneLayerUpAct);

    m_oneLayerDownAct = new QAction(tr("Lower Layer"), this);
    m_layerMenu->addAction(m_oneLayerDownAct);
    m_oneLayerDownAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_BracketLeft));
    this->addAction(m_oneLayerDownAct);

    m_bringToFrontAct = new QAction(tr("Layer to Top"), this);
    m_layerMenu->addAction(m_bringToFrontAct);
    m_bringToFrontAct->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_BracketRight));
    this->addAction(m_bringToFrontAct);

    m_sendTobackAct = new QAction(tr("Layer to Bottom"), this);
    m_layerMenu->addAction(m_sendTobackAct);
    m_sendTobackAct->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_BracketLeft));
    this->addAction(m_sendTobackAct);

//    m_cutScence = new QAction(this);
//    QList<QKeySequence> shortcuts;
//    shortcuts.append(QKeySequence(Qt::Key_Return));
//    shortcuts.append(QKeySequence(Qt::Key_Enter));
//    m_cutScence->setShortcuts(shortcuts);
//    this->addAction(m_cutScence);

    m_viewZoomInAction = new QAction(this);
    m_viewZoomInAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));
    this->addAction(m_viewZoomInAction);

    m_viewZoomOutAction = new QAction(this);
    m_viewZoomOutAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));
    this->addAction(m_viewZoomOutAction);

    // Qt 无法直接使用 ctrl + (+/=) 这个按键组合
    m_viewZoomOutAction1 = new QAction(this);
    m_viewZoomOutAction1->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Equal));
    this->addAction(m_viewZoomOutAction1);

    m_viewOriginalAction = new QAction(this);
    m_viewOriginalAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    this->addAction(m_viewOriginalAction);

    m_group = new QAction(tr("Group"), this);
    m_group->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
    this->addAction(m_group);
    m_contextMenu->addAction(m_group);

    m_unGroup = new QAction(tr("Ungroup"), this);
    m_unGroup->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_G));
    this->addAction(m_unGroup);
    m_contextMenu->addAction(m_unGroup);

    // 右键菜单添加对齐方式
    m_alignMenu = new DMenu(tr("Align"), this);
    m_contextMenu->addMenu(m_alignMenu);

    m_itemsLeftAlign = new QAction(tr("Align left"), this); //左对齐
    m_itemsLeftAlign->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_L));
    m_alignMenu->addAction(m_itemsLeftAlign);
    this->addAction(m_itemsLeftAlign);

    m_itemsHCenterAlign = new QAction(tr("Horizontal centers"), this); //水平居中对齐
    m_itemsHCenterAlign->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_H));
    m_alignMenu->addAction(m_itemsHCenterAlign);
    this->addAction(m_itemsHCenterAlign);

    m_itemsRightAlign = new QAction(tr("Align right"), this); //右对齐
    m_itemsRightAlign->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));
    m_alignMenu->addAction(m_itemsRightAlign);
    this->addAction(m_itemsRightAlign);

    m_itemsTopAlign = new QAction(tr("Align top"), this); //顶对齐
    m_itemsTopAlign->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_T));
    m_alignMenu->addAction(m_itemsTopAlign);
    this->addAction(m_itemsTopAlign);

    m_itemsVCenterAlign = new QAction(tr("Vertical centers"), this); //垂直居中对齐
    m_itemsVCenterAlign->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_V));
    m_alignMenu->addAction(m_itemsVCenterAlign);
    this->addAction(m_itemsVCenterAlign);

    m_itemsBottomAlign = new QAction(tr("Align bottom"), this); //底对齐
    m_itemsBottomAlign->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_B));
    m_alignMenu->addAction(m_itemsBottomAlign);
    this->addAction(m_itemsBottomAlign);

    m_itemsHEqulSpaceAlign = m_alignMenu->addAction(tr("Distribute horizontal space")); //水平等间距对齐
    m_itemsHEqulSpaceAlign->setObjectName("DistributeHorizontalSpace");
    m_itemsVEqulSpaceAlign = m_alignMenu->addAction(tr("Distribute vertical space")); //垂直等间距对齐
    m_itemsVEqulSpaceAlign->setObjectName("DistributeVerticalSpace");

    // 添加对齐菜单
    m_contextMenu->addMenu(m_layerMenu);

//    QAction *pAction = new QAction("RasterToLayer");
//    m_contextMenu->addAction(pAction);
//    connect(pAction, &QAction::triggered, this, [ = ]() {
//        drawScene()->selectGroup()->rasterToSelfLayer(true);
//    });
}

void PageView::initContextMenuConnection()
{
    connect(m_cutAct, &QAction::triggered, this, &PageView::slotOnCut);
    connect(m_copyAct, &QAction::triggered, this, &PageView::slotOnCopy);
    connect(m_pasteAct, &QAction::triggered, this, [ = ]() {
        slotOnPaste(false);
    });
    connect(m_pasteActShortCut, &QAction::triggered, this, [ = ]() {
        slotOnPaste();
    });
    connect(m_selectAllAct, &QAction::triggered, this, &PageView::slotOnSelectAll);
    connect(m_deleteAct, &QAction::triggered, this, &PageView::slotOnDelete);
    connect(m_bringToFrontAct, &QAction::triggered, this, &PageView::slotBringToFront);
    connect(m_sendTobackAct, &QAction::triggered, this, &PageView::slotSendTobackAct);
    connect(m_oneLayerUpAct, &QAction::triggered, this, &PageView::slotOneLayerUp);
    connect(m_oneLayerDownAct, &QAction::triggered, this, &PageView::slotOneLayerDown);
    //connect(m_cutScence, &QAction::triggered, this, &CGraphicsView::slotDoCutScene);

    connect(m_viewZoomInAction, &QAction::triggered, this, &PageView::slotViewZoomIn);
    connect(m_viewZoomOutAction, &QAction::triggered, this, &PageView::slotViewZoomOut);
    connect(m_viewZoomOutAction1, &QAction::triggered, this, &PageView::slotViewZoomOut);
    connect(m_viewOriginalAction, &QAction::triggered, this, &PageView::slotViewOrignal);

    connect(m_undoAct, &QAction::triggered, this, [ = ] {
        CHECK_MOSUEACTIVE_RETURN
        //记录出栈信息
        userActionCount--;
        m_pUndoStack->undo();
        drawScene()->pageContext()->setDirty(isModified());
    });
    connect(m_redoAct, &QAction::triggered, this, [ = ] {
        CHECK_MOSUEACTIVE_RETURN
        //记录入栈信息
        userActionCount++;
        m_pUndoStack->redo();
        drawScene()->pageContext()->setDirty(isModified());
    });

    connect(m_group, &QAction::triggered, this, [ = ] {
        CHECK_CURRENTTOOL_RETURN(this)
        CGraphicsItem *pBaseItem = drawScene()->selectGroup()->getLogicFirst();
        drawScene()->creatGroup(QList<CGraphicsItem *>(), CGraphicsItemGroup::ENormalGroup,
                                true, pBaseItem, true);
    });
    connect(m_unGroup, &QAction::triggered, this, [ = ] {
        CHECK_CURRENTTOOL_RETURN(this)
        drawScene()->cancelGroup(nullptr, true);
    });

    // 连接图元对齐信号
    connect(m_itemsLeftAlign, &QAction::triggered, this, [ = ] {
        this->updateSelectedItemsAlignment(Qt::AlignLeft);
    });
    connect(m_itemsHCenterAlign, &QAction::triggered, this, [ = ] {
        this->updateSelectedItemsAlignment(Qt::AlignHCenter);
    });
    connect(m_itemsRightAlign, &QAction::triggered, this, [ = ] {
        this->updateSelectedItemsAlignment(Qt::AlignRight);
    });
    connect(m_itemsTopAlign, &QAction::triggered, this, [ = ] {
        this->updateSelectedItemsAlignment(Qt::AlignTop);
    });
    connect(m_itemsVCenterAlign, &QAction::triggered, this, [ = ] {
        this->updateSelectedItemsAlignment(Qt::AlignVCenter);
    });
    connect(m_itemsBottomAlign, &QAction::triggered, this, [ = ] {
        this->updateSelectedItemsAlignment(Qt::AlignBottom);
    });
    connect(m_itemsVEqulSpaceAlign, &QAction::triggered, this, [ = ] {
        // [0] 获取选中的图元
        // QList<CGraphicsItem *> allitems = getSelectedValidItems();
        auto currScene = dynamic_cast<PageScene *>(scene());
        QList<CGraphicsItem *> allitems = currScene->selectGroup()->items();

        // [1] 图元个数大于3个才可以进行对齐
        if (allitems.size() < 3)
        {
            return ;
        }

        // [2] 对图元X进行从小到大排序
        std::sort(allitems.begin(), allitems.end(), yValueSortDES);

        // [3] 统计所有图元占有的高度
        qreal sum_items_height = 0;
        for (int i = 0; i < allitems.size(); i++)
        {
            sum_items_height += allitems.at(i)->sceneBoundingRect().height();
        }

        // [3] 计算每两个之间的间隔距离
        auto curScene = dynamic_cast<PageScene *>(scene());
        QRectF scence_BR = curScene->selectGroup()->sceneBoundingRect();

        // [4] 用于记录保存图元的位置，便于撤销和返回
        QMap<CGraphicsItem *, QPointF> startPos;
        QMap<CGraphicsItem *, QPointF> endPos;

        if (sum_items_height > scence_BR.height())
        {
            // [5] 按照相邻进行移动位置
            for (int i = 1; i < allitems.size(); i++) {
                startPos.insert(allitems.at(i), allitems.at(i)->sceneBoundingRect().topLeft());
                QPointF endPoint(allitems.at(i)->sceneBoundingRect().left()
                                 , allitems.at(i - 1)->sceneBoundingRect().bottom());
                allitems.at(i)->move(allitems.at(i)->sceneBoundingRect().topLeft(), endPoint);
                endPos.insert(allitems.at(i), allitems.at(i)->sceneBoundingRect().topLeft());
            }
        } else
        {
            qreal space_height = (scence_BR.height() - sum_items_height)
                                 / (allitems.size() - 1);

            // [6] 按照y值进行移动位置
            for (int i = 1; i < allitems.size() - 1; i++) {
                startPos.insert(allitems.at(i), allitems.at(i)->sceneBoundingRect().topLeft());
                QPointF endPoint(allitems.at(i)->sceneBoundingRect().left()
                                 , allitems.at(i - 1)->sceneBoundingRect().bottom());
                endPoint = endPoint + QPointF(0, space_height);
                allitems.at(i)->move(allitems.at(i)->sceneBoundingRect().topLeft(), endPoint);
                endPos.insert(allitems.at(i), allitems.at(i)->sceneBoundingRect().topLeft());
            }
        }

        // [7] 设置出入栈
        QUndoCommand *addCommand = new CItemsAlignCommand(static_cast<PageScene *>(scene()), startPos, endPos);
        pushUndoStack(addCommand);
    });

    connect(m_itemsHEqulSpaceAlign, &QAction::triggered, this, [ = ] {
        // [0] 获取选中的图元
        // QList<CGraphicsItem *> allitems = getSelectedValidItems();
        auto currScene = dynamic_cast<PageScene *>(scene());
        QList<CGraphicsItem *> allitems = currScene->selectGroup()->items();

        // [1] 图元个数大于3个才可以进行对齐
        if (allitems.size() < 3)
        {
            return ;
        }

        // [2] 对图元X进行从小到大排序
        std::sort(allitems.begin(), allitems.end(), xValueSortDES);

        // [3] 统计所有图元占有的宽度
        qreal sum_items_width = 0;
        for (int i = 0; i < allitems.size(); i++)
        {
            sum_items_width += allitems.at(i)->sceneBoundingRect().width();
        }

        // [4] 计算每两个之间的间隔距离
        auto curScene = dynamic_cast<PageScene *>(scene());
        QRectF scence_BR = curScene->selectGroup()->sceneBoundingRect();

        // [5] 用于记录保存图元的位置，便于撤销和返回
        QMap<CGraphicsItem *, QPointF> startPos;
        QMap<CGraphicsItem *, QPointF> endPos;

        if (sum_items_width > scence_BR.width())
        {
            // [6] 按照相邻进行移动位置
            for (int i = 1; i < allitems.size(); i++) {
                startPos.insert(allitems.at(i), allitems.at(i)->sceneBoundingRect().topLeft());
                QPointF endPoint(allitems.at(i - 1)->sceneBoundingRect().right()
                                 , allitems.at(i)->sceneBoundingRect().top());
                allitems.at(i)->move(allitems.at(i)->sceneBoundingRect().topLeft(), endPoint);
                endPos.insert(allitems.at(i), allitems.at(i)->sceneBoundingRect().topLeft());
            }
        } else
        {
            qreal space_width = (scence_BR.width() - sum_items_width)
                                / (allitems.size() - 1);
            // [7] 按照x值的间隔进行移动位置
            for (int i = 1; i < allitems.size() - 1; i++) {
                startPos.insert(allitems.at(i), allitems.at(i)->sceneBoundingRect().topLeft());
                QPointF endPoint(allitems.at(i - 1)->sceneBoundingRect().right()
                                 , allitems.at(i)->sceneBoundingRect().top());
                endPoint = endPoint + QPointF(space_width, 0);
                allitems.at(i)->move(allitems.at(i)->sceneBoundingRect().topLeft(), endPoint);
                endPos.insert(allitems.at(i), allitems.at(i)->sceneBoundingRect().topLeft());
            }
        }

        // [8] 设置出入栈
        QUndoCommand *addCommand = new CItemsAlignCommand(static_cast<PageScene *>(scene()), startPos, endPos);
        pushUndoStack(addCommand);
    });

    // 连接鼠标hovered子菜单刷新图层状态
    connect(m_contextMenu, &QMenu::hovered, this, [ = ](QAction * action) {
        if (action->text() == tr("Layer") && m_layerMenu->isHidden()) {
            // 此处由于图层菜单显示较慢，以下代码会重复执行几次，待后期进一步优化
//            bool layerUp = canLayerUp();
//            m_oneLayerUpAct->setEnabled(layerUp);
//            m_bringToFrontAct->setEnabled(layerUp);

//            bool layerDown = canLayerDown();
//            m_oneLayerDownAct->setEnabled(layerDown);
//            m_sendTobackAct->setEnabled(layerDown);
        }
    });
}

void PageView::initTextContextMenu()
{
    m_textMenu = new DMenu(this);

    m_textCutAction = new QAction(tr("Cut"), m_textMenu);
    m_textCopyAction = new QAction(tr("Copy"), m_textMenu);
    m_textPasteAction = new QAction(tr("Paste"), m_textMenu);
    m_textSelectAllAction = new QAction(tr("Select All"), m_textMenu);
    m_textUndoAct = new QAction(tr("Undo"), m_textMenu);
    m_textRedoAct = new QAction(tr("Redo"), m_textMenu);
    m_textLeftAlignAct = new QAction(tr("Text Align Left"), m_textMenu);                 // 左对齐
    m_textRightAlignAct = new QAction(tr("Text Align Right"), m_textMenu);            // 右对齐
    m_textCenterAlignAct = new QAction(tr("Text Align Center"), m_textMenu);     //  水平垂直居中对齐
    m_textDeleteAction = new QAction(tr("Delete"), m_textMenu);

    m_textMenu->addAction(m_textCutAction);
    m_textMenu->addAction(m_textCopyAction);
    m_textMenu->addAction(m_textPasteAction);
    m_textMenu->addAction(m_textSelectAllAction);
    m_textMenu->addAction(m_textDeleteAction);
    m_textMenu->addSeparator();
    m_textMenu->addAction(m_textUndoAct);
    m_textMenu->addAction(m_textRedoAct);
    m_textMenu->addSeparator();

    m_textMenu->addAction(m_textLeftAlignAct);
    m_textMenu->addAction(m_textRightAlignAct);
    m_textMenu->addAction(m_textCenterAlignAct);
}

void PageView::initTextContextMenuConnection()
{
    connect(m_textCutAction, &QAction::triggered, this, &PageView::slotOnTextCut);
    connect(m_textCopyAction, &QAction::triggered, this, &PageView::slotOnTextCopy);
    connect(m_textPasteAction, &QAction::triggered, this, &PageView::slotOnTextPaste);
    connect(m_textSelectAllAction, &QAction::triggered, this, &PageView::slotOnTextSelectAll);
    connect(m_textUndoAct, &QAction::triggered, this, &PageView::slotOnTextUndo);
    connect(m_textRedoAct, &QAction::triggered, this, &PageView::slotOnTextRedo);
    connect(m_textDeleteAction, &QAction::triggered, this, &PageView::slotOnTextDelete);

    //  设置文字图元内部对齐方式
    connect(m_textLeftAlignAct, &QAction::triggered, this, [ = ]() {
        slotSetTextAlignment(Qt::AlignLeft);
    });
    connect(m_textRightAlignAct, &QAction::triggered, this, [ = ]() {
        slotSetTextAlignment(Qt::AlignRight);
    });
    connect(m_textCenterAlignAct, &QAction::triggered, this, [ = ]() {
        slotSetTextAlignment(Qt::AlignCenter);
    });
}

void PageView::initConnection()
{
    qRegisterMetaType<SGraphicsTextUnitData>("SGraphicsTextUnitData");
    qRegisterMetaType<SGraphicsUnitHead>("SGraphicsUnitHead");
    qRegisterMetaType<CGraphicsUnit>("CGraphicsUnit&");
}

void PageView::contextMenuEvent(QContextMenuEvent *event)
{
    CHECK_CURRENTTOOL_RETURN(this)
    QPointF pos = this->mapToScene(event->pos());
    QRectF rect = this->scene()->sceneRect();

    enum {EShowTxtMenu, EShowCommonMenu};
    int menuTp = EShowCommonMenu;

    // 0.规避显示条件
    if (!rect.contains(pos) || !m_isShowContext) {
        return;
    }

    // 1.判断确定要显示的菜单
    auto pProxDrawItem = activeProxDrawItem();
    CGraphicsItem *tmpitem = pProxDrawItem;
    QList<CGraphicsItem *> selectItems = drawScene()->selectGroup()->items();
    QMenu *pMenu = m_contextMenu;
    if (selectItems.count() > 0) {
        if (pProxDrawItem != nullptr && pProxDrawItem->isSelected()) {
            // 1.1 显示文字图元右键菜单
            if (TextType == pProxDrawItem->type() &&  static_cast<CGraphicsTextItem *>(tmpitem)->isEditState()) {

                // 文本框右键菜单中剪切和复制只有在选中文字才会激活
                CGraphicsTextItem *textItem = static_cast<CGraphicsTextItem *>(tmpitem);
                bool showFalg = textItem->textEditor()->textCursor().hasSelection();
                m_textCutAction->setEnabled(showFalg);
                m_textCopyAction->setEnabled(showFalg);

                //根据当前条件,判断是否可执行,未激活的Action应该置灰
                QTextDocument *doc = textItem->textEditor()->document();
                if (doc != nullptr) {
                    m_textUndoAct->setEnabled(doc->isUndoAvailable());
                    m_textRedoAct->setEnabled(doc->isRedoAvailable());
                }

                m_textPasteAction->setEnabled(textItem->textEditor()->canPaste());

                menuTp = EShowTxtMenu;
                pMenu = m_textMenu;
            }
        }
    }
    // 2.设置菜单中action的状态
    if (menuTp == EShowTxtMenu) {

        // 2.1 文本图元对齐显示状态
        setTextAlignMenuActionStatus(tmpitem);

    } else {
        // 2.3 一般图元的cut，copy，delete，paste操作的状态
        setCcdpMenuActionStatus(selectItems.count() > 0);

        m_selectAllAct->setEnabled(drawScene()->getBzItems().count() > 0);
        m_group->setEnabled(drawScene()->isGroupable());
        m_unGroup->setEnabled(drawScene()->isUnGroupable());

        // 2.4 设置选择的图元个数显示能进行的对齐状态
        setAlignMenuActionStatus(selectItems.count() > 0);

        // 2.5 设置右键菜单图层选项状态
        setLayerMenuActionStatus(selectItems.count() > 0);
    }

    //判断剪切板状态是否可粘贴
    setClipboardStatus();

    //显示菜单
    showMenu(pMenu);

    DGraphicsView::contextMenuEvent(event);
}

void PageView::showMenu(DMenu *pMenu)
{
    //判断当前状态是否屏蔽右键菜单
    //CHECK_CURRENTTOOL_RETURN(this)

    QPoint curPos = QCursor::pos();
    //保存当前鼠标位置为文字粘贴的位置
    letfMenuPopPos = this->mapToScene(this->viewport()->mapFromGlobal(QCursor::pos()));

    QSize menSz = pMenu->sizeHint();

    QRect menuRect = QRect(curPos, menSz);


    QScreen *pCurScren = this->window()->windowHandle()->screen();
    qWarning() << "menuRect" << menuRect.right() << menuRect.left() << menuRect.bottom() << menuRect.top();
    qWarning() << "geomeRect" << pCurScren->geometry().right() << pCurScren->geometry().bottom();
    if (pCurScren != nullptr) {
        QRect geomeRect = pCurScren->geometry();
        bool isadjustright = false;
        int offset = 20;
        if (!geomeRect.contains(menuRect)) {
            if (menuRect.left() + menuRect.width() > geomeRect.right()) {
                int move = menuRect.width() + offset;
                menuRect.adjust(-move, 0, -move, 0);
                isadjustright = true;
            }

            if (menuRect.bottom() > geomeRect.bottom()) {
                int move = menuRect.bottom() - geomeRect.bottom();
                if (isadjustright)
                    menuRect.adjust(0, -move, 0, -move);
                else
                    menuRect.adjust(offset, -move, offset, -move);
            }
        }
    }

    pMenu->move(menuRect.topLeft());

    pMenu->show();
}


void PageView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
}

void PageView::paintEvent(QPaintEvent *event)
{
    if (!_cacheEnable)
        DGraphicsView::paintEvent(event);
    else {

        QPainter painter(this->viewport());
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        painter.drawPixmap(QPoint(0, 0), _cachePixmap);

        //绘制缓冲时的额外的前景显示
        IDrawTool *pTool = page()->currentTool_p();

        if (pTool != nullptr) {
            pTool->drawMore(&painter, mapToScene(QRect(QPoint(0, 0), _cachePixmap.size())).boundingRect(), drawScene());
        }
    }
}

void PageView::drawItems(QPainter *painter, int numItems, QGraphicsItem *items[], const QStyleOptionGraphicsItem options[])
{
    QRectF rect = scene()->sceneRect();
    painter->setClipping(true);
    painter->setClipRect(rect);
    DGraphicsView::drawItems(painter, numItems, items, options);
}

void PageView::leaveEvent(QEvent *event)
{
//    if (page()->currentTool_p() != nullptr) {
//        if (drawScene() != nullptr) {
//            auto e = CDrawToolEvent::fromQEvent_single(event, drawScene());
//            page()->currentTool_p()->leaveEvent(&e);
//        }
//    }
    QGraphicsView::leaveEvent(event);
}

//void PageView::slotStartLoadDDF(QRectF rect)
//{
//    scene()->setSceneRect(rect);
//}

void PageView::slotOnCut()
{
    CHECK_CURRENTTOOL_RETURN(this)

    //1.将数据复制到粘贴板
    slotOnCopy();

    //2.删除当前选中的图元(直接调用删除响应槽即可)
    slotOnDelete();
}


void PageView::slotOnCopy()
{
    CHECK_CURRENTTOOL_RETURN(this)
    CShapeMimeData *data = new CShapeMimeData(drawScene()->getGroupTreeInfo(drawScene()->selectGroup()));
    data->setText("");
    page()->borad()->setClipBoardShapeData(data);
    m_pasteAct->setEnabled(true);
}

void PageView::slotOnPaste(bool textItemInCenter)
{
    CHECK_MOSUEACTIVE_RETURN
    QMimeData *mp = page()->borad()->clipBoardShapeData();
    if (mp == nullptr)
        return;

    if (mp->hasImage()) {
        QTimer::singleShot(100, nullptr, [ = ] {
            QImage image = qvariant_cast<QImage>(mp->imageData());
            image = image.convertToFormat(QImage::Format_ARGB32);
            auto pos = page()->context()->pageRect().center() - image.rect().center();
            QRectF rect = QRectF();
            int r = -1;
            if (adaptImgPosAndRect(page()->scene(), "", image, pos, rect, r))
            {
                page()->context()->addImage(image, pos, rect, true);
            }

        });
    } else if (mp->hasUrls()) {
        QList<QUrl> urls = mp->urls();
        QStringList strList;
        foreach (auto url, urls) {
            strList << url.toLocalFile();
        }

        page()->borad()->loadFiles(strList);
    } else if (mp->hasText()) {
        QString filePath = mp->text();
        // clean selection
        this->drawScene()->clearSelectGroup();

        // add text item
        CGraphicsItem *item = CGraphicsItem::creatItemInstance(TextType);
        if (item) {
            CGraphicsTextItem *textItem = static_cast<CGraphicsTextItem *>(item);
            if (textItem) {
                page()->setCurrentTool(selection);
                IDrawTool::setViewToSelectionTool(this);
                textItem->textEditor()->setPlainText(filePath);
                QList<QVariant> vars;
                vars << reinterpret_cast<long long>(scene());
                vars << reinterpret_cast<long long>(item);
                drawScene()->addItem(item);
                textItem->updateToDefaultTextFormat();
                textItem->textEditor()->applyDefaultToFirstFormat();
                textItem->textEditor()->document()->clearUndoRedoStacks();
                if (textItemInCenter) {
                    item->setPos(this->mapToScene(viewport()->rect().center()) - QPointF(item->boundingRect().width(), item->boundingRect().height()) / 2);
                } else {
                    item->setPos(letfMenuPopPos);
                }

                qreal newZ = this->drawScene()->getMaxZValue() + 1;
                item->setZValue(newZ);
                CUndoRedoCommand::recordUndoCommand(CUndoRedoCommand::ESceneChangedCmd,
                                                    CSceneUndoRedoCommand::EItemAdded, vars, true, true);
                CUndoRedoCommand::recordRedoCommand(CUndoRedoCommand::ESceneChangedCmd,
                                                    CSceneUndoRedoCommand::EItemAdded, vars);
                CUndoRedoCommand::finishRecord();
                drawScene()->selectItem(item, true, true, true);
                textItem->setTextState(CGraphicsTextItem::EInEdit, false);

            } else {
                delete item;
                item = nullptr;
            }
        }
    } else {
        qDebug() << "mp->hasImage()"  << mp->hasImage() << endl;

        //粘贴画板内部图元
        CShapeMimeData *data = qobject_cast<CShapeMimeData *>(mp);
        if (data != nullptr) {
            drawScene()->clearSelectGroup();

            //0.复制前记录当前场景的组合快照(用于撤销)
            drawScene()->recordSecenInfoToCmd(CSceneUndoRedoCommand::EGroupChanged, UndoVar);

            //1.复制生成新的图元并添加到场景中去
            CGraphicsItemGroup *pGroup = drawScene()->loadGroupTreeInfo(data->itemsTreeInfo(), true);

            QList<CGraphicsItem *> needSelected;
            if (pGroup != nullptr) {
                needSelected = pGroup->items();
                QList<CGraphicsItem *> allItems = pGroup->getBzItems(true);

                //2.调整复制生成图元的位置,并记录他们添加到场景动作用于撤销还原
                QList<QVariant> vars;
                vars << reinterpret_cast<long long>(scene());
                foreach (CGraphicsItem *item, allItems) {
                    vars << reinterpret_cast<long long>(item);
                    item->moveBy(10, 10);
                }
                CUndoRedoCommand::recordUndoCommand(CUndoRedoCommand::ESceneChangedCmd,
                                                    CSceneUndoRedoCommand::EItemAdded, vars, false, true);

                if (pGroup->groupType() == CGraphicsItemGroup::EVirRootGroup) {
                    drawScene()->cancelGroup(pGroup);
                    delete pGroup;
                }
            }

            //3.复制后记录当前场景的组合快照(用于还原)
            drawScene()->recordSecenInfoToCmd(CSceneUndoRedoCommand::EGroupChanged, RedoVar);

            //4.撤销还原入栈
            CUndoRedoCommand::finishRecord();

            foreach (auto p, needSelected) {
                drawScene()->selectItem(p, true, false, false);

                CGraphItemMoveEvent event(CGraphItemEvent::EMove);
                event.setEventPhase(EChanged);
                event._oldScenePos = p->scenePos();
                event._scenePos = event._oldScenePos + QPointF(10, 10);
                p->operating(&event);
            }
            drawScene()->selectGroup()->updateBoundingRect();
            drawScene()->selectGroup()->updateAttributes();
        }
    }
    //5.粘贴数据进入画板,鼠标设置为选择工具
    page()->setCurrentTool(selection);
}

void PageView::slotOnSelectAll()
{
    CHECK_CURRENTTOOL_RETURN(this)
    drawScene()->selectItemsByRect(sceneRect());
    auto items = drawScene()->getBzItems();
    foreach (auto p, items) {
        drawScene()->selectItem(p, false, false, false);
    }
    drawScene()->selectGroup()->updateAttributes();
    drawScene()->selectGroup()->updateBoundingRect();
}

void PageView::slotOnDelete()
{
    CHECK_CURRENTTOOL_RETURN(this)

    // 得到要被删除的基本业务图元
    QList<CGraphicsItem *> allItems = drawScene()->selectGroup()->items(true);

    // 如果当前有多选图元并且当前不是连笔绘制图元才执行删除
    if (allItems.count() > 0) {

        //获取到涉及到的组合图元(删除基本业务图元前要先取消这些组合)
        QList<CGraphicsItemGroup *> groups = drawScene()->selectGroup()->getGroups(true);

        QList<QVariant> vars;
        vars << reinterpret_cast<long long>(scene());
        for (CGraphicsItem *pItem : allItems) {
            if (pItem->isBzItem()) {
                vars << reinterpret_cast<long long>(pItem);
            }
        }
        //保证在删除之前执行保存当前组合的快照情况
        drawScene()->recordSecenInfoToCmd(CSceneUndoRedoCommand::EGroupChanged, UndoVar);


        //删除放在保存当前组合的快照情况的后面,才能保证重做时先添加回来图元再还原组合快照情况
        CUndoRedoCommand::recordUndoCommand(CUndoRedoCommand::ESceneChangedCmd,
                                            CSceneUndoRedoCommand::EItemRemoved, vars, false, true);


        //删除基本图元前先清除他们涉及到的组合
        foreach (auto gp, groups)
            drawScene()->cancelGroup(gp);

        //执行删除操作
        foreach (CGraphicsItem *pItem, allItems) {
            drawScene()->removeCItem(pItem);
        }

        //记录在删除之后的组合的快照情况,用于重做
        drawScene()->recordSecenInfoToCmd(CSceneUndoRedoCommand::EGroupChanged, RedoVar);


        //结束改次撤销还原信息的收集,生成undoredo对象以响应之后的撤销还原操作
        CUndoRedoCommand::finishRecord();

        //刷新多选图元
        drawScene()->selectGroup()->updateBoundingRect();

        //drawScene()->refreshLook();
    }
}

void PageView::slotOneLayerUp()
{
    CHECK_CURRENTTOOL_RETURN(this)
    drawScene()->moveBzItemsLayer(drawScene()->selectGroup()->items(), EUpLayer, 1, nullptr, true);
}

void PageView::slotOneLayerDown()
{
    CHECK_CURRENTTOOL_RETURN(this)
    drawScene()->moveBzItemsLayer(drawScene()->selectGroup()->items(), EDownLayer, 1, nullptr, true);
}

void PageView::slotBringToFront()
{
    CHECK_CURRENTTOOL_RETURN(this)
    drawScene()->moveBzItemsLayer(drawScene()->selectGroup()->items(), EUpLayer, -1, nullptr, true);
}

void PageView::slotSendTobackAct()
{
    CHECK_CURRENTTOOL_RETURN(this)
    drawScene()->moveBzItemsLayer(drawScene()->selectGroup()->items(), EDownLayer, -1, nullptr, true);
}

//void PageView::slotQuitCutMode()
//{
//    page()->setCurrentTool(selection);
//}

//void CGraphicsView::slotDoCutScene()
//{
//    // [42259] 解决处于裁剪的时候编辑输入框裁剪大小回车不响应输入框，因为view设置了全局的快捷键

//    if (getDrawParam()->getCurrentDrawToolMode() == cut) {
//        QLineEdit *foucsLIneedit = qobject_cast<QLineEdit *>(dApp->focusObject());
//        if (foucsLIneedit != nullptr) {
//            m_cutScence->setEnabled(false);
//            QKeyEvent event(QEvent::KeyPress, Qt::Key_Return, dApp->keyboardModifiers());
//            dApp->sendEvent(foucsLIneedit, &event);
//            m_cutScence->setEnabled(true);
//        } else {
//            static_cast<CDrawScene *>(scene())->doCutScene();
//            this->getDrawParam()->setCurrentDrawToolMode(EDrawToolMode::selection);
//        }
//    } else {
//        if (dApp->focusObject() != nullptr) {
//            m_cutScence->setEnabled(false);
//            QKeyEvent event(QEvent::KeyPress, Qt::Key_Return, dApp->keyboardModifiers());
//            dApp->sendEvent(dApp->focusObject(), &event);
//            m_cutScence->setEnabled(true);
//        }
//    }
//}

void PageView::slotViewZoomIn()
{
    CHECK_MOSUEACTIVE_RETURN
    zoomIn(EViewCenter);
}

void PageView::slotViewZoomOut()
{
    CHECK_MOSUEACTIVE_RETURN
    zoomOut(EViewCenter);
}

void PageView::slotViewOrignal()
{
    CHECK_MOSUEACTIVE_RETURN
    this->scale(1.0);
    emit signalSetScale(m_scale);
}

void PageView::slotOnTextCut()
{
    if (!scene()->selectedItems().isEmpty()) {
        QGraphicsItem *item =  scene()->selectedItems().first();
        CGraphicsTextItem *tmpitem = static_cast<CGraphicsTextItem *>(item);
        if (TextType == item->type() &&  tmpitem->isEditState()) {
            tmpitem->doCut();
        }
    }
}

void PageView::slotOnTextCopy()
{
    if (!scene()->selectedItems().isEmpty()) {
        QGraphicsItem *item =  scene()->selectedItems().first();
        CGraphicsTextItem *tmpitem = static_cast<CGraphicsTextItem *>(item);
        if (TextType == item->type() &&  tmpitem->isEditState()) {
            tmpitem->doCopy();
        }
    }
}

void PageView::slotOnTextPaste()
{
    if (!scene()->selectedItems().isEmpty()) {
        QGraphicsItem *item =  scene()->selectedItems().first();
        CGraphicsTextItem *tmpitem = static_cast<CGraphicsTextItem *>(item);
        if (TextType == item->type() &&  tmpitem->isEditState()) {
            tmpitem->doPaste();
        }
    }
}

void PageView::slotOnTextSelectAll()
{
    if (!scene()->selectedItems().isEmpty()) {
        QGraphicsItem *item =  scene()->selectedItems().first();
        CGraphicsTextItem *tmpitem = static_cast<CGraphicsTextItem *>(item);
        if (TextType == item->type() &&  tmpitem->isEditState()) {
            tmpitem->doSelectAll();
        }
    }
}

void PageView::slotSetTextAlignment(const Qt::Alignment &align)
{
    if (!scene()->selectedItems().isEmpty()) {
        QGraphicsItem *item =  scene()->selectedItems().first();
        CGraphicsTextItem *tmpitem = static_cast<CGraphicsTextItem *>(item);
        if (TextType == item->type() &&  tmpitem->isEditState()) {
            tmpitem->setSelectTextBlockAlign(align);
        }
    }
}

void PageView::slotOnTextUndo()
{
    if (!scene()->selectedItems().isEmpty()) {
        QGraphicsItem *item =  scene()->selectedItems().first();
        CGraphicsTextItem *tmpitem = static_cast<CGraphicsTextItem *>(item);
        if (TextType == item->type() &&  tmpitem->isEditState()) {
            tmpitem->doUndo();
        }
    }
}

void PageView::slotOnTextRedo()
{
    if (!scene()->selectedItems().isEmpty()) {
        QGraphicsItem *item =  scene()->selectedItems().first();
        CGraphicsTextItem *tmpitem = static_cast<CGraphicsTextItem *>(item);
        if (TextType == item->type() &&  tmpitem->isEditState()) {
            tmpitem->doRedo();
        }
    }
}

void PageView::slotOnTextDelete()
{
    if (!scene()->selectedItems().isEmpty()) {
        QGraphicsItem *item =  scene()->selectedItems().first();
        CGraphicsTextItem *tmpitem = static_cast<CGraphicsTextItem *>(item);
        if (TextType == item->type() &&  tmpitem->isEditState()) {
            tmpitem->doDelete();
        }
    }
}

//void PageView::clearScene()
//{
//    ///清除场景选中
//    scene()->clearSelection();

//    //清空撤销栈
//    m_pUndoStack->clear();

//    auto curScene = static_cast<PageScene *>(scene());

//    curScene->resetScene();
//}

void PageView::pushUndoStack(QUndoCommand *cmd)
{
    //记录入栈信息
    userActionCount++;
    m_pUndoStack->beginMacro("");
    m_pUndoStack->push(cmd);
    m_pUndoStack->endMacro();
    drawScene()->pageContext()->setDirty(isModified());
}

void PageView::pushActionCount()
{
    //记录入栈
    userActionCount++;
    drawScene()->pageContext()->setDirty(isModified());
}


bool PageView::isModified() const
{
    return userActionCount > 0;
}

//bool PageView::isModifyStashEmpty()
//{
//    return (!m_pUndoStack->canRedo() && !m_pUndoStack->canUndo());
//}

bool PageView::isKeySpacePressed()
{
    return _spaceKeyPressed;
}
PageScene *PageView::drawScene()const
{
    return dynamic_cast<PageScene *>(scene());
}

void PageView::setCacheEnable(bool enable, bool fruzzCurFrame)
{
    if (enable == _cacheEnable)
        return;

    _cacheEnable = enable;
    if (_cacheEnable) {

        _cachePixmap = QPixmap(this->viewport()->size() * devicePixelRatioF());
        _cachePixmap.fill(QColor(0, 0, 0, 0));


        if (fruzzCurFrame) {
            QPainter painter(&_cachePixmap);
            painter.setPen(Qt::NoPen);
            painter.setRenderHint(QPainter::Antialiasing);
            this->scene()->render(&painter, QRectF(0, 0, _cachePixmap.width(), _cachePixmap.height()),
                                  QRectF(mapToScene(QPoint(0, 0)), mapToScene(QPoint(_cachePixmap.size().width(), _cachePixmap.size().height()))), Qt::IgnoreAspectRatio);

            painter.drawRect(viewport()->rect());
        }
    }
    viewport()->update();
}

bool PageView::isCacheEnabled()
{
    return _cacheEnable;
}

QPixmap &PageView::cachedPixmap()
{
    return _cachePixmap;
}

QWidget *PageView::activeProxWidget()
{
    if (activeProxItem() != nullptr)
        return activeProxItem()->widget();
    return nullptr;
}

QGraphicsProxyWidget *PageView::activeProxItem()
{
    if (scene() == nullptr || scene()->focusItem() == nullptr)
        return nullptr;

    if (scene()->focusItem()->type() == QGraphicsProxyWidget::Type) {
        auto pProxyIt = static_cast<QGraphicsProxyWidget *>(scene()->focusItem());
        if (pProxyIt->flags()&QGraphicsItem::ItemHasNoContents) {
            return nullptr;
        }
        return pProxyIt;
    }
    return nullptr;
}

CGraphicsItem *PageView::activeProxDrawItem()
{
    if (activeProxItem() != nullptr) {
        return dynamic_cast<CGraphicsItem *>(activeProxItem()->parentItem());
    }
    return nullptr;
}

void PageView::captureFocus(bool force)
{
    auto pView = this;
    if (pView != nullptr) {
        if (pView->activeProxDrawItem() != nullptr) {
            auto pText = static_cast<CGraphicsTextItem *>(pView->activeProxDrawItem());
            if (pText != nullptr && pText->isEditState()) {
                pText->toFocusEiditor();
            }
        } else {
            if (force)
                pView->setFocus();
        }
    }
}

void PageView::showEvent(QShowEvent *event)
{
    this->setTransformationAnchor(AnchorViewCenter);
    QGraphicsView::showEvent(event);
}

void PageView::updateSelectedItemsAlignment(Qt::AlignmentFlag align)
{
    CHECK_CURRENTTOOL_RETURN(this)

    // 获取选择的组合中全部图元
    auto curScene = dynamic_cast<PageScene *>(scene());
    QList<CGraphicsItem *> allItems = curScene->selectGroup()->items();

    // 模拟图元移动事件
    CGraphItemMoveEvent event(CGraphItemEvent::EMove);
    event.setEventPhase(EChanged);
    // 移动的起点相对位置
    event._oldScenePos = QPointF(0, 0);

    // [0] 没有选中的图元直接返回
    if (!allItems.size()) {
        return;
    }

    // [1] 用于记录保存图元的位置，便于撤销和返回
    QMap<CGraphicsItem *, QPointF> startPos;
    QMap<CGraphicsItem *, QPointF> endPos;

    // 获取多个图元和单个图元的边界值
    QRectF currSceneRect;
    if (allItems.size() > 1) {
        currSceneRect = curScene->selectGroup()->mapRectToScene(curScene->selectGroup()->rect());
        if (align == Qt::AlignVCenter) {align = Qt::AlignHCenter;}
        else if (align == Qt::AlignHCenter) {align = Qt::AlignVCenter;}
    } else {
        currSceneRect = sceneRect();
    }

    // [1] 设置对齐坐标
    for (int i = 0; i < allItems.size(); i++) {
        QRectF itemRect = allItems.at(i)->mapRectToScene(allItems.at(i)->rect());

        startPos.insert(allItems.at(i), allItems.at(i)->sceneBoundingRect().topLeft());

        event._scenePos = alignmentMovPos(currSceneRect, itemRect, align);

        allItems.at(i)->operating(&event);
        //图片图元应该保存命令
        if (nullptr != dynamic_cast<JDynamicLayer *>(allItems.at(i))) {
            allItems.at(i)->operatingEnd(&event);
        }

        endPos.insert(allItems.at(i), allItems.at(i)->sceneBoundingRect().topLeft());
    }

    // [4] 设置出入栈
    QUndoCommand *addCommand = new CItemsAlignCommand(static_cast<PageScene *>(scene()), startPos, endPos);
    pushUndoStack(addCommand);
}

QPointF PageView::alignmentMovPos(const QRectF &fatherRect, const QRectF &childRect,
                                  Qt::AlignmentFlag align)
{
    QPointF moveVector = QPointF(0, 0);

    switch (align) {
    case Qt::AlignLeft: {
        moveVector.setX(fatherRect.left() - childRect.left());
        break;
    }
    case Qt::AlignVCenter: {
        moveVector.setY(fatherRect.center().y() - childRect.center().y());
        break;
    }
    case Qt::AlignRight: {
        moveVector.setX(fatherRect.right() - childRect.right());
        break;
    }
    case Qt::AlignTop: {
        moveVector.setY(fatherRect.top() - childRect.top());
        break;
    }
    case Qt::AlignHCenter: {
        moveVector.setX(fatherRect.center().x() - childRect.center().x());
        break;
    }
    case Qt::AlignBottom: {
        moveVector.setY(fatherRect.bottom() - childRect.bottom());
        break;
    }
    default: {
        moveVector = QPointF(0, 0);
        break;
    }
    }
    return moveVector;
}

void PageView::setCcdpMenuActionStatus(bool enable)
{
    m_copyAct->setEnabled(enable);
    m_cutAct->setEnabled(enable);
    m_deleteAct->setEnabled(enable);
}

void PageView::setClipboardStatus()
{
    bool pasteFlag = false;
    QMimeData *mp = page()->borad()->clipBoardShapeData();
    // 判断剪切板数据是否为文字
    if (mp->hasText()) {
        pasteFlag = true;
    } else if (mp->hasImage()) {
        pasteFlag = true;
    } else if (mp->hasFormat("drawItems")) {
        pasteFlag = true;
    }

    m_pasteAct->setEnabled(pasteFlag);
    m_undoAct->setEnabled(m_pUndoStack->canUndo());
    m_redoAct->setEnabled(m_pUndoStack->canRedo());
}

void PageView::setLayerMenuActionStatus(bool layervistual)
{
    m_layerMenu->setEnabled(layervistual);

    bool layerUp = drawScene()->isCurrentZMovable(EUpLayer);
    m_oneLayerUpAct->setEnabled(layerUp);
    m_bringToFrontAct->setEnabled(layerUp);

    bool layerDown = drawScene()->isCurrentZMovable(EDownLayer);
    m_oneLayerDownAct->setEnabled(layerDown);
    m_sendTobackAct->setEnabled(layerDown);
}

void PageView::setAlignMenuActionStatus(bool acticonvistual)
{
    m_alignMenu->setEnabled(acticonvistual);

    m_itemsLeftAlign->setEnabled(acticonvistual);      //左对齐
    m_itemsHCenterAlign->setEnabled(acticonvistual);   //水平居中对齐
    m_itemsRightAlign->setEnabled(acticonvistual);     //右对齐
    m_itemsTopAlign->setEnabled(acticonvistual);       //顶对齐
    m_itemsVCenterAlign->setEnabled(acticonvistual);   //垂直居中对齐
    m_itemsBottomAlign->setEnabled(acticonvistual);    //底对齐

    auto curScene = dynamic_cast<PageScene *>(scene());
    const int selectItemsCount = curScene->selectGroup()->items().size();
    m_itemsVEqulSpaceAlign->setEnabled(selectItemsCount >= 3);//水平等间距对齐
    m_itemsHEqulSpaceAlign->setEnabled(selectItemsCount >= 3);//垂直等间距对齐
}

void PageView::setTextAlignMenuActionStatus(CGraphicsItem *tmpitem)
{
    // 根据是否是两点情况显示对齐
    if (static_cast<CGraphicsTextItem *>(tmpitem)->isAutoAdjustSize()) {
        m_textLeftAlignAct->setEnabled(false);
        m_textRightAlignAct->setEnabled(false);
        m_textCenterAlignAct->setEnabled(false);
    } else {
        m_textLeftAlignAct->setEnabled(true);
        m_textRightAlignAct->setEnabled(true);
        m_textCenterAlignAct->setEnabled(true);
    }
}

//拖曳加载文件
void PageView::dropEvent(QDropEvent *e)
{
    //pass to class QAbstractScrollArea to handle, then block this event for scene.
    QAbstractScrollArea::dropEvent(e);
}

void PageView::dragEnterEvent(QDragEnterEvent *event)
{
    //pass to class QAbstractScrollArea to handle, then block this event for scene.
    QAbstractScrollArea::dragEnterEvent(event);
}

void PageView::dragMoveEvent(QDragMoveEvent *event)
{
    //pass to class QAbstractScrollArea to handle, then block this event for scene.
    QAbstractScrollArea::dragMoveEvent(event);
}

void PageView::enterEvent(QEvent *event)
{
//    if (page()->currentTool_p() != nullptr && drawScene() != nullptr) {
//        auto e = CDrawToolEvent::fromQEvent_single(event, drawScene());
//        page()->currentTool_p()->enterEvent(&e);
//    }
    QGraphicsView::enterEvent(event);
}

void PageView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space) {
        if (!event->isAutoRepeat()) {
            if (activeProxWidget() == nullptr && dApp->mouseButtons() == Qt::NoButton) {
                _spaceKeyPressed = true;

                page()->setDrawCursor(QCursor(Qt::ClosedHandCursor));
                page()->blockSettingDrawCursor(true);
                page()->view()->viewport()->update();
            }
        }
    }
    QGraphicsView::keyPressEvent(event);
}

void PageView::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space) {
        if (!event->isAutoRepeat()) {
            if (_spaceKeyPressed) {
                _spaceKeyPressed = false;
                page()->blockSettingDrawCursor(false);
                if (page()->currentTool_p() != nullptr)
                    page()->currentTool_p()->refresh();
            }
        }
    }
    QGraphicsView::keyReleaseEvent(event);
}

bool PageView::eventFilter(QObject *o, QEvent *e)
{
    if (viewport() == o) {
        bool finished = false;
        if (e->type() == QEvent::MouseButtonPress) {
            QMouseEvent *event = dynamic_cast<QMouseEvent *>(e);
            _pressBeginPos = event->pos();
            _recordMovePos = _pressBeginPos;
            if (_spaceKeyPressed && event->button() == Qt::LeftButton) {
                finished       = true;
            }
        } else if (e->type() == QEvent::MouseMove) {
            QMouseEvent *event = dynamic_cast<QMouseEvent *>(e);
            {
                if (_spaceKeyPressed && event->buttons() == Qt::LeftButton) {
                    //移动卷轴
                    PageScene *pScene = qobject_cast<PageScene *>(scene());
                    if (pScene != nullptr) {
                        pScene->blockMouseMoveEvent(true);
                    }
                    QPointF mov = event->pos() - _recordMovePos;
                    int horValue = this->horizontalScrollBar()->value() - qRound(mov.x());
                    //qDebug() << "old hor value = " << this->horizontalScrollBar()->value() << "new hor value = " << horValue;
                    this->horizontalScrollBar()->setValue(qMin(qMax(this->horizontalScrollBar()->minimum(), horValue), this->horizontalScrollBar()->maximum()));

                    int verValue = this->verticalScrollBar()->value() - qRound(mov.y());
                    //qDebug() << "mov.y() = " << mov.y() << "cur value = " << this->verticalScrollBar()->value() << "wanted value = " << verValue << "max = " << this->verticalScrollBar()->maximum();
                    this->verticalScrollBar()->setValue(qMin(qMax(this->verticalScrollBar()->minimum(), verValue), this->verticalScrollBar()->maximum()));

                    if (pScene != nullptr) {
                        pScene->blockMouseMoveEvent(false);
                    }

                    finished = true;
                }
                _recordMovePos = event->pos();
            }
        } else if (e->type() == QEvent::Leave) {
            if (drawScene() != nullptr) {
                drawScene()->doLeave();
            }
        }
        return finished;
    }
    return DGraphicsView::eventFilter(o, e);
}

bool PageView::viewportEvent(QEvent *event)
{
    if (event->type() == QEvent::Leave) {
        if (page()->currentTool_p() != nullptr) {
            if (drawScene() != nullptr) {
                auto e = CDrawToolEvent::fromQEvent_single(event, drawScene());
                page()->currentTool_p()->leaveEvent(&e);
            }
        }
    } else if (event->type() == QEvent::Enter) {
        if (page()->currentTool_p() != nullptr && drawScene() != nullptr) {
            auto e = CDrawToolEvent::fromQEvent_single(event, drawScene());
            page()->currentTool_p()->enterEvent(&e);
        }
    }
    return DGraphicsView::viewportEvent(event);
}

bool PageView::gestureEvent(QGestureEvent *event)
{
    if (QGesture *pinch = event->gesture(Qt::PinchGesture))
        pinchTriggered(static_cast<QPinchGesture *>(pinch));
    return true;
}

void PageView::pinchTriggered(QPinchGesture *gesture)
{
    QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags();
    if (changeFlags & QPinchGesture::RotationAngleChanged) {
        qreal rotationDelta = gesture->rotationAngle() - gesture->lastRotationAngle();
        Q_UNUSED(rotationDelta);
    }
    if (changeFlags & QPinchGesture::ScaleFactorChanged) {
        scaleWithCenter(gesture->scaleFactor(), viewport()->mapFromGlobal(gesture->centerPoint().toPoint()));
    }
    if (gesture->state() == Qt::GestureFinished) {
    }
    update();
}





