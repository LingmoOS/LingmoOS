// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "toolattrimgr.h"
#include "drawboard.h"
#include "drawtool.h"
#include "context/pagescene.h"
#include "items/layeritem.h"
#include "gui/pageview.h"

#include <QWindow>
#include <QScreen>
#include <QAbstractButton>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QLine>

ToolAttriMgr::ToolAttriMgr(QWidget *parent):
#ifndef USE_DTK
    QWidget(parent), m_curArrowDirention(ArrowBottom)
#else
    DArrowRectangle(DArrowRectangle::ArrowRight, FloatWidget, parent)
#endif
{
    setWgtAccesibleName(this, "ToolAttriMgr");
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Popup | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setFocusPolicy(Qt::NoFocus);
    m_vLayout = new QVBoxLayout;

    setLayout(m_vLayout);
    setObjectName("ToolAttriMgr");
#ifndef USE_DTK
    QGraphicsDropShadowEffect  *effect = new QGraphicsDropShadowEffect(this);
    effect->setBlurRadius(20);
    effect->setOffset(QPoint(0, 0));
    this->setGraphicsEffect(effect);
    m_vLayout->setSpacing(0);
    m_vLayout->setContentsMargins(arrowLenth + margin, arrowLenth + margin, arrowLenth + margin, arrowLenth + margin);
#else
    m_vLayout->setSpacing(5);
    m_vLayout->setContentsMargins(5 + margin, 10 + margin, 5 + margin, 20 + margin);
    setRadiusArrowStyleEnable(true);
    setRadius(15);
#endif

}

void ToolAttriMgr::showAt(int active, const QPoint &pos)
{
#ifndef USE_DTK
    wantArrowPosInGlobel = pos;

    currentWindowPos = wantArrowPosInGlobel - QPoint(size().width() / 2, size().height() + 16);
    QWidget::move(currentWindowPos);
    QWidget::show();
#else
    m_pos = pos;
    if (active == ToolActivedAndClicked) {
        m_vLayout->setContentsMargins(margin, margin - 5, margin, margin);
        setArrowDirection(DArrowRectangle::ArrowBottom);
    } else {
        m_vLayout->setContentsMargins(margin, margin, margin - 10, margin);
        setArrowDirection(DArrowRectangle::ArrowLeft);

        QRectF r = dynamic_cast<DrawBoard *>(parent())->currentPage()->scene()->selectionBoundingRect();
        auto view = dynamic_cast<DrawBoard *>(parent())->currentPage()->view();
        m_pos = view->viewport()->mapToGlobal(view->mapFromScene((r.topRight() + r.bottomRight()) / 2));
    }

    if (DArrowRectangle::ArrowLeft == arrowDirection()) {
        m_pos.setX(m_pos.x() + arrowOffset);
    } else if (DArrowRectangle::ArrowRight == arrowDirection()) {
        m_pos.setX(m_pos.x() - arrowOffset);
    } else if (DArrowRectangle::ArrowTop == arrowDirection()) {
        m_pos.setY(m_pos.y() + arrowOffset);
    } else if (DArrowRectangle::ArrowBottom == arrowDirection()) {
        m_pos.setY(m_pos.y() - arrowOffset);
    }

    this->updateGeometry();
    this->adjustSize();

    show(m_pos.x(), m_pos.y());
#endif
}

void ToolAttriMgr::showWidgets(int active, const QList<QWidget *> &oldWidgets, const QList<QWidget *> &exposeWidgets)
{
    Q_UNUSED(active)

    foreach (auto w, oldWidgets) {
        w->setParent(nullptr);
        w->hide();
    }
    foreach (auto w, exposeWidgets) {
        if (w->parent() != this) {
            w->setParent(this);
            layout()->addWidget(w);
            layout()->setAlignment(w, Qt::AlignLeft);
            w->show();
        }
    }
    this->updateGeometry();
    this->adjustSize();
}

void ToolAttriMgr::resizeEvent(QResizeEvent *event)
{
    if (isVisible()) {
        QSize sizeChange = event->oldSize() - event->size();
#ifndef USE_DTK
        m_paintPath = getDrawPath(m_curArrowDirention);
        if (ArrowBottom == m_curArrowDirention) {
            move(pos().x(), pos().y() + sizeChange.height());
        }
    }
    return QWidget::resizeEvent(event);
#else
        QWidget::move(QPoint(pos().x() + sizeChange.width(), pos().y() + sizeChange.height()));
        if (arrowDirection() == DArrowRectangle::ArrowRight || arrowDirection() == DArrowRectangle::ArrowLeft) {
            QMetaObject::invokeMethod(this, [ = ] {
                show(m_pos.x(), m_pos.y());
            }, Qt::QueuedConnection);

        }
    }

    return DArrowRectangle::resizeEvent(event);
#endif
}
#ifndef USE_DTK
void ToolAttriMgr::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);

    painter.setBrush(palette().background().color());

    painter.setPen(Qt::NoPen);

    painter.drawPath(m_paintPath);
}

void ToolAttriMgr::showEvent(QShowEvent *event)
{
    if (nullptr == windowHandle() || nullptr == windowHandle()->screen()) {
        return QWidget::showEvent(event);
    }

    //如果超出屏幕，调整显示位置
    QRectF vRect = geometry();
    QRectF screenRect = windowHandle()->screen()->geometry();
    m_curArrowDirention = ArrowBottom;
    m_paintPath = getDrawPath(m_curArrowDirention);
    if (screenRect.top() > vRect.top()) {
        currentWindowPos = QPoint(currentWindowPos.x(), currentWindowPos.y() + vRect.height() + buttonHeight);
        move(currentWindowPos);
        m_curArrowDirention = ArrowTop;
        m_paintPath = getDrawPath(m_curArrowDirention);
    }
    QWidget::showEvent(event);
}

QPainterPath ToolAttriMgr::getDrawPath(ArrowDirection direction)
{
    QPainterPath path;
    QRectF r = getValidRect();
    if (NoArrow == direction) {
        path.addRoundedRect(r, radius, radius);
    } else {
        QVector<QPoint> points;
        QVector<QRectF> rects;
        getDrawRectPointsAndArcRect(r.toRect(), points, rects);


        for (int i = 0; i < points.size(); ++i) {

            if (i != 0) {
                path.lineTo(points.at(i));
            } else {
                path.moveTo(points.at(0));
            }

            if (i < rects.size()) {
                path.arcTo(rects.at(i), i * 90, 90);
                QPoint midPoint = QLine(path.currentPosition().toPoint(), points.at(i + 1)).center();

                if (ArrowTop == direction && 0 == i) {
                    path.lineTo(midPoint + QPoint(arrowWidth / 2, 0));

                    if (m_bArcArrow) {
                        drawCubicToPath(path, midPoint + QPoint(arrowWidth / 2, 0), midPoint + QPoint(0, -arrowHeight), midPoint + QPoint(-arrowWidth / 2, 0));
                    } else
                        path.lineTo(midPoint + QPoint(0, -arrowHeight));

                    path.lineTo(midPoint + QPoint(-arrowWidth / 2, 0));
                } else if (ArrowLeft == direction && 1 == i) {
                    path.lineTo(midPoint + QPoint(0, -arrowWidth / 2));

                    if (m_bArcArrow)
                        drawCubicToPath(path, midPoint + QPoint(0, -arrowWidth / 2), midPoint + QPoint(-arrowHeight, 0), midPoint + QPoint(0, arrowWidth / 2));
                    else
                        path.lineTo(midPoint + QPoint(-arrowHeight, 0));

                    path.lineTo(midPoint + QPoint(0, arrowWidth / 2));
                } else if (ArrowBottom == direction && 2 == i) {
                    path.lineTo(midPoint + QPoint(-arrowWidth / 2, 0));

                    if (m_bArcArrow)
                        drawCubicToPath(path, midPoint + QPoint(-arrowWidth / 2, 0), midPoint + + QPoint(0, arrowHeight), midPoint + QPoint(arrowWidth / 2, 0));
                    else
                        path.lineTo(midPoint + QPoint(0, arrowHeight));

                    path.lineTo(midPoint + QPoint(arrowWidth / 2, 0));
                } else if (ArrowRight == direction && 3 == i) {
                    path.lineTo(midPoint + QPoint(0, arrowWidth / 2));

                    if (m_bArcArrow)
                        drawCubicToPath(path, midPoint + QPoint(0, arrowWidth / 2), midPoint + QPoint(arrowHeight, 0), midPoint + QPoint(0, -arrowWidth / 2));
                    else
                        path.lineTo(midPoint + QPoint(arrowHeight, 0));

                    path.lineTo(midPoint + QPoint(0, -arrowWidth / 2));
                }

            }

        }
    }

    path.closeSubpath();

    return  path;
}

QRect ToolAttriMgr::getValidRect()
{
    int adjust = arrowLenth;
    QRect r = rect();
    return r.adjusted(adjust, adjust, -adjust, -adjust);
}

void ToolAttriMgr::getDrawRectPointsAndArcRect(QRect rect, QVector<QPoint> &points, QVector<QRectF> &rects)
{
    points.clear();
    rects.clear();

    if (rect.width() < 2 * radius || rect.height() < 2 * radius)
        return;
    //从右上角开始，保证开始角度是0度

    points << QPoint(rect.topRight() + QPoint(0, radius));
    points << QPoint(rect.topLeft() + QPoint(radius, 0));
    points << QPoint(rect.bottomLeft() - QPoint(0, radius));
    points << QPoint(rect.bottomRight() - QPoint(radius, 0));
    points << QPoint(rect.topRight() + QPoint(0, radius));

    rects << QRectF(rect.topRight().x() - 2 * radius, rect.topRight().y(), 2 * radius, 2 * radius);
    rects << QRectF(rect.topLeft().x(), rect.topLeft().y(), 2 * radius, 2 * radius);
    rects << QRectF(rect.bottomLeft().x(), rect.bottomLeft().y() - 2 * radius, 2 * radius, 2 * radius);
    rects << QRectF(rect.bottomRight().x() - 2 * radius, rect.bottomRight().y() - 2 * radius, 2 * radius, 2 * radius);
}

void ToolAttriMgr::drawCubicToPath(QPainterPath &path, QPoint p1, QPoint p2, QPoint endPoint)
{
    QPoint sp = p1;
    QPoint ep = p2;
    QPointF c1 = QPointF((sp.x() + ep.x()) / 2, sp.y());
    QPointF c2 = QPointF((sp.x() + ep.x()) / 2, ep.y());
    path.cubicTo(c1, c2, ep);

    sp = p2;
    ep = endPoint;
    c1 = QPointF((sp.x() + ep.x()) / 2, sp.y());
    c2 = QPointF((sp.x() + ep.x()) / 2, ep.y());
    path.cubicTo(c1, c2, ep);
}
#endif


