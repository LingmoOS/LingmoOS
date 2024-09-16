// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "erasertool.h"
#include "cundoredocommand.h"
#include "pagescene.h"
#include "pageview.h"
#include "drawboard.h"

#include <QApplication>
#include <QToolButton>

class EraserTool::EraserTool_private
{
public:
    explicit EraserTool_private(EraserTool *qq): q(qq) {}

    EraserTool *q;

    QMap<PageScene *, RasterItem *> rasters;

    QMap<int, RasterPaintCompsitor> rasterPaintCompsitor;

    PageScene *currentScene = nullptr;

    bool m_leaved = false;

    QMap<int, QList<QLineF> > _activeTempLines;
};
EraserTool::EraserTool(QObject *parent)
    : DrawItemTool(parent), EraserTool_d(new EraserTool_private(this))
{
    setCursor(QCursor(Qt::BlankCursor));
    setContinued(true);
    setClearSelectionOnActived(false);

    auto m_eraserBtn = toolButton();
    m_eraserBtn->setShortcut(QKeySequence(QKeySequence(Qt::Key_E)));
    setWgtAccesibleName(m_eraserBtn, "Eraser tool button");
    m_eraserBtn->setToolTip(tr("Eraser (E)"));
    m_eraserBtn->setIconSize(TOOL_ICON_RECT);
    m_eraserBtn->setFixedSize(TOOL_BUTTON_RECT);
    m_eraserBtn->setCheckable(true);
    m_eraserBtn->setIcon(QIcon::fromTheme("rubber_normal"));
    setTouchSensitiveRadius(0);
    setEnable(false);
}

EraserTool::~EraserTool()
{

}

int EraserTool::toolType() const
{
    return eraser;
}

SAttrisList EraserTool::attributions()
{
    SAttrisList result;
    result << defaultAttriVar(EEraserWidth);
    return result;
}

PageItem *EraserTool::drawItemStart(ToolSceneEvent *event)
{
    PageItem *pItem = getRasterItem(event);

    return pItem;
}

void EraserTool::drawItemUpdate(ToolSceneEvent *event, PageItem *pItem)
{
    auto layer = dynamic_cast<RasterItem *>(pItem);
    auto picture = paintTempErasePen(event, layer);

    if (layer != nullptr) {
//        bool blockStatus = layer->isBlocked();
//        layer->setBlocked(false);
        layer->addPaint(picture, false, false);
//        layer->setBlocked(blockStatus);
    }
    d_EraserTool()->rasterPaintCompsitor[event->uuid()].merge(picture);

//    // 获取到当前顶层图元
//    auto currenItem = event->scene()->topPageItem(event->pos(), true,
//                                                  event->eventType() == ToolSceneEvent::ETouchEvent ? drawBoard()->touchFeelingEnhanceValue() : 0);

//    // 判断顶层图元是否发生变化
//    bool eraserActive = (layer == currenItem) ? true : false;

//    // 更新鼠标光标
//    event->scene()->page()->setDrawCursor(eraserActive ? cursor() : QCursor(Qt::ForbiddenCursor));

    event->view()->viewport()->update();
}

void EraserTool::drawItemFinish(ToolSceneEvent *event, PageItem *pItem)
{
    d_EraserTool()->_activeTempLines.find(event->uuid())->clear();
    auto picture = d_EraserTool()->rasterPaintCompsitor.take(event->uuid());

    auto pLayer = dynamic_cast<RasterItem *>(pItem);
    if (pLayer != nullptr) {
//        bool blockStatus = pLayer->isBlocked();
//        pLayer->setBlocked(false);
        pLayer->addPaint(picture.result(), true, false, false);
//        pLayer->setBlocked(blockStatus);
    }
    if (event->isFinalOne()) {
        d_EraserTool()->rasters.remove(event->scene());
    }

    //setCursor(cursor());

    event->view()->viewport()->update();
}

void EraserTool::drawItemHover(ToolSceneEvent *event)
{
    setCursor(Qt::BlankCursor);
    event->view()->viewport()->update();
}

int EraserTool::minMoveUpdateDistance()
{
    return 0;
}

void EraserTool::onCurrentPageChanged(Page *newPage)
{
    auto newScene = (newPage == nullptr ? nullptr : newPage->scene());

    if (newScene != d_EraserTool()->currentScene) {
        if (d_EraserTool()->currentScene != nullptr) {
            disconnect(d_EraserTool()->currentScene, &PageScene::selectionChanged, this, &EraserTool::_onSceneSelectionChanged);
        }

        d_EraserTool()->currentScene = newScene;

        if (d_EraserTool()->currentScene != nullptr) {
            connect(d_EraserTool()->currentScene, &PageScene::selectionChanged, this, &EraserTool::_onSceneSelectionChanged);
            _onSceneSelectionChanged(d_EraserTool()->currentScene->selectedPageItems());
        }
    }
}

void EraserTool::onStatusChanged(DrawTool::EStatus oldStatus, DrawTool::EStatus nowStatus)
{
    auto scene = currentPage() != nullptr ? currentPage()->scene() : nullptr;

    if (scene == nullptr)
        return;

    if (oldStatus == EIdle && nowStatus == EReady) {
        //qApp->installEventFilter(this);
        scene->update();
        d_EraserTool()->m_leaved = false;
    } else if (oldStatus == EReady && nowStatus == EIdle) {
        if (drawBoard()->currentPage() != nullptr)
            drawBoard()->currentPage()->view()->viewport()->update();
        //qApp->removeEventFilter(this);
        d_EraserTool()->m_leaved = true;
    }
}

void EraserTool::enterSceneEvent(ToolSceneEvent *event)
{
    d_EraserTool()->m_leaved = false;
    event->view()->viewport()->update();
}

void EraserTool::leaveSceneEvent(ToolSceneEvent *event)
{
    DrawItemTool::leaveSceneEvent(event);
    d_EraserTool()->m_leaved = true;
    event->view()->viewport()->update();
}

void EraserTool::drawMore(QPainter *painter, const QRectF &rect, PageScene *scene)
{
    Q_UNUSED(rect)
    DrawItemTool::drawMore(painter, rect, scene);

    if (d_EraserTool()->m_leaved)
        return;

    // 自绘鼠标光标
    auto view = scene->firstPageView();

    // 如果鼠标选中的位置不是图片就返回
    if (view->viewport()->cursor().shape() == Qt::ForbiddenCursor) {
        return;
    }

    auto posInViewport = view->viewport()->mapFromGlobal(QCursor::pos());

    if (!view->viewport()->rect().contains(posInViewport)) {
        return;
    }

    auto width = scene->firstPageView()->page()->defaultAttriVar(EEraserWidth).toInt();
    painter->save();
    painter->setClipping(false);
    auto pos =  view->mapToScene(posInViewport);

    qreal half = qMax(width / 2, 1);
    auto rectEllipse = QRectF(pos - QPointF(half, half), pos + QPointF(half, half));
    QPen pen(QColor(0, 0, 0, 200));
    pen.setWidthF(2 / view->getScale());
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(rectEllipse);

    pen.setColor(QColor(255, 255, 255, 100));
    pen.setWidthF(1 / view->getScale());
    painter->setPen(pen);
    auto rectEllipse2 = QRectF(pos - QPointF(half + 2 / view->getScale(), half + 2 / view->getScale()),
                               pos + QPointF(half + 2 / view->getScale(), half + 2 / view->getScale()));
    painter->drawEllipse(rectEllipse2);
    painter->restore();
}

RasterPaint EraserTool::paintTempErasePen(ToolSceneEvent *event, RasterItem *pItem)
{
    PageView *pView = event->scene()->firstPageView();
    RasterPaint picture;
    QPainter painter(&picture);

    QPointF  prePos = pItem->mapFromScene(event->lastEvent()->pos()) ;
    QPointF  pos = pItem->mapFromScene((event->pos())) ;
    QLineF line(prePos, pos);

    auto &tempLines = d_EraserTool()->_activeTempLines[event->uuid()];
    tempLines << line;

    QPainterPath  drawPath;

    if (tempLines.size() > 2) {
        tempLines.removeFirst();
    }

    if (tempLines.size() >= 2) {
        QPointF line1Center = tempLines.first().center();
        QLineF line2 = tempLines.last();
        QPointF line2Center = line2.center();
        drawPath.moveTo(line1Center);
        drawPath.cubicTo(line1Center, line2.p1(), line2Center);
    }


    QPen pen;
    pen.setWidthF(pView->page()->defaultAttriVar(EEraserWidth).value<qreal>());
    pen.setCapStyle(Qt::RoundCap);
    pen.setColor(Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.setPen(pen);
    //painter.drawLine(line);

    if (line.p1() == line.p2() && tempLines.size() <= 1)//点击时需要绘制
        painter.drawLine(line);
    else
        painter.drawPath(drawPath);

    painter.end();

    return picture;
}

RasterItem *EraserTool::getRasterItem(ToolSceneEvent *event)
{
    RasterItem *pItem = nullptr;
    auto findIt = d_EraserTool()->rasters.find(event->scene());
    if (d_EraserTool()->rasters.find(event->scene()) == d_EraserTool()->rasters.end()) {
        auto scene = event->scene();
        RasterItem *rasterItem = nullptr;
        auto selectedItems = scene->items(event->pos());
        if (selectedItems.count() >= 1) {
            for (auto item : selectedItems) {
                auto pSelected = dynamic_cast<RasterItem *>(item);
                if (pSelected != nullptr) {
                    rasterItem = pSelected;
                    break;
                }
            }
        }

        if (rasterItem != nullptr) {
            scene->selectPageItem(rasterItem);
            d_EraserTool()->rasters.insert(scene, rasterItem);
        }

        pItem = rasterItem;
    } else {
        pItem = findIt.value();
    }

    return pItem;
}

void EraserTool::_onSceneSelectionChanged(const QList<PageItem *> &selectedItems)
{
    if (selectedItems.count() == 1) {
        auto first = selectedItems.first();
        setEnable(first->type() == RasterItemType);
    } else {
        setEnable(false);
    }
}
