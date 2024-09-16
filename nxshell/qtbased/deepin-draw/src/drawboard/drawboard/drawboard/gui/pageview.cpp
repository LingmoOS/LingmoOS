// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pageview.h"
#include "drawboard.h"
#include "pagecontext.h"
#include "cshapemimedata.h"
#include "pageitem.h"
#include "globaldefine.h"
#include "pagescene.h"
#include "itemgroup.h"
#include "layeritem.h"


#include "cundocommands.h"
#include "cundoredocommand.h"
#include "cgraphicsitemevent.h"
#include "progresslayout.h"
#include "DataHanderInterface.h"
#include "ctextedit.h"
#include "textitem.h"
#include "cmenu.h"
#include "attributemanager.h"

#include <QTimer>

#include <QFileDialog>
#include <QUndoStack>

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
#include <QOpenGLWidget>
#include <DMenu>

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

#define CHECK_MOSUEACTIVE_RETURN \
    if(qApp->mouseButtons() == Qt::LeftButton)\
    {\
        auto view1 = page()->borad()->currentPage()->view();\
        if(view1 != nullptr && view1->viewport()->rect().contains(view1->viewport()->mapFromGlobal(QCursor::pos())))\
        {\
            return;\
        }\
    }
#define CHECK_CURRENTTOOL_RETURN(pView) \
    CHECK_MOSUEACTIVE_RETURN \
    auto view = page()->borad()->currentPage()->view();\
    if(pView != view){return;}\
    if(pView == nullptr){return;}\
    int currentMode = view->page()->currentTool();\
    if (currentMode != selection) {\
        return;\
    }


class PageView::PageView_private
{
public:
    explicit PageView_private(PageView *qq): q(qq) {}

    void slotOnCut()
    {
        CHECK_CURRENTTOOL_RETURN(q)

        //1.将数据复制到粘贴板
        slotOnCopy();

        //2.删除当前选中的图元(直接调用删除响应槽即可)
        slotOnDelete();
    }

    void slotOnCopy()
    {
        CHECK_CURRENTTOOL_RETURN(q)
        auto selectedItems = returnSortZItems(q->pageScene()->selectedPageItems(), EAesSort);
        UnitList units;
        foreach (auto p, selectedItems) {
            units.append(p->getItemUnit(UnitUsage_Copy));
        }
        DrawBoardMimeData *data = new DrawBoardMimeData(units);
        page()->borad()->setClipBoardShapeData(data);
    }

    void slotOnPaste(bool textItemInCenter = true)
    {
        CHECK_MOSUEACTIVE_RETURN
        QMimeData *mp = page()->borad()->clipBoardShapeData();
        if (mp == nullptr)
            return;

        if (mp->hasImage()) {
            QTimer::singleShot(100, nullptr, [ = ] {
                QImage image = qvariant_cast<QImage>(mp->imageData());
                auto pos = page()->context()->pageRect().center() - image.rect().center();
                QRectF rect = QRectF();
                int r = -1;
                if (adaptImgPosAndRect(page()->scene(), "", image, pos, rect, r))
                {
                    page()->context()->addImageItem(image, pos, rect);
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
            q->pageScene()->clearSelections();
            auto textItem = new TextItem(mp->text());
            auto defaultAttris = page()->borad()->tool(text)->attributions();
            foreach (auto attris, defaultAttris) {
                textItem->setAttributionVar(attris.attri, attris.var, EChanged);
            }

            if (textItemInCenter) {
                textItem->setPos(q->mapToScene(q->viewport()->rect().center()) - QPointF(textItem->itemRect().width(), textItem->itemRect().height()) / 2);
            } else {
                textItem->setPos(letfMenuPopPos);
            }

            q->pageScene()->addPageItem(textItem);
            textItem->setEditing(true);
            UndoRecorder recoder(q->pageScene()->currentTopLayer(), LayerUndoCommand::ChildItemAdded, q->pageScene()->selectedPageItems());
        } else if (mp->hasFormat("pageItems")) {
            q->pageScene()->clearSelections();
            DrawBoardMimeData *data = qobject_cast<DrawBoardMimeData *>(mp);
            UnitList units = data->unit();
            foreach (auto u, units) {
                auto newItem = PageItem::creatItemInstance(u.head.dataType, u);
                newItem->moveBy(10, 10);
                q->pageScene()->addPageItem(newItem);
                q->pageScene()->selectPageItem(newItem);
            }
            UndoRecorder recoder(q->pageScene()->currentTopLayer(), LayerUndoCommand::ChildItemAdded, q->pageScene()->selectedPageItems());
        }
        //5.粘贴数据进入画板,鼠标设置为选择工具
        page()->setCurrentTool(selection);
    }

    void slotOnSelectAll()
    {
        CHECK_CURRENTTOOL_RETURN(q)
        //只选中画布内图元
        QList<QGraphicsItem *> sceneitems =  q->pageScene()->items(q->pageScene()->sceneRect());
        QList<PageItem *> layeritems = q->pageScene()->currentTopLayer()->items();
        QList<PageItem *> selectitem;
        foreach (auto p, layeritems) {
            if (sceneitems.contains(p)) {
                selectitem.push_back(p);
            }
        }
        q->pageScene()->selectPageItem(selectitem);
    }

    void slotOnDelete()
    {
        CHECK_CURRENTTOOL_RETURN(q)
        auto selectedItes = q->pageScene()->selectedPageItems();
        foreach (auto p, selectedItes) {
            q->pageScene()->removePageItem(p);
        }
        UndoRecorder recoder(q->pageScene()->currentTopLayer(), LayerUndoCommand::ChildItemRemoved, selectedItes);
    }

    void updateLayerActions()
    {
        auto selectItemsCount = q->pageScene()->selectedItemCount();
        setLayerMenuActionStatus(selectItemsCount > 0);
    }
    void slotOneLayerUp()
    {
        CHECK_CURRENTTOOL_RETURN(q)
        q->pageScene()->movePageItemsZValue(q->pageScene()->selectedPageItems(), UpItemZ, 1, true);
        updateLayerActions();
    }

    void slotOneLayerDown()
    {
        CHECK_CURRENTTOOL_RETURN(q)
        q->pageScene()->movePageItemsZValue(q->pageScene()->selectedPageItems(), DownItemZ, 1, true);
        updateLayerActions();
    }

    void slotBringToFront()
    {
        CHECK_CURRENTTOOL_RETURN(q)
        q->pageScene()->movePageItemsZValue(q->pageScene()->selectedPageItems(), UpItemZ, -1, true);
        updateLayerActions();
    }

    void slotSendTobackAct()
    {
        CHECK_CURRENTTOOL_RETURN(q)
        q->pageScene()->movePageItemsZValue(q->pageScene()->selectedPageItems(), DownItemZ, -1, true);
        updateLayerActions();
    }

    void slotQuitCutMode()
    {
        page()->setCurrentTool(selection);
    }

    void slotViewZoomIn()
    {
        CHECK_MOSUEACTIVE_RETURN
        q->zoomIn(EViewCenter);
    }

    void slotViewZoomOut()
    {
        CHECK_MOSUEACTIVE_RETURN
        q->zoomOut(EViewCenter);
    }
    Page *page()
    {
        return q->page();
    }
    void slotViewOrignal()
    {
        CHECK_MOSUEACTIVE_RETURN
        q->scale(1.0);
        emit q->signalSetScale(m_scale);
    }



    QPointF alignmentMovPos(const QRectF &fatherRect, const QRectF &childRect,
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
    void updateSelectedItemsAlignment(Qt::AlignmentFlag align)
    {
        //CHECK_CURRENTTOOL_RETURN(q)

        // 获取选择的组合中全部图元
        auto curScene = q->pageScene();
        QList<PageItem *> allItems = curScene->selectedPageItems();

        // 模拟图元移动事件
        PageItemMoveEvent event(PageItemEvent::EMove);
        event.setEventPhase(EChanged);
        // 移动的起点相对位置
        event._oldScenePos = QPointF(0, 0);

        // [0] 没有选中的图元直接返回
        if (!allItems.size()) {
            return;
        }

        // [1] 用于记录保存图元的位置，便于撤销和返回
        QMap<PageItem *, QPointF> startPos;
        QMap<PageItem *, QPointF> endPos;

        // 获取多个图元和单个图元的边界值
        QRectF currSceneRect;
        if (allItems.size() > 1) {
            currSceneRect = curScene->selectionBoundingRect();
            if (align == Qt::AlignVCenter) {align = Qt::AlignHCenter;}
            else if (align == Qt::AlignHCenter) {align = Qt::AlignVCenter;}
        } else {
            currSceneRect = q->sceneRect();
        }

        // [1] 设置对齐坐标
        for (int i = 0; i < allItems.size(); i++) {
            QRectF itemRect = allItems.at(i)->mapRectToScene(allItems.at(i)->itemRect());

            startPos.insert(allItems.at(i), allItems.at(i)->sceneBoundingRect().topLeft());

            event._scenePos = alignmentMovPos(currSceneRect, itemRect, align);

            allItems.at(i)->operating(&event);

            endPos.insert(allItems.at(i), allItems.at(i)->sceneBoundingRect().topLeft());
        }

        // [4] 设置出入栈
        QUndoCommand *addCommand = new CItemsAlignCommand(q->pageScene(), startPos, endPos);
        q->stack()->push(addCommand);
    }

    void itemsVEqulSpaceAlignment()
    {
        // [0] 获取选中的图元
        // QList<CGraphicsItem *> allitems = getSelectedValidItems();
        auto curScene = q->pageScene();
        QList<PageItem *> allitems = curScene->selectedPageItems();

        // [1] 图元个数大于3个才可以进行对齐
        if (allitems.size() < 3) {
            return ;
        }

        // [2] 对图元X进行从小到大排序
        std::sort(allitems.begin(), allitems.end(), yValueSortDES);

        // [3] 统计所有图元占有的高度
        qreal sum_items_height = 0;
        for (int i = 0; i < allitems.size(); i++) {
            sum_items_height += allitems.at(i)->sceneBoundingRect().height();
        }

        // [3] 计算每两个之间的间隔距离
        QRectF scence_BR = curScene->selectionBoundingRect();

        // [4] 用于记录保存图元的位置，便于撤销和返回
        QMap<PageItem *, QPointF> startPos;
        QMap<PageItem *, QPointF> endPos;

        if (sum_items_height > scence_BR.height()) {
            // [5] 按照相邻进行移动位置
            for (int i = 1; i < allitems.size(); i++) {
                startPos.insert(allitems.at(i), allitems.at(i)->sceneBoundingRect().topLeft());
                QPointF endPoint(allitems.at(i)->sceneBoundingRect().left()
                                 , allitems.at(i - 1)->sceneBoundingRect().bottom());
                allitems.at(i)->move(allitems.at(i)->sceneBoundingRect().topLeft(), endPoint);
                endPos.insert(allitems.at(i), allitems.at(i)->sceneBoundingRect().topLeft());
            }
        } else {
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
        QUndoCommand *addCommand = new CItemsAlignCommand(curScene, startPos, endPos);
        q->stack()->push(addCommand);
    }

    void itemsHEqulSpaceAlignment()
    {
        // [0] 获取选中的图元
        // QList<CGraphicsItem *> allitems = getSelectedValidItems();
        auto curScene = q->pageScene();
        QList<PageItem *> allitems = curScene->selectedPageItems();

        // [1] 图元个数大于3个才可以进行对齐
        if (allitems.size() < 3) {
            return ;
        }

        // [2] 对图元X进行从小到大排序
        std::sort(allitems.begin(), allitems.end(), xValueSortDES);

        // [3] 统计所有图元占有的宽度
        qreal sum_items_width = 0;
        for (int i = 0; i < allitems.size(); i++) {
            sum_items_width += allitems.at(i)->sceneBoundingRect().width();
        }

        // [4] 计算每两个之间的间隔距离
        QRectF scence_BR = curScene->selectionBoundingRect();

        // [5] 用于记录保存图元的位置，便于撤销和返回
        QMap<PageItem *, QPointF> startPos;
        QMap<PageItem *, QPointF> endPos;

        if (sum_items_width > scence_BR.width()) {
            // [6] 按照相邻进行移动位置
            for (int i = 1; i < allitems.size(); i++) {
                startPos.insert(allitems.at(i), allitems.at(i)->sceneBoundingRect().topLeft());
                QPointF endPoint(allitems.at(i - 1)->sceneBoundingRect().right()
                                 , allitems.at(i)->sceneBoundingRect().top());
                allitems.at(i)->move(allitems.at(i)->sceneBoundingRect().topLeft(), endPoint);
                endPos.insert(allitems.at(i), allitems.at(i)->sceneBoundingRect().topLeft());
            }
        } else {
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
        QUndoCommand *addCommand = new CItemsAlignCommand(curScene, startPos, endPos);
        q->stack()->push(addCommand);
    }

    void initContextMenu()
    {
        m_contextMenu = new CMenu(q);

        //CMenu enterEvent激活全部action
        m_layerMenu = new DMenu(tr("Layer"), q);

        m_cutAct = new QAction(tr("Cut"), q);
        m_contextMenu->addAction(m_cutAct);
        m_cutAct->setShortcut(QKeySequence::Cut);
        q->addAction(m_cutAct);

        m_copyAct = new QAction(tr("Copy"), q);
        m_contextMenu->addAction(m_copyAct);
        m_copyAct->setShortcut(QKeySequence::Copy);
        q->addAction(m_copyAct);

        m_pasteAct = new QAction(tr("Paste"), q);
        m_pasteActShortCut = new QAction(m_pasteAct);
        m_contextMenu->addAction(m_pasteAct);
        m_pasteActShortCut->setShortcut(QKeySequence::Paste);
        q->addAction(m_pasteAct);
        q->addAction(m_pasteActShortCut);

        m_selectAllAct = new QAction(tr("Select All"), q);
        m_contextMenu->addAction(m_selectAllAct);
        m_selectAllAct->setShortcut(QKeySequence::SelectAll);
        q->addAction(m_selectAllAct);

        m_contextMenu->addSeparator();

        m_deleteAct = new QAction(tr("Delete"), q);
        m_contextMenu->addAction(m_deleteAct);
        m_deleteAct->setShortcut(QKeySequence::Delete);
        q->addAction(m_deleteAct);

        //m_undoAct = m_pUndoStack->createUndoAction(this, tr("Undo"));
        m_undoAct = new QAction(tr("Undo"), q);
        m_undoAct->setEnabled(m_pUndoStack->canUndo());
        connect(m_pUndoStack, SIGNAL(canUndoChanged(bool)),
                m_undoAct, SLOT(setEnabled(bool)));
        m_contextMenu->addAction(m_undoAct);
        m_undoAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));
        q->addAction(m_undoAct);

        //m_redoAct = m_pUndoStack->createRedoAction(this, tr("Redo"));
        m_redoAct = new QAction(tr("Redo"), q);
        m_redoAct->setEnabled(m_pUndoStack->canRedo());
        connect(m_pUndoStack, SIGNAL(canRedoChanged(bool)),
                m_redoAct, SLOT(setEnabled(bool)));
        m_contextMenu->addAction(m_redoAct);
        m_redoAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));
        q->addAction(m_redoAct);
        m_contextMenu->addSeparator();

        m_oneLayerUpAct = new QAction(tr("Raise layer"), q);
        m_layerMenu->addAction(m_oneLayerUpAct);
        m_oneLayerUpAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_BracketRight));
        q->addAction(m_oneLayerUpAct);

        m_oneLayerDownAct = new QAction(tr("Lower layer"), q);
        m_layerMenu->addAction(m_oneLayerDownAct);
        m_oneLayerDownAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_BracketLeft));
        q->addAction(m_oneLayerDownAct);

        m_bringToFrontAct = new QAction(tr("Layer to Top"), q);
        m_layerMenu->addAction(m_bringToFrontAct);
        m_bringToFrontAct->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_BracketRight));
        q->addAction(m_bringToFrontAct);

        m_sendTobackAct = new QAction(tr("Layer to Bottom"), q);
        m_layerMenu->addAction(m_sendTobackAct);
        m_sendTobackAct->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_BracketLeft));
        q->addAction(m_sendTobackAct);

        //    m_cutScence = new QAction(this);
        //    QList<QKeySequence> shortcuts;
        //    shortcuts.append(QKeySequence(Qt::Key_Return));
        //    shortcuts.append(QKeySequence(Qt::Key_Enter));
        //    m_cutScence->setShortcuts(shortcuts);
        //    this->addAction(m_cutScence);

        m_viewZoomInAction = new QAction(q);
        m_viewZoomInAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));
        q->addAction(m_viewZoomInAction);

        m_viewZoomOutAction = new QAction(q);
        m_viewZoomOutAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));
        q->addAction(m_viewZoomOutAction);

        // Qt 无法直接使用 ctrl + (+/=) 这个按键组合
        m_viewZoomOutAction1 = new QAction(q);
        m_viewZoomOutAction1->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Equal));
        q->addAction(m_viewZoomOutAction1);

        m_viewOriginalAction = new QAction(q);
        m_viewOriginalAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
        q->addAction(m_viewOriginalAction);

        m_group = new QAction(tr("Group"), q);
        m_group->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
        q->addAction(m_group);
        m_contextMenu->addAction(m_group);

        m_unGroup = new QAction(tr("Ungroup"), q);
        m_unGroup->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_G));
        q->addAction(m_unGroup);
        m_contextMenu->addAction(m_unGroup);

        // 右键菜单添加对齐方式
        m_alignMenu = new DMenu(tr("Align"), q);
        m_contextMenu->addMenu(m_alignMenu);

        m_itemsLeftAlign = new QAction(tr("Align left"), q); //左对齐
        m_itemsLeftAlign->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_L));
        m_alignMenu->addAction(m_itemsLeftAlign);
        q->addAction(m_itemsLeftAlign);

        m_itemsHCenterAlign = new QAction(tr("Align center horizontally"), q); //水平居中对齐
        m_itemsHCenterAlign->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_H));
        m_alignMenu->addAction(m_itemsHCenterAlign);
        q->addAction(m_itemsHCenterAlign);

        m_itemsRightAlign = new QAction(tr("Align right"), q); //右对齐
        m_itemsRightAlign->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));
        m_alignMenu->addAction(m_itemsRightAlign);
        q->addAction(m_itemsRightAlign);

        m_itemsTopAlign = new QAction(tr("Align top"), q); //顶对齐
        m_itemsTopAlign->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_T));
        m_alignMenu->addAction(m_itemsTopAlign);
        q->addAction(m_itemsTopAlign);

        m_itemsVCenterAlign = new QAction(tr("Align center vertically"), q); //垂直居中对齐
        m_itemsVCenterAlign->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_V));
        m_alignMenu->addAction(m_itemsVCenterAlign);
        q->addAction(m_itemsVCenterAlign);

        m_itemsBottomAlign = new QAction(tr("Align bottom"), q); //底对齐
        m_itemsBottomAlign->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_B));
        m_alignMenu->addAction(m_itemsBottomAlign);
        q->addAction(m_itemsBottomAlign);

        m_itemsHEqulSpaceAlign = m_alignMenu->addAction(tr("Distribute horizontally")); //水平等间距对齐
        m_itemsHEqulSpaceAlign->setObjectName("DistributeHorizontalSpace");
        m_itemsVEqulSpaceAlign = m_alignMenu->addAction(tr("Distribute vertically")); //垂直等间距对齐
        m_itemsVEqulSpaceAlign->setObjectName("DistributeVerticalSpace");

        // 添加对齐菜单
        m_contextMenu->addMenu(m_layerMenu);
        /*m_contextMenu = new QMenu(q);

        itemStyleAction = new QAction(tr("Style"), m_contextMenu);

        m_contextMenu->addAction(itemStyleAction);

        //CMenu enterEvent激活全部action
        m_layerMenu = new QMenu(tr("Layer"), q);

        m_cutAct = new QAction(tr("Cut"), q);
        m_contextMenu->addAction(m_cutAct);
        m_cutAct->setShortcut(QKeySequence::Cut);
        q->addAction(m_cutAct);

        m_copyAct = new QAction(tr("Copy"), q);
        m_contextMenu->addAction(m_copyAct);
        m_copyAct->setShortcut(QKeySequence::Copy);
        q->addAction(m_copyAct);

        m_pasteAct = new QAction(tr("Paste"), q);
        m_pasteActShortCut = new QAction(q);
        m_contextMenu->addAction(m_pasteAct);
        m_pasteActShortCut->setShortcut(QKeySequence::Paste);
        q->addAction(m_pasteAct);
        q->addAction(m_pasteActShortCut);

        m_selectAllAct = new QAction(tr("Select All"), q);
        //m_contextMenu->addAction(m_selectAllAct);
        m_selectAllAct->setShortcut(QKeySequence::SelectAll);
        q->addAction(m_selectAllAct);

        //m_contextMenu->addSeparator();

        m_deleteAct = new QAction(tr("Delete"), q);
        m_contextMenu->addAction(m_deleteAct);
        m_deleteAct->setShortcut(QKeySequence::Delete);
        q->addAction(m_deleteAct);

        m_undoAct = new QAction(tr("Undo"), q);
        m_undoAct->setEnabled(m_pUndoStack->canUndo());
        connect(m_pUndoStack, SIGNAL(canUndoChanged(bool)),
                m_undoAct, SLOT(setEnabled(bool)));
        //m_contextMenu->addAction(m_undoAct);
        m_undoAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));
        q->addAction(m_undoAct);

        m_redoAct = new QAction(tr("Redo"), q);
        m_redoAct->setEnabled(m_pUndoStack->canRedo());
        connect(m_pUndoStack, SIGNAL(canRedoChanged(bool)),
                m_redoAct, SLOT(setEnabled(bool)));
        //m_contextMenu->addAction(m_redoAct);
        m_redoAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));
        q->addAction(m_redoAct);
        //m_contextMenu->addSeparator();

        m_oneLayerUpAct = new QAction(tr("Raise Layer"), q);
        m_layerMenu->addAction(m_oneLayerUpAct);
        m_oneLayerUpAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_BracketRight));
        q->addAction(m_oneLayerUpAct);

        m_oneLayerDownAct = new QAction(tr("Lower Layer"), q);
        m_layerMenu->addAction(m_oneLayerDownAct);
        m_oneLayerDownAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_BracketLeft));
        q->addAction(m_oneLayerDownAct);

        m_bringToFrontAct = new QAction(tr("Layer to Top"), q);
        m_layerMenu->addAction(m_bringToFrontAct);
        m_bringToFrontAct->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_BracketRight));
        q->addAction(m_bringToFrontAct);

        m_sendTobackAct = new QAction(tr("Layer to Bottom"), q);
        m_layerMenu->addAction(m_sendTobackAct);
        m_sendTobackAct->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_BracketLeft));
        q->addAction(m_sendTobackAct);

        m_viewZoomInAction = new QAction(q);
        m_viewZoomInAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));
        q->addAction(m_viewZoomInAction);

        m_viewZoomOutAction = new QAction(q);
        m_viewZoomOutAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));
        q->addAction(m_viewZoomOutAction);

        // Qt 无法直接使用 ctrl + (+/=) 这个按键组合
        m_viewZoomOutAction1 = new QAction(q);
        m_viewZoomOutAction1->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Equal));
        q->addAction(m_viewZoomOutAction1);

        m_viewOriginalAction = new QAction(q);
        m_viewOriginalAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
        q->addAction(m_viewOriginalAction);

        m_group = new QAction(tr("Group"), q);
        m_group->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
        q->addAction(m_group);
        //m_contextMenu->addAction(m_group);

        m_unGroup = new QAction(tr("Ungroup"), q);
        m_unGroup->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_G));
        q->addAction(m_unGroup);
        //m_contextMenu->addAction(m_unGroup);

        // 右键菜单添加对齐方式
        m_alignMenu = new QMenu(tr("Align"), q);
        //m_contextMenu->addMenu(m_alignMenu);

        m_itemsLeftAlign = new QAction(tr("Align left"), q); //左对齐
        m_itemsLeftAlign->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_L));
        m_alignMenu->addAction(m_itemsLeftAlign);
        q->addAction(m_itemsLeftAlign);

        m_itemsHCenterAlign = new QAction(tr("Horizontal centers"), q); //水平居中对齐
        m_itemsHCenterAlign->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_H));
        m_alignMenu->addAction(m_itemsHCenterAlign);
        q->addAction(m_itemsHCenterAlign);

        m_itemsRightAlign = new QAction(tr("Align right"), q); //右对齐
        m_itemsRightAlign->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));
        m_alignMenu->addAction(m_itemsRightAlign);
        q->addAction(m_itemsRightAlign);

        m_itemsTopAlign = new QAction(tr("Align top"), q); //顶对齐
        m_itemsTopAlign->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_T));
        m_alignMenu->addAction(m_itemsTopAlign);
        q->addAction(m_itemsTopAlign);

        m_itemsVCenterAlign = new QAction(tr("Vertical centers"), q); //垂直居中对齐
        m_itemsVCenterAlign->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_V));
        m_alignMenu->addAction(m_itemsVCenterAlign);
        q->addAction(m_itemsVCenterAlign);

        m_itemsBottomAlign = new QAction(tr("Align bottom"), q); //底对齐
        m_itemsBottomAlign->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_B));
        m_alignMenu->addAction(m_itemsBottomAlign);
        q->addAction(m_itemsBottomAlign);

        m_itemsHEqulSpaceAlign = m_alignMenu->addAction(tr("Distribute horizontal space")); //水平等间距对齐
        m_itemsHEqulSpaceAlign->setObjectName("DistributeHorizontalSpace");
        m_itemsVEqulSpaceAlign = m_alignMenu->addAction(tr("Distribute vertical space")); //垂直等间距对齐
        m_itemsVEqulSpaceAlign->setObjectName("DistributeVerticalSpace");

        m_alignMenu->setEnabled(false);

        // 添加对齐菜单
        m_contextMenu->addMenu(m_layerMenu);*/
    }

    void initContextMenuConnection()
    {
        connect(m_cutAct, &QAction::triggered, q, [ = ]() {slotOnCut();});
        connect(m_copyAct, &QAction::triggered, q, [ = ]() {slotOnCopy();});
        connect(m_pasteAct, &QAction::triggered, q, [ = ]() {slotOnPaste(false);});
        connect(m_pasteActShortCut, &QAction::triggered, q, [ = ]() {slotOnPaste();});
        connect(m_selectAllAct, &QAction::triggered, q, [ = ]() {slotOnSelectAll();});
        connect(m_deleteAct, &QAction::triggered, q, [ = ]() {slotOnDelete();});
        connect(m_bringToFrontAct, &QAction::triggered, q, [ = ]() {slotBringToFront();});
        connect(m_sendTobackAct, &QAction::triggered, q, [ = ]() {slotSendTobackAct();});
        connect(m_oneLayerUpAct, &QAction::triggered, q, [ = ]() {slotOneLayerUp();});
        connect(m_oneLayerDownAct, &QAction::triggered, q, [ = ]() {slotOneLayerDown();});
        connect(itemStyleAction, &QAction::triggered, q, [ = ]() {
            if (q->page() != nullptr && q->page()->borad() != nullptr) {
                extern PageItem *_pageSceneSelectItem(PageScene * scene);
                q->page()->borad()->showAttributions(_pageSceneSelectItem(q->pageScene())->attributions(),
                                                     AttributionManager::ItemStyleEditMenu, m_contextMenu->pos());
            }
        });

        q->connect(m_viewZoomInAction, &QAction::triggered, q, [ = ]() {slotViewZoomIn();});
        q->connect(m_viewZoomOutAction, &QAction::triggered, q, [ = ]() {slotViewZoomOut();});
        q->connect(m_viewZoomOutAction1, &QAction::triggered, q, [ = ]() {slotViewZoomOut();});
        q->connect(m_viewOriginalAction, &QAction::triggered, q, [ = ]() {slotViewOrignal();});

        q->connect(m_pUndoStack, &QUndoStack::canUndoChanged, q, [ = ](bool b) {
            q->pageScene()->pageContext()->setDirty(q->isModified());
            m_undoAct->setEnabled(b);
        }, Qt::QueuedConnection);
        q->connect(m_pUndoStack, &QUndoStack::canRedoChanged, q, [ = ](bool b) {
            m_redoAct->setEnabled(b);
        }, Qt::QueuedConnection);

        connect(q, &PageView::selectionChanged, q, [ = ](const QList<PageItem * > &children) {
            const int selectItemsCount = children.size();
            setAlignMenuActionStatus(selectItemsCount);
            setCcdpMenuActionStatus(selectItemsCount > 0);
            //setClipboardStatus();
            setLayerMenuActionStatus(selectItemsCount > 0);

            extern PageItem *_pageSceneSelectItem(PageScene * scene);
            //itemStyleAction->setEnabled(!_pageSceneSelectItem(q->pageScene())->attributions().isEmpty());
        });

        q->connect(m_undoAct, &QAction::triggered, q, [ = ] {
            CHECK_MOSUEACTIVE_RETURN
            //记录出栈信息
            m_pUndoStack->undo();
            q->pageScene()->pageContext()->setDirty(q->isModified());
        });
        q->connect(m_redoAct, &QAction::triggered, q, [ = ] {
            CHECK_MOSUEACTIVE_RETURN
            //记录入栈信息
            m_pUndoStack->redo();
            q->pageScene()->pageContext()->setDirty(q->isModified());
        });

        q->connect(m_group, &QAction::triggered, q, [ = ] {
            CHECK_CURRENTTOOL_RETURN(q)
            auto gp = q->pageScene()->creatGroup(q->pageScene()->selectedPageItems(), nullptr);
            if (gp != nullptr)
            {
                UndoRecorder recorder(q->pageScene()->currentTopLayer(), LayerUndoCommand::ChildGroupAdded,
                                      QList<PageItem *>() << gp << q->pageScene()->selectedPageItems());
                q->pageScene()->clearSelections();
                q->pageScene()->selectPageItem(gp);
            }
        });
        q->connect(m_unGroup, &QAction::triggered, q, [ = ] {
            CHECK_CURRENTTOOL_RETURN(q)
            auto selectedItems = q->pageScene()->selectedPageItems();
            if (!selectedItems.isEmpty())
            {
                foreach (auto item, selectedItems) {
                    if (item->type() == GroupItemType) {
                        auto gp = static_cast<GroupItem *>(item);
                        UndoRecorder recorder(q->pageScene()->currentTopLayer(), LayerUndoCommand::ChildGroupRemoved,
                                              QList<PageItem *>() << gp << gp->items());
                        q->pageScene()->cancelGroup(gp);
                    }
                }
            }
            qWarning() << "unoup root item = " << q->pageScene()->allRootPageItems().count();
        });

        // 连接图元对齐信号
        q->connect(m_itemsLeftAlign, &QAction::triggered, q, [ = ] {
            updateSelectedItemsAlignment(Qt::AlignLeft);
        });
        q->connect(m_itemsHCenterAlign, &QAction::triggered, q, [ = ] {
            updateSelectedItemsAlignment(Qt::AlignHCenter);
        });
        q->connect(m_itemsRightAlign, &QAction::triggered, q, [ = ] {
            updateSelectedItemsAlignment(Qt::AlignRight);
        });
        q->connect(m_itemsTopAlign, &QAction::triggered, q, [ = ] {
            updateSelectedItemsAlignment(Qt::AlignTop);
        });
        q->connect(m_itemsVCenterAlign, &QAction::triggered, q, [ = ] {
            updateSelectedItemsAlignment(Qt::AlignVCenter);
        });
        q->connect(m_itemsBottomAlign, &QAction::triggered, q, [ = ] {
            updateSelectedItemsAlignment(Qt::AlignBottom);
        });
        connect(m_itemsVEqulSpaceAlign, &QAction::triggered, q, [ = ] {
            itemsVEqulSpaceAlignment();
        });

        connect(m_itemsHEqulSpaceAlign, &QAction::triggered, q, [ = ] {
            itemsHEqulSpaceAlignment();
        });
    }

    void initConnection()
    {
        qRegisterMetaType<TextUnitData>("SGraphicsTextUnitData");
        qRegisterMetaType<UnitHead>("SGraphicsUnitHead");
        qRegisterMetaType<Unit>("CGraphicsUnit&");
    }
    void setCcdpMenuActionStatus(bool enable)
    {
        m_copyAct->setEnabled(enable);
        m_cutAct->setEnabled(enable);
        m_deleteAct->setEnabled(enable);
    }

    void setClipboardStatus()
    {
        bool pasteFlag = false;
        QMimeData *mp = page()->borad()->clipBoardShapeData();
        if (nullptr == mp) {
            return;
        }

        if (mp->hasUrls()) {
            QList<QUrl> urls = mp->urls();
            QStringList strList;
            bool support = false;
            foreach (auto url, urls) {
                auto file = url.toLocalFile();
                if (FileHander::isFileLoadable(file)) {
                    pasteFlag = true;
                    break;
                }
            }
        } else if (mp->hasText()) {
            // 判断剪切板数据是否为文字
            pasteFlag = true;
        } else if (mp->hasImage()) {
            pasteFlag = true;
        } else if (mp->hasFormat("pageItems")) {
            pasteFlag = true;
        }
        m_pasteAct->setEnabled(pasteFlag);
    }

    void setLayerMenuActionStatus(bool layervistual)
    {
        m_layerMenu->setEnabled(layervistual);

        bool layerUp = PageScene::isItemsZMovable(q->pageScene()->selectedPageItems(), UpItemZ);
        m_oneLayerUpAct->setEnabled(layerUp);
        m_bringToFrontAct->setEnabled(layerUp);

        bool layerDown = PageScene::isItemsZMovable(q->pageScene()->selectedPageItems(), DownItemZ);
        m_oneLayerDownAct->setEnabled(layerDown);
        m_sendTobackAct->setEnabled(layerDown);
    }

    void setAlignMenuActionStatus(int selectedCount)
    {
        bool acticonvistual = (selectedCount >= 1);
        m_alignMenu->setEnabled(acticonvistual);

        m_itemsLeftAlign->setEnabled(acticonvistual);      //左对齐
        m_itemsHCenterAlign->setEnabled(acticonvistual);   //水平居中对齐
        m_itemsRightAlign->setEnabled(acticonvistual);     //右对齐
        m_itemsTopAlign->setEnabled(acticonvistual);       //顶对齐
        m_itemsVCenterAlign->setEnabled(acticonvistual);   //垂直居中对齐
        m_itemsBottomAlign->setEnabled(acticonvistual);    //底对齐

        m_itemsVEqulSpaceAlign->setEnabled(selectedCount >= 3);//水平等间距对齐
        m_itemsHEqulSpaceAlign->setEnabled(selectedCount >= 3);//垂直等间距对齐
    }



    void showMenu(QMenu *pMenu)
    {
        //判断当前状态是否屏蔽右键菜单
        //CHECK_CURRENTTOOL_RETURN(this)

        QPoint curPos = QCursor::pos();
        //保存当前鼠标位置为文字粘贴的位置
        letfMenuPopPos = q->mapToScene(q->viewport()->mapFromGlobal(QCursor::pos()));

        QSize menSz = pMenu->sizeHint();

        QRect menuRect = QRect(curPos, menSz);


        QScreen *pCurScren = q->window()->windowHandle()->screen();

        if (pCurScren != nullptr) {
            QRect geomeRect = pCurScren->geometry();
            if (!geomeRect.contains(menuRect)) {
                if (menuRect.right() > geomeRect.right()) {
                    int move = menuRect.right() - geomeRect.right();
                    menuRect.adjust(-move, 0, -move, 0);
                }

                if (menuRect.bottom() > geomeRect.bottom()) {
                    int move = menuRect.bottom() - geomeRect.bottom();
                    menuRect.adjust(0, -move, 0, -move);
                }
            }
        }

        pMenu->move(menuRect.topLeft());

        pMenu->show();
    }

    PageView *q;
    qreal m_scale = 1; //记录当前缩放

    QMenu *m_contextMenu = nullptr;//右键菜单
    QAction *m_cutAct = nullptr;              //剪切
    QAction *m_copyAct = nullptr;            //拷贝
    QAction *m_pasteAct = nullptr;           //粘贴
    QAction *m_pasteActShortCut = nullptr;   //快捷键粘贴
    QAction *m_selectAllAct = nullptr;       //全选
    QAction *m_deleteAct = nullptr;          //删除
    QAction *m_undoAct = nullptr;            //撤销
    QAction *m_redoAct = nullptr;            //重做

    QAction *m_group = nullptr;              //组合
    QAction *m_unGroup = nullptr;            //取消组合

    QMenu *m_layerMenu = nullptr;            //图层菜单
    QAction *m_oneLayerUpAct = nullptr;      //向上一层
    QAction *m_oneLayerDownAct = nullptr;    //向下一层
    QAction *m_bringToFrontAct = nullptr;    //置于最顶层
    QAction *m_sendTobackAct = nullptr;      //置于最底层

    QMenu *m_alignMenu = nullptr;
    QAction *m_itemsLeftAlign = nullptr;     //左对齐
    QAction *m_itemsHCenterAlign = nullptr;  //水平居中对齐
    QAction *m_itemsRightAlign = nullptr;    //右对齐
    QAction *m_itemsTopAlign = nullptr;      //顶对齐
    QAction *m_itemsVCenterAlign = nullptr;  //垂直居中对齐
    QAction *m_itemsBottomAlign = nullptr;   //底对齐
    QAction *m_itemsHEqulSpaceAlign = nullptr; //水平等间距对齐
    QAction *m_itemsVEqulSpaceAlign = nullptr; //垂直等间距对齐

    QAction *m_viewZoomInAction = nullptr; // 缩小快捷键
    QAction *m_viewZoomOutAction = nullptr; // 放大快捷键 ctrl + +
    QAction *m_viewZoomOutAction1 = nullptr; // 放大快捷键 ctrl + =
    QAction *m_viewOriginalAction = nullptr;


    QAction *itemStyleAction = nullptr;


    UndoStack *m_pUndoStack = nullptr;

    bool m_isShowContext = true;
    bool m_isStopContinuousDrawing = false;

    bool    _cacheEnable = false;
    QPixmap _cachePixmap;

    QPointF  letfMenuPopPos; // 右键菜单弹出位置
    QPoint  _pressBeginPos;
    QPoint  _recordMovePos;
    bool    _spaceKeyPressed = false;
    QCursor _tempCursor;
};

PageView::PageView(Page *parentPage)
    : QGraphicsView(parentPage),
      PageView_d(new PageView_private(this))
{
    //setViewport(new QOpenGLWidget(this));
    //viewport()->winId();
    //设置状态paintEvent内部做坐标变换
    setOptimizationFlags(IndirectPainting);
    d_PageView()->m_pUndoStack = new UndoStack(this);

    //设置撤销还原最大步数
    d_PageView()->m_pUndoStack->setUndoLimit(30);

    d_PageView()->initContextMenu();
    d_PageView()->initContextMenuConnection();

    //文字右键菜单初始化
    //d_PageView()->initTextContextMenu();
    //d_PageView()->initTextContextMenuConnection();

    d_PageView()->initConnection();

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
    qreal current_scale = qRound(d_PageView()->m_scale * 100) / 100.0;

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
    qreal current_scale = qRound(d_PageView()->m_scale * 100) / 100.0;
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
    qreal multiple = scale / d_PageView()->m_scale;

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
    return d_PageView()->m_scale;
}

void PageView::scaleWithCenter(qreal factor, const QPoint viewPos)
{
    qreal wantedTotalScaled = d_PageView()->m_scale * factor;

    if (wantedTotalScaled < 0.1) {
        wantedTotalScaled = 0.1;
        factor = wantedTotalScaled / d_PageView()->m_scale;
    } else if (wantedTotalScaled > 20) {
        wantedTotalScaled = 20;
        factor = wantedTotalScaled / d_PageView()->m_scale;
    }


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
    d_PageView()->m_scale *= factor;
    emit signalSetScale(d_PageView()->m_scale);
}

UndoStack *PageView::stack() const
{
    return d_PageView()->m_pUndoStack;
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
            pageScene()->sendEvent(proxyItem, &gsEvent);
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
            DrawTool *currentTool = page()->currentTool_p();
            if (currentTool != nullptr && currentTool->status() == DrawTool::EWorking)
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



void PageView::contextMenuEvent(QContextMenuEvent *event)
{
//    CHECK_CURRENTTOOL_RETURN(this)
//    QPointF pos = this->mapToScene(event->pos());
//    QRectF rect = this->scene()->sceneRect();

//    enum {EShowTxtMenu, EShowCommonMenu};
//    int menuTp = EShowCommonMenu;

//    // 0.规避显示条件
//    if (!rect.contains(pos) || !d_PageView()-> m_isShowContext) {
//        return;
//    }

//    // 1.判断确定要显示的菜单
//    auto pProxDrawItem = activeProxDrawItem();
//    PageItem *tmpitem = pProxDrawItem;
//    QList<PageItem *> selectItems = pageScene()->selectedPageItems();
//    QMenu *pMenu = d_PageView()->m_contextMenu;
//    if (selectItems.count() > 0) {
//        if (pProxDrawItem != nullptr && pProxDrawItem->isItemSelected()) {
//            // 1.1 显示文字图元右键菜单
//            if (TextType == pProxDrawItem->type() &&  static_cast<TextItem *>(tmpitem)->isEditState()) {

//                // 文本框右键菜单中剪切和复制只有在选中文字才会激活
//                TextItem *textItem = static_cast<TextItem *>(tmpitem);
//                bool showFalg = textItem->textEditor()->textCursor().hasSelection();
//                d_PageView()->m_textCutAction->setEnabled(showFalg);
//                d_PageView()->m_textCopyAction->setEnabled(showFalg);

//                //根据当前条件,判断是否可执行,未激活的Action应该置灰
//                QTextDocument *doc = textItem->textEditor()->document();
//                if (doc != nullptr) {
//                    d_PageView()->m_textUndoAct->setEnabled(doc->isUndoAvailable());
//                    d_PageView()->m_textRedoAct->setEnabled(doc->isRedoAvailable());
//                }

//                d_PageView()->m_textPasteAction->setEnabled(textItem->textEditor()->canPaste());

//                menuTp = EShowTxtMenu;
//                pMenu = d_PageView()->m_textMenu;
//            }
//        }
//    }
//    // 2.设置菜单中action的状态
//    if (menuTp == EShowTxtMenu) {

//        // 2.1 文本图元对齐显示状态
//        d_PageView()->setTextAlignMenuActionStatus(tmpitem);

//    } else {
//        // 2.3 一般图元的cut，copy，delete，paste操作的状态
//        d_PageView()->setCcdpMenuActionStatus(selectItems.count() > 0);

//        d_PageView()->m_selectAllAct->setEnabled(pageScene()->allPageItems().count() > 0);
//        d_PageView()->m_group->setEnabled(pageScene()->isGroupable(pageScene()->selectedPageItems()));
//        d_PageView()->m_unGroup->setEnabled(pageScene()->isUnGroupable(pageScene()->selectedPageItems()));

//        // 2.4 设置选择的图元个数显示能进行的对齐状态
//        d_PageView()->setAlignMenuActionStatus(selectItems.count() > 0);

//        // 2.5 设置右键菜单图层选项状态
//        d_PageView()->setLayerMenuActionStatus(selectItems.count() > 0);
//    }

//    //判断剪切板状态是否可粘贴
//    d_PageView()->setClipboardStatus();

    //显示菜单
    //d_PageView()->showMenu(pMenu);
    QPointF pos = this->mapToScene(event->pos());
    QRectF rect = this->scene()->sceneRect();
    // 0.规避显示条件
    if (!rect.contains(pos)) {
        return;
    }

    d_PageView()->letfMenuPopPos = mapToScene(viewport()->mapFromGlobal(QCursor::pos()));

    //获取业务图元
    auto allItems = pageScene()->allPageItems();
    int itemCount = allItems.count();

    bool bVisible = (itemCount > 0);
    d_PageView()->setClipboardStatus();
    d_PageView()->setLayerMenuActionStatus(bVisible);
    d_PageView()->m_selectAllAct->setEnabled(bVisible);

    auto selectedItems = pageScene()->selectedPageItems();
    int selectedCount = selectedItems.count();
    bVisible = (selectedCount > 0);
    d_PageView()->m_cutAct->setEnabled(bVisible);
    d_PageView()->m_deleteAct->setEnabled(bVisible);
    d_PageView()->m_copyAct->setEnabled(bVisible);
    d_PageView()->m_undoAct->setEnabled(d_PageView()->m_pUndoStack->canUndo());
    d_PageView()->m_redoAct->setEnabled(d_PageView()->m_pUndoStack->canRedo());
    d_PageView()->m_layerMenu->menuAction()->setEnabled(bVisible);

    d_PageView()->m_group->setEnabled(selectedCount > 1);
    d_PageView()->m_unGroup->setEnabled(selectedCount >= 1 && selectedItems.first()->type() == GroupItemType);
    d_PageView()->setAlignMenuActionStatus(selectedCount);

    QGraphicsView::contextMenuEvent(event);
}

void PageView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
}

void PageView::paintEvent(QPaintEvent *event)
{
    if (!d_PageView()->_cacheEnable) {
        QGraphicsView::paintEvent(event);
    } else {

        QPainter painter(this->viewport());
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        painter.drawPixmap(QPoint(0, 0), d_PageView()->_cachePixmap);

        //绘制缓冲时的额外的前景显示
        DrawTool *pTool = page()->currentTool_p();

        if (pTool != nullptr) {
            pTool->drawMore(&painter, mapToScene(QRect(QPoint(0, 0), d_PageView()->_cachePixmap.size())).boundingRect(), pageScene());
        }
    }
}

void PageView::drawItems(QPainter *painter, int numItems, QGraphicsItem *items[], const QStyleOptionGraphicsItem options[])
{
    QRectF rect = scene()->sceneRect();
    painter->setClipping(true);
    painter->setClipRect(rect);
    QGraphicsView::drawItems(painter, numItems, items, options);
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


//void PageView::pushActionCount()
//{
//    //记录入栈
//    //userActionCount++;
//    drawScene()->pageContext()->setDirty(isModified());
//}


bool PageView::isModified() const
{
    return d_PageView()->m_pUndoStack->userValidStepCount() > 0;
}

PageScene *PageView::pageScene()const
{
    return qobject_cast<PageScene *>(scene());
}

void PageView::setCacheEnable(bool enable, bool fruzzCurFrame)
{
    if (enable == d_PageView()->_cacheEnable)
        return;

    d_PageView()->_cacheEnable = enable;
    if (d_PageView()->_cacheEnable) {

        d_PageView()->_cachePixmap = QPixmap(this->viewport()->size() * devicePixelRatioF());
        d_PageView()->_cachePixmap.fill(QColor(0, 0, 0, 0));


        if (fruzzCurFrame) {
            QPainter painter(&d_PageView()->_cachePixmap);
            painter.setPen(Qt::NoPen);
            painter.setRenderHint(QPainter::Antialiasing);
            this->scene()->render(&painter, QRectF(0, 0, d_PageView()->_cachePixmap.width(), d_PageView()->_cachePixmap.height()),
                                  QRectF(mapToScene(QPoint(0, 0)), mapToScene(QPoint(d_PageView()->_cachePixmap.size().width(), d_PageView()->_cachePixmap.size().height()))), Qt::IgnoreAspectRatio);

            painter.drawRect(viewport()->rect());
        }
    }
    viewport()->update();
}

bool PageView::isCacheEnabled()
{
    return d_PageView()->_cacheEnable;
}

QPixmap &PageView::cachedPixmap()
{
    return d_PageView()->_cachePixmap;
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

PageItem *PageView::activeProxDrawItem()
{
    if (activeProxItem() != nullptr) {
        return dynamic_cast<PageItem *>(activeProxItem()->parentItem());
    }
    return nullptr;
}

QMenu *PageView::menu() const
{
    return d_PageView()->m_contextMenu;
}

void PageView::alignmentSelectItmes(Qt::AlignmentFlag align)
{
    d_PageView()->updateSelectedItemsAlignment(align);
}

void PageView::itemsEqulSpaceAlignment(bool bHor)
{
    if (bHor) {
        d_PageView()->itemsHEqulSpaceAlignment();
    } else {
        d_PageView()->itemsVEqulSpaceAlignment();
    }
}

void PageView::showEvent(QShowEvent *event)
{
    this->setTransformationAnchor(AnchorViewCenter);
    QGraphicsView::showEvent(event);
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
            if (activeProxWidget() == nullptr && qApp->mouseButtons() == Qt::NoButton) {
                d_PageView()->_spaceKeyPressed = true;

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
            if (d_PageView()->_spaceKeyPressed) {
                d_PageView()->_spaceKeyPressed = false;
                page()->blockSettingDrawCursor(false);
//                if (page()->currentTool_p() != nullptr)
//                    page()->currentTool_p()->refresh();
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
            d_PageView()->_pressBeginPos = event->pos();
            d_PageView()->_recordMovePos = d_PageView()->_pressBeginPos;
            if (d_PageView()->_spaceKeyPressed && event->button() == Qt::LeftButton) {
                finished       = true;
            }
        } else if (e->type() == QEvent::MouseMove) {
            QMouseEvent *event = dynamic_cast<QMouseEvent *>(e);
            {
                if (d_PageView()->_spaceKeyPressed && event->buttons() == Qt::LeftButton) {
                    //移动卷轴
                    PageScene *pScene = qobject_cast<PageScene *>(scene());
                    if (pScene != nullptr) {
                        pScene->blockScene(true);
                    }
                    QPointF mov = event->pos() - d_PageView()->_recordMovePos;
                    int horValue = this->horizontalScrollBar()->value() - qRound(mov.x());
                    //qDebug() << "old hor value = " << this->horizontalScrollBar()->value() << "new hor value = " << horValue;
                    this->horizontalScrollBar()->setValue(qMin(qMax(this->horizontalScrollBar()->minimum(), horValue), this->horizontalScrollBar()->maximum()));

                    int verValue = this->verticalScrollBar()->value() - qRound(mov.y());
                    //qDebug() << "mov.y() = " << mov.y() << "cur value = " << this->verticalScrollBar()->value() << "wanted value = " << verValue << "max = " << this->verticalScrollBar()->maximum();
                    this->verticalScrollBar()->setValue(qMin(qMax(this->verticalScrollBar()->minimum(), verValue), this->verticalScrollBar()->maximum()));

                    if (pScene != nullptr) {
                        pScene->blockScene(false);
                    }

                    finished = true;
                }
                d_PageView()->_recordMovePos = event->pos();
            }
        } else if (e->type() == QEvent::Leave) {
            if (pageScene() != nullptr) {
                DrawTool *pTool =  page()->currentTool_p();
                if (pTool != nullptr) {
                    if (pTool->isWorking()) {
                        QMetaObject::invokeMethod(this, [ = ]() {
                            pTool->interrupt();
                        }, Qt::QueuedConnection);
                    }
                }
            }
        }
        return finished;
    }
    return QGraphicsView::eventFilter(o, e);
}

bool PageView::viewportEvent(QEvent *event)
{
    if (event->type() == QEvent::Leave) {
        if (page()->currentTool_p() != nullptr) {
            if (pageScene() != nullptr) {
                auto e = ToolSceneEvent::fromQEvent_single(event, pageScene());
                page()->currentTool_p()->leaveSceneEvent(&e);
            }
        }
    } else if (event->type() == QEvent::Enter) {
        if (page()->currentTool_p() != nullptr && pageScene() != nullptr) {
            auto e = ToolSceneEvent::fromQEvent_single(event, pageScene());
            page()->currentTool_p()->enterSceneEvent(&e);
        }
    }
    return QGraphicsView::viewportEvent(event);
}



