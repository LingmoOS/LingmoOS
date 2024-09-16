// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "blurtool.h"
#include "pagescene.h"
#include "pagecontext.h"
#include "pageview.h"
#include "itemgroup.h"
#include "rasteritem.h"
#include "global.h"

#include <QGraphicsView>
#include <QToolButton>
#include <QDebug>

class BlurTool::BlurTool_private
{
public:
    explicit BlurTool_private(BlurTool *qc): q(qc) {}

    BlurTool *q;

    int blurTp = 0;
    int blurWidth = 30;
    int blurRadius = 10;

    QMap<PageScene *, RasterItem *> _layers;

    QMap<RasterItem *, QImage> tempImages;

    QMap<int, QPainterPath> blurActivedOrgPath;
    QMap<int, QPainterPath> blurActivedStrokerPath;

    bool m_leaved = false;
    PageScene *currentScene = nullptr;
};

BlurTool::BlurTool(QObject *parent): DrawItemTool(parent), BlurTool_d(new BlurTool_private(this))
{
    setClearSelectionOnActived(false);
    setCursor(QCursor(Qt::BlankCursor));
    setContinued(true);
    auto m_blurBtn = toolButton();
    m_blurBtn->setShortcut(QKeySequence(QKeySequence(Qt::Key_B)));
    setWgtAccesibleName(m_blurBtn, "Blur tool button");
    m_blurBtn->setToolTip(tr("Blur (B)"));
    m_blurBtn->setIconSize(TOOL_ICON_RECT);
    m_blurBtn->setFixedSize(TOOL_BUTTON_RECT);
    m_blurBtn->setCheckable(true);
    m_blurBtn->setIcon(QIcon::fromTheme("vague_normal"));
    connect(m_blurBtn, &QToolButton::toggled, m_blurBtn, [ = ](bool b) {
        QIcon icon       = QIcon::fromTheme("vague_normal");
        QIcon activeIcon = QIcon::fromTheme("vague_highlight");
        m_blurBtn->setIcon(b ? activeIcon : icon);
    });
}

int BlurTool::toolType() const
{
    return blur;
}

SAttrisList BlurTool::attributions()
{
    SAttrisList result;
    result << defaultAttriVar(EEBlurStyle)
           << defaultAttriVar(EBlurAttri);
    return result;
}

int BlurTool::blurType() const
{
    return d_BlurTool()->blurTp;
}

void BlurTool::setBlurType(int tp)
{
    d_BlurTool()->blurTp = tp;
}

void BlurTool::setAttributionVar(int attri, const QVariant &var, int phase, bool autoCmdStack)
{
    //设置模糊类型
    if (attri == EEBlurStyle)
        setBlurType(var.toInt());
}

PageItem *BlurTool::drawItemStart(ToolSceneEvent *event)
{
    RasterItem *pItem = getRasterItem(event);

    if (nullptr != pItem) {
        QPointF  pos = pItem->mapFromScene((event->pos()));
        d_BlurTool()->blurActivedOrgPath[event->uuid()].moveTo(pos);
    }

    return pItem;
}

void BlurTool::drawItemUpdate(ToolSceneEvent *event, PageItem *pItem)
{
    auto rasterItem = dynamic_cast<RasterItem *>(pItem);
    //点下时进行鼠标样式的初始化:鼠标样式是否设置为模糊样式的条件
    bool cursorBlurEnable = (pItem != nullptr) && (pItem->sceneBoundingRect().contains(event->pos()));
    drawBoard()->currentPage()->setDrawCursor(cursorBlurEnable ? cursor() : QCursor(Qt::ForbiddenCursor));
    creatBlurSrokerPaths(event, rasterItem);
    event->view()->viewport()->update();
}

void BlurTool::drawItemFinish(ToolSceneEvent *event, PageItem *pItem)
{
    d_BlurTool()->blurActivedOrgPath.remove(event->uuid());
    auto blurStrokerPath = d_BlurTool()->blurActivedStrokerPath.take(event->uuid());


    RasterItem *rst = static_cast<RasterItem *>(pItem);
    blurStrokerPath = rst->imgTrans().map(blurStrokerPath);
    RasterBlurCmd *cmd = new RasterBlurCmd(blurStrokerPath, d_BlurTool()->blurTp);
    rst->appendComand(cmd, true, false);

    if (event->isFinalOne()) {
        d_BlurTool()->_layers.remove(event->scene());
        d_BlurTool()->tempImages.remove(rst);
        setCursor(cursor());
    }
    event->view()->viewport()->update();
}

void BlurTool::drawItemHover(ToolSceneEvent *event)
{
    setCursor(Qt::BlankCursor);
    event->view()->viewport()->update();
}

void BlurTool::onStatusChanged(EStatus oldStatus, EStatus nowStatus)
{
    auto scene = currentPage() != nullptr ? currentPage()->scene() : nullptr;

    if (scene == nullptr)
        return;

    if (oldStatus == EIdle && nowStatus == EReady) {
        //qApp->installEventFilter(this);
        scene->update();
        d_BlurTool()->m_leaved = false;
    } else if (oldStatus == EReady && nowStatus == EIdle) {
        if (drawBoard()->currentPage() != nullptr)
            drawBoard()->currentPage()->view()->viewport()->update();
        //qApp->removeEventFilter(this);
        d_BlurTool()->m_leaved = true;
    }
}

void BlurTool::onCurrentPageChanged(Page *newPage)
{
    auto newScene = (newPage == nullptr ? nullptr : newPage->scene());

    if (newScene != d_BlurTool()->currentScene) {
        if (d_BlurTool()->currentScene != nullptr) {
            disconnect(d_BlurTool()->currentScene, &PageScene::selectionChanged, this, &BlurTool::_onSceneSelectionChanged);
        }

        d_BlurTool()->currentScene = newScene;

        if (newScene != nullptr) {
            connect(d_BlurTool()->currentScene, &PageScene::selectionChanged, this, &BlurTool::_onSceneSelectionChanged);
            _onSceneSelectionChanged(newScene->selectedPageItems());
        }
    }
}

int BlurTool::minMoveUpdateDistance()
{
    return 0;
}

void BlurTool::enterSceneEvent(ToolSceneEvent *event)
{
    d_BlurTool()->m_leaved = false;
    event->view()->viewport()->update();
}

void BlurTool::leaveSceneEvent(ToolSceneEvent *event)
{
    DrawItemTool::leaveSceneEvent(event);
    d_BlurTool()->m_leaved = true;
    event->view()->viewport()->update();
}

void BlurTool::drawMore(QPainter *painter, const QRectF &rect, PageScene *scene)
{
    Q_UNUSED(rect)
    DrawItemTool::drawMore(painter, rect, scene);
    if (d_BlurTool()->m_leaved)
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

    auto width = scene->firstPageView()->page()->defaultAttriVar(EBlurAttri).toInt();
    painter->save();
    painter->setClipping(false);
    auto pos =  view->mapToScene(posInViewport);

    qreal half = qMax(width / 2, 1);
    auto rectEllipse = QRectF(pos - QPointF(half, half), pos + QPointF(half, half));
    QPen pen(QColor(0, 0, 0, 40));
    pen.setWidthF(2 / view->getScale());
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(rectEllipse);

    pen.setColor(QColor(0, 0, 0, 40));
    pen.setWidthF(1 / view->getScale());
    painter->setPen(pen);
    auto rectEllipse2 = QRectF(pos - QPointF(half + 2 / view->getScale(), half + 2 / view->getScale()),
                               pos + QPointF(half + 2 / view->getScale(), half + 2 / view->getScale()));
    painter->drawEllipse(rectEllipse2);
    painter->restore();
}

void BlurTool::clearPointRecording()
{
    d_BlurTool()->blurActivedOrgPath.clear();
    d_BlurTool()->blurActivedStrokerPath.clear();
    DrawItemTool::clearPointRecording();
}

RasterItem *BlurTool::getRasterItem(ToolSceneEvent *event)
{
    RasterItem *pItem = nullptr;
    auto findIt = d_BlurTool()->_layers.find(event->scene());
    if (d_BlurTool()->_layers.find(event->scene()) == d_BlurTool()->_layers.end()) {
        auto scene = event->scene();
        RasterItem *layer = nullptr;
        if (scene->selectedItemCount() == 1) {
            auto pSelected = dynamic_cast<RasterItem *>(scene->selectedPageItems().first());
            if (pSelected != nullptr && pSelected->rasterType() == RasterItem::EImageType) {
                layer = pSelected;
            }
        }

        if (layer != nullptr) {
            scene->selectPageItem(layer);
            d_BlurTool()->_layers.insert(scene, layer);
        } else {
            //groupitem
            QList<RasterItem *> lRaster;
            if (scene->selectedPageItems().first()->type() == GroupItemType) {
                GroupItem *group = dynamic_cast<GroupItem *>(scene->selectedPageItems().first());
                for (auto ch : group->items(true)) {
                    RasterItem *pRaster = static_cast<RasterItem *>(ch);
                    if (nullptr == pRaster || !pRaster->sceneBoundingRect().contains(event->pos())) {
                        continue;
                    }

                    if ((ch->type() == RasterItemType) && (pRaster->rasterType() == pRaster->ERasterType::EImageType)) {
                        layer = pRaster;
                        scene->selectPageItem(group);
                        d_BlurTool()->_layers.insert(scene, layer);
                        break;
                    }
                }
            }
        }

        if (nullptr != layer) {
            pItem = layer;
        }

    } else {
        pItem = findIt.value();
    }

    return pItem;
}

QPainterPath &BlurTool::getActivedOrgPath(ToolSceneEvent *event, RasterItem *rstItem)
{
    if (!d_BlurTool()->blurActivedOrgPath.contains(event->uuid())) {
        QPainterPath path; path.moveTo(event->pos());
        d_BlurTool()->blurActivedOrgPath.insert(event->uuid(), path);
    }
    return d_BlurTool()->blurActivedOrgPath[event->uuid()];
}

QPainterPath &BlurTool::getActivedStrokerPath(ToolSceneEvent *event, RasterItem *rstItem)
{
    return d_BlurTool()->blurActivedStrokerPath[event->uuid()];
}

void BlurTool::creatBlurSrokerPaths(ToolSceneEvent *event, RasterItem *rstItem)
{
    if (d_BlurTool()->tempImages.find(rstItem) == d_BlurTool()->tempImages.end()) {
        QImage img = NSBlur::blurImage(rstItem->rImage(), d_BlurTool()->blurRadius, d_BlurTool()->blurTp);
        d_BlurTool()->tempImages[rstItem] = img;
    }

    //QPointF  prePos = rstItem->mapFromScene(event->lastEvent()->pos());
    QPointF  pos = rstItem->mapFromScene((event->pos()));
    PageView *pView = event->scene()->firstPageView();
    auto &path = d_BlurTool()->blurActivedOrgPath[event->uuid()];
    QPointF lastPos = path.elementAt(path.elementCount() - 1);
    path.lineTo(pos == lastPos ? pos + QPointF(0.000001, 0.000001) : pos);
    QPen pen;

    pen.setWidthF(pView->page()->defaultAttriVar(EBlurAttri).value<qreal>());
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);

    d_BlurTool()->blurActivedStrokerPath[event->uuid()] = VectorItem::getPenStrokerPath(path,
                                                                                        pen, true, 0, false);
    event->scene()->update();
}

void BlurTool::_onSceneSelectionChanged(const QList<PageItem *> &selectedItems)
{
    if (selectedItems.count() == 1) {
        RasterItem *item = static_cast<RasterItem *>(selectedItems.first());
        bool isenable = (item->type() == RasterItemType) && (item->rasterType() == item->ERasterType::EImageType);
        setEnable(isenable);

        if (selectedItems.first()->type() == GroupItemType) {
            GroupItem *group = dynamic_cast<GroupItem *>(selectedItems.first());
            for (auto ch : group->items(true)) {
                RasterItem *pRaset = static_cast<RasterItem *>(ch);
                if ((ch->type() == RasterItemType) && (pRaset->rasterType() == pRaset->ERasterType::EImageType)) {
                    setEnable(true);
                }
            }
        }

    } else {
        setEnable(false);
    }
}
