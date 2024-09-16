// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "drawitemtool.h"
#include "pagescene.h"
#include "cundoredocommand.h"
#include "rasteritem.h"
#include "pageview.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QToolButton>

class DrawItemTool::DrawItemTool_private
{
    explicit DrawItemTool_private(DrawItemTool *q): _q(q)
    {

    }

//    void saveZ(PageScene *scene, PageItem *item)
//    {
//        _tempZs.clear();
//        auto invokedItems = scene->allRootPageItems(EAesSort);
//        for (int i = 0; i < invokedItems.size(); ++i) {
//            auto pItem = invokedItems.at(i);
//            saveItemZValue(pItem);
//        }
//    }

//    void moveItemsToTop(PageScene *scene, const QList<PageItem *> items)
//    {
//        scene->movePageItemsZValue(items, UpItemZ, -1);
//    }

//    void restoreZ()
//    {
//        for (auto it = _tempZs.begin(); it != _tempZs.end(); ++it) {
//            it.key()->setPageZValue(it.value());
//        }
//        _tempZs.clear();
//    }

//    void saveItemZValue(PageItem *pItem)
//    {
//        if (pItem->isBzItem()) {
//            _tempZs.insert(pItem, pItem->pageZValue());
//        } else if (pItem->isPageGroup()) {
//            auto items = static_cast<GroupItem *>(pItem)->items();
//            foreach (auto p, items) {
//                saveItemZValue(p);
//                _tempZs.insert(pItem, pItem->pageZValue());
//            }
//        }
//    }

    void cachItemZ(PageItem *item)
    {
        _tempZs.insert(item, item->zValue());
        auto scene = item->pageScene();
        if (scene != nullptr)
            scene->movePageItemsZValue(QList<PageItem *>() << item, UpItemZ, -1);
    }
    void restoreItemZ()
    {
        for (auto it = _tempZs.begin(); it != _tempZs.end(); ++it) {
            it.key()->setPageZValue(it.value());
        }
        _tempZs.clear();
    }



    DrawItemTool *_q;

    QMap<PageItem *, qreal> _tempZs;

    struct SItemInfo {
        SItemInfo(PageItem *p, bool isnew = true): item(p), isCreatedNew(isnew) {}
        PageItem *item = nullptr;
        bool isCreatedNew = true;
    };
    QMap<int, SItemInfo> creatingInfo;

    bool autoAddToStack = true;
    bool autoCheckMoved = true;
    bool drawOnTop = true;

    QSet<PageItem *> newAddedItems;
    PageItem *drawObjectItem = nullptr;

    friend class DrawItemTool;
};
DrawItemTool::DrawItemTool(QObject *parent): DrawTool(parent),
    DrawItemTool_d(new DrawItemTool_private(this))
{
    setMaxTouchPoint(10);
    setClearSelectionOnActived(true);
}

void DrawItemTool::pressOnScene(ToolSceneEvent *event)
{
    auto lists = event->scene()->items();
    auto item = drawItemStart(event);
    if (item != nullptr) {
        bool isCreatedItem = !lists.contains(item);

        if (isCreatedItem) {
            event->scene()->addPageItem(item);
            if (event->scene() != item->pageScene())
                event->scene()->addPageItem(item);
            d_DrawItemTool()->newAddedItems.insert(item);
        }
        foreach (auto stri, attributions()) {
            item->setAttributionVar(stri.attri, stri.var, EChanged);
        }
        if (!isCreatedItem) {
            if (event->isFirstOne()) {
                d_DrawItemTool()->drawObjectItem = item;
                if (isAutoPushToUndoStack()) {
                    UndoStack::recordUndo(item);
                }
                //置顶
                if (isDrawItemOnTop()) {
                    //d_DrawItemTool()->saveZ(event->scene());
                    //d_DrawItemTool()->moveItemsToTop(event->scene(), event->scene()->selectedPageItems());
                    d_DrawItemTool()->cachItemZ(item);
                }
            }
        }
        d_DrawItemTool()->creatingInfo.insert(event->uuid(), DrawItemTool_private::SItemInfo(item, isCreatedItem));

    }
    if (minMoveUpdateDistance() == 0)
        /*moveOnScene*/toolMoveOnScene(event);
}

void DrawItemTool::moveOnScene(ToolSceneEvent *event)
{
    auto find = d_DrawItemTool()->creatingInfo.find(event->uuid());
    if (find != d_DrawItemTool()->creatingInfo.end()) {
        auto pageItem = find.value().item;
        drawItemUpdate(event, pageItem);
        pageItem->updateShape();
        pageItem->update();
    } else {
        drawItemHover(event);
    }
}

void DrawItemTool::releaseOnScene(ToolSceneEvent *event)
{
    auto find = d_DrawItemTool()->creatingInfo.find(event->uuid());
    if (find != d_DrawItemTool()->creatingInfo.end()) {
        auto pageItem = find.value().item;
        drawItemFinish(event, pageItem);

        if (find.value().isCreatedNew) {
            if (isAutoCheckItemDeleted() && !event->haveMoved()) {
                pageItem->pageScene()->removePageItem(pageItem);
                d_DrawItemTool()->newAddedItems.remove(pageItem);
                delete pageItem;
                pageItem = nullptr;
            }
            if (event->isFinalOne()) {
                if (!d_DrawItemTool()->newAddedItems.isEmpty()) {

                    event->scene()->clearSelections();
                    foreach (auto p, d_DrawItemTool()->newAddedItems) {
                        event->scene()->selectPageItem(static_cast<PageItem *>(p));
                    }

                    UndoRecorder block(isAutoPushToUndoStack() ? event->scene()->currentTopLayer() : nullptr, LayerUndoCommand::ChildItemAdded,
                                       d_DrawItemTool()->newAddedItems.toList());

                    d_DrawItemTool()->newAddedItems.clear();
                }
            }
        } else {
            if (event->isFinalOne()) {
                if (isDrawItemOnTop()) {
                    d_DrawItemTool()->restoreItemZ();
                }
                if (isAutoPushToUndoStack()) {
                    UndoStack::recordRedo(d_DrawItemTool()->drawObjectItem);
                    UndoStack::finishRecord(event->view()->stack());
                }
                d_DrawItemTool()->drawObjectItem = nullptr;
            }
        }
        d_DrawItemTool()->creatingInfo.erase(find);

        if (pageItem != nullptr)
            pageItem->updateShape();
    }
}

bool DrawItemTool::isAutoPushToUndoStack() const
{
    return d_DrawItemTool()->autoAddToStack;
}

void DrawItemTool::setAutoPushToundoStack(bool b)
{
    d_DrawItemTool()->autoAddToStack = b;
}

bool DrawItemTool::isAutoCheckItemDeleted() const
{
    return d_DrawItemTool()->autoCheckMoved;
}

void DrawItemTool::setAutoCheckItemDeleted(bool b)
{
    d_DrawItemTool()->autoCheckMoved = b;
}

bool DrawItemTool::isDrawItemOnTop() const
{
    return d_DrawItemTool()->drawOnTop;
}

void DrawItemTool::setDrawItemOnTop(bool b)
{
    d_DrawItemTool()->drawOnTop = b;
}

void DrawItemTool::drawItemFinish(ToolSceneEvent *event, PageItem *pItem)
{
    Q_UNUSED(event)
    Q_UNUSED(pItem)
}

void DrawItemTool::drawItemHover(ToolSceneEvent *event)
{
    Q_UNUSED(event)
}

void DrawItemTool::clearPointRecording()
{
    d_DrawItemTool()->creatingInfo.clear();
    DrawTool::clearPointRecording();
}

void DrawItemTool::cursorScale(QPixmap &s_cur)
{
    //获取屏幕分辨率
    QDesktopWidget *desktopWidget = QApplication::desktop();
    //需要乘以系统缩放系数才是最终的大小
    s_cur = s_cur.scaled(qRound(s_cur.width() * desktopWidget->devicePixelRatioF()),
                         qRound(s_cur.height() * desktopWidget->devicePixelRatioF()), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

DrawComItemTool::DrawComItemTool(QObject *parent): DrawItemTool(parent)
{
    auto m_selectBtn = toolButton();
    m_selectBtn->setIconSize(QSize(48, 48));
    m_selectBtn->setFixedSize(QSize(37, 37));
    m_selectBtn->setCheckable(true);
    connect(m_selectBtn, &QToolButton::toggled, m_selectBtn, [ = ](bool b) {
        QIcon icon       = QIcon::fromTheme("ddc_choose tools_normal", QIcon(RCC_DRAWBASEPATH + "ddc_choose tools_normal_48px.svg"));
        QIcon activeIcon = QIcon::fromTheme("ddc_choose tools_active", QIcon(RCC_DRAWBASEPATH + "ddc_choose tools_active_48px.svg"));
        m_selectBtn->setIcon(b ? activeIcon : icon);
    });
    m_selectBtn->setIcon(QIcon::fromTheme("ddc_choose tools_normal", QIcon(RCC_DRAWBASEPATH + "ddc_choose tools_normal_48px.svg")));
}

int DrawComItemTool::toolType() const
{
    return EDrawComItemTool;
}

SAttrisList DrawComItemTool::attributions()
{
    SAttrisList attris;

    //0-5
    attris << SAttri(9999, 0);

    return attris;
}

void DrawComItemTool::setAttributionVar(int attri, const QVariant &var, int phase, bool autoCmdStack)
{
    Q_UNUSED(phase)
    Q_UNUSED(autoCmdStack)
    if (attri == 9999) {
        int value = var.toInt();
        if (value >= 0 && value < 6)
            setCurrent(var.toInt() + 65537);
    }
}

void DrawComItemTool::setCurrent(int itemType)
{
    qWarning() << "setCurrent ====== " << itemType;
    currentTp = itemType;
}

int DrawComItemTool::current() const
{
    return currentTp;
}

PageItem *DrawComItemTool::drawItemStart(ToolSceneEvent *event)
{
    if (event->isNormalPressed()) {
        auto p = PageItem::creatItemInstance(currentTp);
        return p;
    }
    return nullptr;
}

void DrawComItemTool::drawItemUpdate(ToolSceneEvent *event, PageItem *p)
{
    RectBaseItem *pRectItem = dynamic_cast<RectBaseItem *>(p);

    if (nullptr != pRectItem) {
        QPointF startPos = event->firstEvent()->pos();
        QPointF pointMouse = event->pos();
        bool shiftKeyPress = event->keyboardModifiers() & Qt::ShiftModifier;
        bool altKeyPress = event->keyboardModifiers() & Qt::AltModifier;
        QRectF resultRect;

        if (shiftKeyPress && !altKeyPress) {
            QPointF resultPoint = pointMouse;
            qreal w = resultPoint.x() - startPos.x();
            qreal h = resultPoint.y() - startPos.y();
            qreal abslength = abs(w) - abs(h);
            if (abslength >= 0.1) {
                if (h >= 0) {
                    resultPoint.setY(startPos.y() + abs(w));
                } else {
                    resultPoint.setY(startPos.y() - abs(w));
                }

            } else {
                if (w >= 0) {
                    resultPoint.setX(startPos.x() + abs(h));
                } else {
                    resultPoint.setX(startPos.x() - abs(h));
                }
            }
            QRectF rectF(startPos, resultPoint);
            resultRect = rectF.normalized();

        }
        //按下ALT键
        else if (!shiftKeyPress && altKeyPress) {

            QPointF point1 = pointMouse;
            QPointF centerPoint = startPos;
            QPointF point2 = 2 * centerPoint - point1;
            QRectF rectF(point1, point2);
            resultRect = rectF.normalized();
        }
        //ALT SHIFT都按下
        else if (shiftKeyPress && altKeyPress) {
            QPointF resultPoint = pointMouse;
            qreal w = resultPoint.x() - startPos.x();
            qreal h = resultPoint.y() - startPos.y();
            qreal abslength = abs(w) - abs(h);
            if (abslength >= 0.1) {
                if (h >= 0) {
                    resultPoint.setY(startPos.y() + abs(w));
                } else {
                    resultPoint.setY(startPos.y() - abs(w));
                }

            } else {
                if (w >= 0) {
                    resultPoint.setX(startPos.x() + abs(h));
                } else {
                    resultPoint.setX(startPos.x() - abs(h));
                }
            }
            QPointF point1 = resultPoint;
            QPointF centerPoint = startPos;
            QPointF point2 = 2 * centerPoint - point1;
            QRectF rectF(point1, point2);
            resultRect = rectF.normalized();
        } else {
            QPointF resultPoint = pointMouse;
            QRectF rectF(startPos, resultPoint);
            resultRect = rectF.normalized();
        }

        pRectItem->setRect(resultRect);
        event->setAccepted(true);
    }
}

void DrawComItemTool::drawItemFinish(ToolSceneEvent *event, PageItem *pItem)
{
    Q_UNUSED(event)
    Q_UNUSED(pItem)
}
