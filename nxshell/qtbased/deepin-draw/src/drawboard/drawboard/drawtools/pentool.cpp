// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pentool.h"
#include "pagescene.h"
#include "pageview.h"
#include "rasteritem.h"
#include "cgraphicsitemevent.h"
#include "cundoredocommand.h"
#include "erasertool.h"

#include <QAbstractItemView>
#include <QUndoStack>
#include <QtMath>
#include <QPicture>
#include <QToolButton>
#include <QDesktopWidget>
#include <QApplication>
const QSize PEN_CURSOR_SIZE = QSize(9, 26);
class PenTool::PenTool_private
{
public:
    explicit PenTool_private(PenTool *qq): q(qq), m_pRenderImage(":/icons/deepin/builtin/texts/crayon.png") {}

    PenTool *q;

    QMap<PageScene *, RasterItem *> _rasterItems;
    QMap<int, RasterPaintCompsitor> _activePictures;
    QImage m_pRenderImage;
    QMap<int, QList<QLineF> > _activeTempLines;


    QPen getViewDefualtPen(PageView *view) const
    {
        PageView *pView = view;
        QPen pen;
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        pen.setColor(pView->page()->defaultAttriVar(EPenBrushColor).value<QColor>());
        pen.setWidthF(pView->page()->defaultAttriVar(EPenWidthProperty).value<qreal>());
        return pen;
    }

    QBrush getViewDefualtBrush(PageView *view) const
    {
        PageView *pView = view;
        QBrush brush;
        brush.setColor(pView->page()->defaultAttriVar(EPenBrushColor).value<QColor>());
        return brush;
    }

    QPixmap pictureColorChanged(const QImage &image, const QColor &color)
    {
        QImage _image = image.scaled(24, 24).convertToFormat(QImage::Format_ARGB32);
        for (int i = 0; i < _image.width(); ++i) {
            for (int j = 0; j < _image.height(); ++j) {
                int alpha = _image.pixelColor(i, j).alpha();
                if (alpha != 0) {
                    auto c = color;
                    c.setAlpha(alpha);
                    _image.setPixelColor(i, j, c);
                }
            }
        }
        QPixmap pixmap;
        pixmap.convertFromImage(_image);
        return pixmap;
    }

    RasterPaint paintNormalPen(ToolSceneEvent *event, RasterItem *pItem)
    {
        RasterPaint picture;
        QPainter painter(&picture);

        QPointF  prePos = pItem->mapFromScene(event->lastEvent()->pos());
        QPointF  pos = pItem->mapFromScene((event->pos())) ;

        QLineF l(prePos, pos);
        auto &tempLines = _activeTempLines[event->uuid()];
        tempLines.append(l);

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

        PageView *pView = event->scene()->firstPageView();
        QPen pen;
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        //qWarning() << pView->page()->defaultAttriVar(EPenColor).value<QColor>() << pView->page()->defaultAttriVar(EPenWidth).value<qreal>();
        pen.setColor(pView->page()->defaultAttriVar(EPenBrushColor).value<QColor>());
        //pen.setWidthF(pView->page()->defaultAttriVar(EPenWidth).value<qreal>());
        pen.setWidthF(pView->page()->defaultAttriVar(EPenWidthProperty).value<qreal>());
        painter.setPen(pen);
        painter.setRenderHint(QPainter::Antialiasing);

        painter.setCompositionMode(QPainter::CompositionMode_Source);

        if (l.p1() == l.p2() && tempLines.size() <= 1)//点击时需要绘制
            painter.drawLine(l);
        else
            painter.drawPath(drawPath);

        //painter.drawLine(l);

        painter.end();

        return picture;
    }

    RasterPaint paintCalligraphyPen(ToolSceneEvent *event, RasterItem *pItem)
    {
        PageView *pView = event->scene()->firstPageView();

        RasterPaint picture;
        QPainter painter(&picture);

        QPointF  prePos = pItem->mapFromScene(event->lastEvent()->pos());
        QPointF  pos = pItem->mapFromScene((event->pos())) ;

        QLineF l(prePos, pos);
        const qreal angleDegress = 30;
        const qreal cW = pView->page()->defaultAttriVar(EPenWidthProperty).value<qreal>() + 10;
        qreal offXLen = qCos(qDegreesToRadians(angleDegress)) * (cW / 2.0);
        qreal offYLen = qSin(qDegreesToRadians(angleDegress)) * (cW / 2.0);
        QPointF point1(prePos.x() - offXLen, prePos.y() - offYLen);
        QPointF point2(prePos.x() + offXLen, prePos.y() + offYLen);

        QPointF point3(pos.x() - offXLen, pos.y() - offYLen);
        QPointF point4(pos.x() + offXLen, pos.y() + offYLen);

        QPainterPath path;
        path.moveTo(point1);

        if (l.length() > 20) {
            path.quadTo(QLineF(point1, point2).center(), point2);
            path.quadTo(QLineF(point2, point4).center(), point4);
            path.quadTo(QLineF(point4, point3).center(), point3);
            path.quadTo(QLineF(point3, point1).center(), point1);
        } else {
            path.lineTo(point2);
            path.lineTo(point4);
            path.lineTo(point3);
            path.lineTo(point1);
        }
        painter.setRenderHint(QPainter::Antialiasing);

        painter.setBrush(pView->page()->defaultAttriVar(EPenBrushColor).value<QColor>());

        QPen p; p.setColor(pView->page()->defaultAttriVar(EPenBrushColor).value<QColor>());
        painter.setPen(p);

        painter.setCompositionMode(QPainter::CompositionMode_Source);

        painter.drawPath(path);

        painter.end();

        return picture;
    }

    RasterPaint paintCrayonPen(ToolSceneEvent *event, RasterItem *pItem, qreal space)
    {
        auto db = q->drawBoard();
        QColor color = db->pageAttriVariant(db->currentPage(), EPenBrushColor).value<QColor>();

        int px = -1;
        int penWidth = db->pageAttriVariant(db->currentPage(), EPenWidthProperty).toInt();
        if (penWidth > 1)
            px = penWidth;

        QImage renderImage = pictureColorChanged(m_pRenderImage, color).toImage().scaled(24 + px, 24 + px).convertToFormat(QImage::Format_ARGB32);

        RasterPaint picture;
        QPainter painter(&picture);

        QPointF  prePos = pItem->mapFromScene(event->lastEvent()->pos());
        QPointF  pos = pItem->mapFromScene((event->pos())) ;

        QLineF line(prePos, pos);
        qreal length = line.length();
        qreal width = 1.0 - length / 80.0;
        width = qMax(width, 0.25);

        painter.setOpacity(width);

        qreal total = space;
        QSize brushTextureSize = renderImage.size();

        if (!qFuzzyIsNull(total)) {
            QRect rect = QRect(QPointF(line.p2().toPoint() - QPointF(brushTextureSize.width() / 2, brushTextureSize.height() / 2)).toPoint(),
                               brushTextureSize);
            painter.drawImage(rect, renderImage);

            event->setPos(event->pos());
        }
        //event->setPosXAccepted(false);
        //event->setPosYAccepted(false);

        while (total < length) {
            line.setLength(total);
            QRect rect = QRect(QPointF(line.p2().toPoint() - QPointF(brushTextureSize.width() / 2, brushTextureSize.height() / 2)).toPoint(),
                               brushTextureSize);

            painter.drawImage(rect, renderImage);
            total += space;
        }

        return picture;
    }

    void paintPictureToView(const QPicture &picture, PageView *view)
    {
        QPainter painter(&view->cachedPixmap());

        auto trans = _rasterItems[view->pageScene()]->sceneTransform() * view->viewportTransform();

        painter.setTransform(trans);

        painter.drawPicture(QPoint(0, 0), picture);

        view->viewport()->update();
    }
};

PenTool::PenTool(QObject *parent)
    : DrawItemTool(parent), PenTool_d(new PenTool_private(this))

{
    setContinued(true);
    setMaxTouchPoint(10);
    setClearSelectionOnActived(false);

    QPixmap s_cur = QPixmap(":/cursorIcons/brush_mouse.svg");
    cursorScale(s_cur);
    //画笔特殊处理，大小需要再设置
    QDesktopWidget *desktopWidget = QApplication::desktop();
    qreal ratiof = desktopWidget->devicePixelRatioF();
    setCursor(QCursor(s_cur, qRound(PEN_CURSOR_SIZE.width()* ratiof), qRound(PEN_CURSOR_SIZE.height()* ratiof)));
    setTouchSensitiveRadius(0);

    auto m_penBtn = toolButton();
    m_penBtn->setShortcut(QKeySequence(QKeySequence(Qt::Key_P)));
    setWgtAccesibleName(m_penBtn, "Pencil tool button");
    m_penBtn->setToolTip(tr("Pencil (P)"));
    m_penBtn->setIconSize(TOOL_ICON_RECT);
    m_penBtn->setFixedSize(TOOL_BUTTON_RECT);
    m_penBtn->setCheckable(true);
    m_penBtn->setIcon(QIcon::fromTheme("pen_normal"));
    connect(m_penBtn, &QToolButton::toggled, m_penBtn, [ = ](bool b) {
        QIcon icon       = QIcon::fromTheme("pen_normal");
        QIcon activeIcon = QIcon::fromTheme("pen_highlight");
        m_penBtn->setIcon(b ? activeIcon : icon);
    });
}

int PenTool::toolType() const
{
    return pen;
}

SAttrisList PenTool::attributions()
{
    SAttrisList result;
    result << defaultAttriVar(EPenBrushColor)
           << defaultAttriVar(EPenStyle)
           //<< defaultAttriVar(EPenWidth)
           << defaultAttriVar(EEnablePenBrushStyle)
           << defaultAttriVar(ERotProperty)
           << defaultAttriVar(EPenWidthProperty);
    return result;
}



PageItem *PenTool::drawItemStart(ToolSceneEvent *event)
{
    PageItem *pItem = nullptr;

    if (event->isNormalPressed()) {
        if (event->isFirstOne())
            event->view()->setCacheEnable(true);

        auto findIt = d_PenTool()->_rasterItems.find(event->scene());
        if (d_PenTool()->_rasterItems.find(event->scene()) == d_PenTool()->_rasterItems.end()) {
            auto scene = event->scene();
            RasterItem *layer = nullptr;
            auto selectedPageItems = scene->selectedPageItems();
            if (selectedPageItems.count() == 1) {
                auto pSelected = dynamic_cast<RasterItem *>(selectedPageItems.first());
                if (pSelected != nullptr && pSelected->rasterType() == RasterItem::EPenType
                        && !pSelected->isBlocked()) {
                    layer = pSelected;
                }
            }
            if (layer == nullptr) {
                layer = new RasterItem;
                scene->addPageItem(layer);
            }
            d_PenTool()->_rasterItems.insert(scene, layer);
            pItem = layer;
        } else {
            pItem = findIt.value();
        }
    }
    return pItem;
}

void PenTool::drawItemUpdate(ToolSceneEvent *event, PageItem *pItem)
{
    int penStyle = drawBoard()->currentPage()->defaultAttriVar(EPenStyle).toInt();

    auto layer = dynamic_cast<RasterItem *>(pItem);

    RasterPaint picture;
    switch (penStyle) {
    case ENormalPen: {
        picture = d_PenTool()->paintNormalPen(event, layer);
        break;
    }
    case ECalligraphyPen: {
        picture = d_PenTool()->paintCalligraphyPen(event, layer);
        break;
    }
    case ECrayonPen : {
        picture = d_PenTool()->paintCrayonPen(event, layer, 8);
        break;
    }
    default:
        break;
    }
    d_PenTool()->_activePictures[event->uuid()].merge(picture);

    //实时显示,绘制到临时显示的缓冲图上
    d_PenTool()->paintPictureToView(picture, event->view());
}

void PenTool::drawItemFinish(ToolSceneEvent *event, PageItem *pItem)
{
    d_PenTool()->_activeTempLines.take(event->uuid());
    auto picture = d_PenTool()->_activePictures.take(event->uuid());

    auto pLayer = dynamic_cast<RasterItem *>(pItem);
    if (pLayer != nullptr)
        pLayer->addPaint(picture.result(), true, true, false);

    if (event->isFinalOne()) {
        event->view()->setCacheEnable(false);
        d_PenTool()->_rasterItems.remove(event->scene());
    }
}

int PenTool::minMoveUpdateDistance()
{
    return 0;
}

void PenTool::onStatusChanged(EStatus oldStatus, EStatus nowStatus)
{
    auto scene = currentPage() != nullptr ? currentPage()->scene() : nullptr;

    if (scene == nullptr)
        return;

    if (oldStatus == EIdle && nowStatus == EReady) {
        scene->setSelectionsVisible(false);
    }

    if (oldStatus == EReady && nowStatus == EIdle) {
        scene->setSelectionsVisible(true);
    }
}


