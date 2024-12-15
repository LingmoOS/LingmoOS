// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imagesvgitem.h"

#if !defined(QT_NO_GRAPHICSVIEW) && !defined(QT_NO_WIDGETS)

#include "dsvgrenderer.h"
#include "qdebug.h"
#include "qpainter.h"
#include "qstyleoption.h"
#include <QSvgRenderer>
QT_BEGIN_NAMESPACE

#define Q_DECLARE_PUBLIC(Class)                                    \
    inline Class* q_func() { return static_cast<Class *>(q_ptr); } \
    inline const Class* q_func() const { return static_cast<const Class *>(q_ptr); } \
    friend class Class;

LibImageSvgItem::LibImageSvgItem(QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    setParentItem(parent);
    m_renderer = new QSvgRenderer(this);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    setMaximumCacheSize(QSize(1024, 768));
}

LibImageSvgItem::LibImageSvgItem(const QString &fileName, QGraphicsItem *parent)
//:QGraphicsSvgItem(parent)
    : QGraphicsObject(parent)
{
    setParentItem(parent);
    m_renderer = new QSvgRenderer(this);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    setMaximumCacheSize(QSize(1024, 768));
    m_renderer->load(fileName);
    updateDefaultSize();
}

LibImageSvgItem::~LibImageSvgItem()
{
}

QSvgRenderer *LibImageSvgItem::renderer() const
{
    return m_renderer;
}

QRectF LibImageSvgItem::boundingRect() const
{
    return m_boundingRect;
}

static void qt_graphicsItem_highlightSelected(QGraphicsItem *item, QPainter *painter,
                                              const QStyleOptionGraphicsItem *option)
{
    const QRectF murect = painter->transform().mapRect(QRectF(0, 0, 1, 1));
    if (qFuzzyIsNull(qMax(murect.width(), murect.height())))
        return;

    const QRectF mbrect = painter->transform().mapRect(item->boundingRect());
    if (qMin(mbrect.width(), mbrect.height()) < qreal(1.0))
        return;

    qreal itemPenWidth;
    switch (item->type()) {
    case QGraphicsEllipseItem::Type:
        itemPenWidth = static_cast<QGraphicsEllipseItem *>(item)->pen().widthF();
        break;
    case QGraphicsPathItem::Type:
        itemPenWidth = static_cast<QGraphicsPathItem *>(item)->pen().widthF();
        break;
    case QGraphicsPolygonItem::Type:
        itemPenWidth = static_cast<QGraphicsPolygonItem *>(item)->pen().widthF();
        break;
    case QGraphicsRectItem::Type:
        itemPenWidth = static_cast<QGraphicsRectItem *>(item)->pen().widthF();
        break;
    case QGraphicsSimpleTextItem::Type:
        itemPenWidth = static_cast<QGraphicsSimpleTextItem *>(item)->pen().widthF();
        break;
    case QGraphicsLineItem::Type:
        itemPenWidth = static_cast<QGraphicsLineItem *>(item)->pen().widthF();
        break;
    default:
        itemPenWidth = 1.0;
    }
    const qreal pad = itemPenWidth / 2;

    const qreal penWidth = 0;  // cosmetic pen

    const QColor fgcolor = option->palette.windowText().color();
    const QColor bgcolor(  // ensure good contrast against fgcolor
        fgcolor.red() > 127 ? 0 : 255, fgcolor.green() > 127 ? 0 : 255,
        fgcolor.blue() > 127 ? 0 : 255);

    painter->setPen(QPen(bgcolor, penWidth, Qt::SolidLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(item->boundingRect().adjusted(pad, pad, -pad, -pad));

    painter->setPen(QPen(option->palette.windowText(), 0, Qt::DashLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(item->boundingRect().adjusted(pad, pad, -pad, -pad));
}

void LibImageSvgItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    //    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (!m_renderer->isValid())
        return;

    if (m_elemId.isEmpty())
        m_renderer->render(painter, m_boundingRect);
    else
        m_renderer->render(painter, m_elemId, m_boundingRect);

    if (option->state & QStyle::State_Selected)
        qt_graphicsItem_highlightSelected(this, painter, option);
}

int LibImageSvgItem::type() const
{
    return Type;
}

void LibImageSvgItem::updateDefaultSize()
{
    QRectF bounds;
    if (m_elemId.isEmpty()) {
        bounds = QRectF(QPointF(0, 0), m_renderer->defaultSize());
    } else {
        bounds = m_renderer->boundsOnElement(m_elemId);
    }
    if (m_boundingRect.size() != bounds.size()) {
        prepareGeometryChange();
        m_boundingRect.setSize(bounds.size());
    }
}

void LibImageSvgItem::setMaximumCacheSize(const QSize &size)
{
    Q_UNUSED(size);
//    QGraphicsItem::d_ptr->setExtra(QGraphicsItemPrivate::ExtraMaxDeviceCoordCacheSize, size);
    update();
}

QSize LibImageSvgItem::maximumCacheSize() const
{
    return QSize();//QGraphicsItem::d_ptr->extra(QGraphicsItemPrivate::ExtraMaxDeviceCoordCacheSize).toSize();
}

void LibImageSvgItem::setElementId(const QString &id)
{
    m_elemId = id;
    updateDefaultSize();
    update();
}

QString LibImageSvgItem::elementId() const
{
    return m_elemId;
}

void LibImageSvgItem::setSharedRenderer(QSvgRenderer *renderer)
{
    m_renderer = renderer;

    updateDefaultSize();

    update();
}

void LibImageSvgItem::setCachingEnabled(bool caching)
{
    setCacheMode(caching ? QGraphicsItem::DeviceCoordinateCache : QGraphicsItem::NoCache);
}

bool LibImageSvgItem::isCachingEnabled() const
{
    return cacheMode() != QGraphicsItem::NoCache;
}

#endif  // QT_NO_WIDGETS
