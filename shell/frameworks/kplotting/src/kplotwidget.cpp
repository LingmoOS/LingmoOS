/*  -*- C++ -*-
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 Jason Harris <kstars@30doradus.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kplotwidget.h"

#include <math.h>

#include <QHash>
#include <QHelpEvent>
#include <QPainter>
#include <QToolTip>
#include <QtAlgorithms>

#include "kplotaxis.h"
#include "kplotobject.h"
#include "kplotpoint.h"

#define XPADDING 20
#define YPADDING 20
#define BIGTICKSIZE 10
#define SMALLTICKSIZE 4
#define TICKOFFSET 0

class Q_DECL_HIDDEN KPlotWidget::Private
{
public:
    Private(KPlotWidget *qq)
        : q(qq)
        , cBackground(Qt::black)
        , cForeground(Qt::white)
        , cGrid(Qt::gray)
        , showGrid(false)
        , showObjectToolTip(true)
        , useAntialias(false)
        , autoDelete(true)
    {
        // create the axes and setting their default properties
        KPlotAxis *leftAxis = new KPlotAxis();
        leftAxis->setTickLabelsShown(true);
        axes.insert(LeftAxis, leftAxis);
        KPlotAxis *bottomAxis = new KPlotAxis();
        bottomAxis->setTickLabelsShown(true);
        axes.insert(BottomAxis, bottomAxis);
        KPlotAxis *rightAxis = new KPlotAxis();
        axes.insert(RightAxis, rightAxis);
        KPlotAxis *topAxis = new KPlotAxis();
        axes.insert(TopAxis, topAxis);
    }

    ~Private()
    {
        if (autoDelete) {
            qDeleteAll(objectList);
        }
        qDeleteAll(axes);
    }

    KPlotWidget *q;

    void calcDataRectLimits(double x1, double x2, double y1, double y2);
    /**
     * @return a value indicating how well the given rectangle is
     * avoiding masked regions in the plot.  A higher returned value
     * indicates that the rectangle is intersecting a larger portion
     * of the masked region, or a portion of the masked region which
     * is weighted higher.
     * @param r The rectangle to be tested
     */
    float rectCost(const QRectF &r) const;

    // Colors
    QColor cBackground, cForeground, cGrid;
    // draw options
    bool showGrid;
    bool showObjectToolTip;
    bool useAntialias;
    bool autoDelete;
    // padding
    int leftPadding, rightPadding, topPadding, bottomPadding;
    // hashmap with the axes we have
    QHash<Axis, KPlotAxis *> axes;
    // List of KPlotObjects
    QList<KPlotObject *> objectList;
    // Limits of the plot area in data units
    QRectF dataRect, secondDataRect;
    // Limits of the plot area in pixel units
    QRect pixRect;
    // Array holding the mask of "used" regions of the plot
    QImage plotMask;
};

KPlotWidget::KPlotWidget(QWidget *parent)
    : QFrame(parent)
    , d(new Private(this))
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);

    d->secondDataRect = QRectF(); // default: no secondary data rect
    // sets the default limits
    d->calcDataRectLimits(0.0, 1.0, 0.0, 1.0);

    setDefaultPaddings();
}

KPlotWidget::~KPlotWidget() = default;

QSize KPlotWidget::minimumSizeHint() const
{
    return QSize(150, 150);
}

QSize KPlotWidget::sizeHint() const
{
    return size();
}

void KPlotWidget::setLimits(double x1, double x2, double y1, double y2)
{
    d->calcDataRectLimits(x1, x2, y1, y2);
    update();
}

void KPlotWidget::Private::calcDataRectLimits(double x1, double x2, double y1, double y2)
{
    double XA1;
    double XA2;
    double YA1;
    double YA2;
    if (x2 < x1) {
        XA1 = x2;
        XA2 = x1;
    } else {
        XA1 = x1;
        XA2 = x2;
    }
    if (y2 < y1) {
        YA1 = y2;
        YA2 = y1;
    } else {
        YA1 = y1;
        YA2 = y2;
    }

    if (XA2 == XA1) {
        // qWarning() << "x1 and x2 cannot be equal. Setting x2 = x1 + 1.0";
        XA2 = XA1 + 1.0;
    }
    if (YA2 == YA1) {
        // qWarning() << "y1 and y2 cannot be equal. Setting y2 = y1 + 1.0";
        YA2 = YA1 + 1.0;
    }
    dataRect = QRectF(XA1, YA1, XA2 - XA1, YA2 - YA1);

    q->axis(LeftAxis)->setTickMarks(dataRect.y(), dataRect.height());
    q->axis(BottomAxis)->setTickMarks(dataRect.x(), dataRect.width());

    if (secondDataRect.isNull()) {
        q->axis(RightAxis)->setTickMarks(dataRect.y(), dataRect.height());
        q->axis(TopAxis)->setTickMarks(dataRect.x(), dataRect.width());
    }
}

void KPlotWidget::setSecondaryLimits(double x1, double x2, double y1, double y2)
{
    double XA1;
    double XA2;
    double YA1;
    double YA2;
    if (x2 < x1) {
        XA1 = x2;
        XA2 = x1;
    } else {
        XA1 = x1;
        XA2 = x2;
    }
    if (y2 < y1) {
        YA1 = y2;
        YA2 = y1;
    } else {
        YA1 = y1;
        YA2 = y2;
    }

    if (XA2 == XA1) {
        // qWarning() << "x1 and x2 cannot be equal. Setting x2 = x1 + 1.0";
        XA2 = XA1 + 1.0;
    }
    if (YA2 == YA1) {
        // qWarning() << "y1 and y2 cannot be equal. Setting y2 = y1 + 1.0";
        YA2 = YA1 + 1.0;
    }
    d->secondDataRect = QRectF(XA1, YA1, XA2 - XA1, YA2 - YA1);

    axis(RightAxis)->setTickMarks(d->secondDataRect.y(), d->secondDataRect.height());
    axis(TopAxis)->setTickMarks(d->secondDataRect.x(), d->secondDataRect.width());

    update();
}

void KPlotWidget::clearSecondaryLimits()
{
    d->secondDataRect = QRectF();
    axis(RightAxis)->setTickMarks(d->dataRect.y(), d->dataRect.height());
    axis(TopAxis)->setTickMarks(d->dataRect.x(), d->dataRect.width());

    update();
}

QRectF KPlotWidget::dataRect() const
{
    return d->dataRect;
}

QRectF KPlotWidget::secondaryDataRect() const
{
    return d->secondDataRect;
}

void KPlotWidget::addPlotObject(KPlotObject *object)
{
    // skip null pointers
    if (!object) {
        return;
    }
    d->objectList.append(object);
    update();
}

void KPlotWidget::addPlotObjects(const QList<KPlotObject *> &objects)
{
    bool addedsome = false;
    for (KPlotObject *o : objects) {
        if (!o) {
            continue;
        }

        d->objectList.append(o);
        addedsome = true;
    }
    if (addedsome) {
        update();
    }
}

QList<KPlotObject *> KPlotWidget::plotObjects() const
{
    return d->objectList;
}

void KPlotWidget::setAutoDeletePlotObjects(bool autoDelete)
{
    d->autoDelete = autoDelete;
}

void KPlotWidget::removeAllPlotObjects()
{
    if (d->objectList.isEmpty()) {
        return;
    }

    if (d->autoDelete) {
        qDeleteAll(d->objectList);
    }
    d->objectList.clear();
    update();
}

void KPlotWidget::resetPlotMask()
{
    d->plotMask = QImage(pixRect().size(), QImage::Format_ARGB32);
    QColor fillColor = Qt::black;
    fillColor.setAlpha(128);
    d->plotMask.fill(fillColor.rgb());
}

void KPlotWidget::resetPlot()
{
    if (d->autoDelete) {
        qDeleteAll(d->objectList);
    }
    d->objectList.clear();
    clearSecondaryLimits();
    d->calcDataRectLimits(0.0, 1.0, 0.0, 1.0);
    KPlotAxis *a = axis(RightAxis);
    a->setLabel(QString());
    a->setTickLabelsShown(false);
    a = axis(TopAxis);
    a->setLabel(QString());
    a->setTickLabelsShown(false);
    axis(KPlotWidget::LeftAxis)->setLabel(QString());
    axis(KPlotWidget::BottomAxis)->setLabel(QString());
    resetPlotMask();
}

void KPlotWidget::replacePlotObject(int i, KPlotObject *o)
{
    // skip null pointers and invalid indexes
    if (!o || i < 0 || i >= d->objectList.count()) {
        return;
    }
    if (d->objectList.at(i) == o) {
        return;
    }
    if (d->autoDelete) {
        delete d->objectList.at(i);
    }
    d->objectList.replace(i, o);
    update();
}

QColor KPlotWidget::backgroundColor() const
{
    return d->cBackground;
}

QColor KPlotWidget::foregroundColor() const
{
    return d->cForeground;
}

QColor KPlotWidget::gridColor() const
{
    return d->cGrid;
}

void KPlotWidget::setBackgroundColor(const QColor &bg)
{
    d->cBackground = bg;
    update();
}

void KPlotWidget::setForegroundColor(const QColor &fg)
{
    d->cForeground = fg;
    update();
}

void KPlotWidget::setGridColor(const QColor &gc)
{
    d->cGrid = gc;
    update();
}

bool KPlotWidget::isGridShown() const
{
    return d->showGrid;
}

bool KPlotWidget::isObjectToolTipShown() const
{
    return d->showObjectToolTip;
}

bool KPlotWidget::antialiasing() const
{
    return d->useAntialias;
}

void KPlotWidget::setAntialiasing(bool b)
{
    d->useAntialias = b;
    update();
}

void KPlotWidget::setShowGrid(bool show)
{
    d->showGrid = show;
    update();
}

void KPlotWidget::setObjectToolTipShown(bool show)
{
    d->showObjectToolTip = show;
}

KPlotAxis *KPlotWidget::axis(Axis type)
{
    QHash<Axis, KPlotAxis *>::Iterator it = d->axes.find(type);
    return it != d->axes.end() ? it.value() : nullptr;
}

const KPlotAxis *KPlotWidget::axis(Axis type) const
{
    QHash<Axis, KPlotAxis *>::ConstIterator it = d->axes.constFind(type);
    return it != d->axes.constEnd() ? it.value() : nullptr;
}

QRect KPlotWidget::pixRect() const
{
    return d->pixRect;
}

QList<KPlotPoint *> KPlotWidget::pointsUnderPoint(const QPoint &p) const
{
    QList<KPlotPoint *> pts;
    for (const KPlotObject *po : std::as_const(d->objectList)) {
        const auto pointsList = po->points();
        for (KPlotPoint *pp : pointsList) {
            if ((p - mapToWidget(pp->position()).toPoint()).manhattanLength() <= 4) {
                pts << pp;
            }
        }
    }

    return pts;
}

bool KPlotWidget::event(QEvent *e)
{
    if (e->type() == QEvent::ToolTip) {
        if (d->showObjectToolTip) {
            QHelpEvent *he = static_cast<QHelpEvent *>(e);
            QList<KPlotPoint *> pts = pointsUnderPoint(he->pos() - QPoint(leftPadding(), topPadding()) - contentsRect().topLeft());
            if (!pts.isEmpty()) {
                QToolTip::showText(he->globalPos(), pts.front()->label(), this);
            }
        }
        e->accept();
        return true;
    } else {
        return QFrame::event(e);
    }
}

void KPlotWidget::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);
    setPixRect();
    resetPlotMask();
}

void KPlotWidget::setPixRect()
{
    int newWidth = contentsRect().width() - leftPadding() - rightPadding();
    int newHeight = contentsRect().height() - topPadding() - bottomPadding();
    // PixRect starts at (0,0) because we will translate by leftPadding(), topPadding()
    d->pixRect = QRect(0, 0, newWidth, newHeight);
}

QPointF KPlotWidget::mapToWidget(const QPointF &p) const
{
    float px = d->pixRect.left() + d->pixRect.width() * (p.x() - d->dataRect.x()) / d->dataRect.width();
    float py = d->pixRect.top() + d->pixRect.height() * (d->dataRect.y() + d->dataRect.height() - p.y()) / d->dataRect.height();
    return QPointF(px, py);
}

void KPlotWidget::maskRect(const QRectF &rf, float fvalue)
{
    QRect r = rf.toRect().intersected(d->pixRect);
    int value = int(fvalue);
    QColor newColor;
    for (int ix = r.left(); ix < r.right(); ++ix) {
        for (int iy = r.top(); iy < r.bottom(); ++iy) {
            newColor = QColor(d->plotMask.pixel(ix, iy));
            newColor.setAlpha(200);
            newColor.setRed(qMin(newColor.red() + value, 255));
            d->plotMask.setPixel(ix, iy, newColor.rgba());
        }
    }
}

void KPlotWidget::maskAlongLine(const QPointF &p1, const QPointF &p2, float fvalue)
{
    if (!d->pixRect.contains(p1.toPoint()) && !d->pixRect.contains(p2.toPoint())) {
        return;
    }

    int value = int(fvalue);

    // Determine slope and zeropoint of line
    double m = (p2.y() - p1.y()) / (p2.x() - p1.x());
    double y0 = p1.y() - m * p1.x();
    QColor newColor;

    // Mask each pixel along the line joining p1 and p2
    if (m > 1.0 || m < -1.0) { // step in y-direction
        int y1 = int(p1.y());
        int y2 = int(p2.y());
        if (y1 > y2) {
            y1 = int(p2.y());
            y2 = int(p1.y());
        }

        for (int y = y1; y <= y2; ++y) {
            int x = int((y - y0) / m);
            if (d->pixRect.contains(x, y)) {
                newColor = QColor(d->plotMask.pixel(x, y));
                newColor.setAlpha(100);
                newColor.setRed(qMin(newColor.red() + value, 255));
                d->plotMask.setPixel(x, y, newColor.rgba());
            }
        }

    } else { // step in x-direction
        int x1 = int(p1.x());
        int x2 = int(p2.x());
        if (x1 > x2) {
            x1 = int(p2.x());
            x2 = int(p1.x());
        }

        for (int x = x1; x <= x2; ++x) {
            int y = int(y0 + m * x);
            if (d->pixRect.contains(x, y)) {
                newColor = QColor(d->plotMask.pixel(x, y));
                newColor.setAlpha(100);
                newColor.setRed(qMin(newColor.red() + value, 255));
                d->plotMask.setPixel(x, y, newColor.rgba());
            }
        }
    }
}

// Determine optimal placement for a text label for point pp.  We want
// the label to be near point pp, but we don't want it to overlap with
// other labels or plot elements.  We will use a "downhill simplex"
// algorithm to find a label position that minimizes the pixel values
// in the plotMask image over the label's rect().  The sum of pixel
// values in the label's rect is the "cost" of placing the label there.
//
// Because a downhill simplex follows the local gradient to find low
// values, it can get stuck in local minima.  To mitigate this, we will
// iteratively attempt each of the initial path offset directions (up,
// down, right, left) in the order of increasing cost at each location.
void KPlotWidget::placeLabel(QPainter *painter, KPlotPoint *pp)
{
    int textFlags = Qt::TextSingleLine | Qt::AlignCenter;

    QPointF pos = mapToWidget(pp->position());
    if (!d->pixRect.contains(pos.toPoint())) {
        return;
    }

    QFontMetricsF fm(painter->font(), painter->device());
    QRectF bestRect = fm.boundingRect(QRectF(pos.x(), pos.y(), 1, 1), textFlags, pp->label());
    float xStep = 0.5 * bestRect.width();
    float yStep = 0.5 * bestRect.height();
    float maxCost = 0.05 * bestRect.width() * bestRect.height();
    float bestCost = d->rectCost(bestRect);

    // We will travel along a path defined by the maximum decrease in
    // the cost at each step.  If this path takes us to a local minimum
    // whose cost exceeds maxCost, then we will restart at the
    // beginning and select the next-best path.  The indices of
    // already-tried paths are stored in the TriedPathIndex list.
    //
    // If we try all four first-step paths and still don't get below
    // maxCost, then we'll adopt the local minimum position with the
    // best cost (designated as bestBadCost).
    int iter = 0;
    QList<int> TriedPathIndex;
    float bestBadCost = 10000;
    QRectF bestBadRect;

    // needed to halt iteration from inside the switch
    bool flagStop = false;

    while (bestCost > maxCost) {
        // Displace the label up, down, left, right; determine which
        // step provides the lowest cost
        QRectF upRect = bestRect;
        upRect.moveTop(upRect.top() + yStep);
        float upCost = d->rectCost(upRect);
        QRectF downRect = bestRect;
        downRect.moveTop(downRect.top() - yStep);
        float downCost = d->rectCost(downRect);
        QRectF leftRect = bestRect;
        leftRect.moveLeft(leftRect.left() - xStep);
        float leftCost = d->rectCost(leftRect);
        QRectF rightRect = bestRect;
        rightRect.moveLeft(rightRect.left() + xStep);
        float rightCost = d->rectCost(rightRect);

        // which direction leads to the lowest cost?
        QList<float> costList;
        costList << upCost << downCost << leftCost << rightCost;
        int imin = -1;
        for (int i = 0; i < costList.size(); ++i) {
            if (iter == 0 && TriedPathIndex.contains(i)) {
                continue; // Skip this first-step path, we already tried it!
            }

            // If this first-step path doesn't improve the cost,
            // skip this direction from now on
            if (iter == 0 && costList[i] >= bestCost) {
                TriedPathIndex.append(i);
                continue;
            }

            if (costList[i] < bestCost && (imin < 0 || costList[i] < costList[imin])) {
                imin = i;
            }
        }

        // Make a note that we've tried the current first-step path
        if (iter == 0 && imin >= 0) {
            TriedPathIndex.append(imin);
        }

        // Adopt the step that produced the best cost
        switch (imin) {
        case 0: // up
            bestRect.moveTop(upRect.top());
            bestCost = upCost;
            break;
        case 1: // down
            bestRect.moveTop(downRect.top());
            bestCost = downCost;
            break;
        case 2: // left
            bestRect.moveLeft(leftRect.left());
            bestCost = leftCost;
            break;
        case 3: // right
            bestRect.moveLeft(rightRect.left());
            bestCost = rightCost;
            break;
        case -1: // no lower cost found!
            // We hit a local minimum.  Keep the best of these as bestBadRect
            if (bestCost < bestBadCost) {
                bestBadCost = bestCost;
                bestBadRect = bestRect;
            }

            // If all of the first-step paths have now been searched, we'll
            // have to adopt the bestBadRect
            if (TriedPathIndex.size() == 4) {
                bestRect = bestBadRect;
                flagStop = true; // halt iteration
                break;
            }

            // If we haven't yet tried all of the first-step paths, start over
            if (TriedPathIndex.size() < 4) {
                iter = -1; // anticipating the ++iter below
                bestRect = fm.boundingRect(QRectF(pos.x(), pos.y(), 1, 1), textFlags, pp->label());
                bestCost = d->rectCost(bestRect);
            }
            break;
        }

        // Halt iteration, because we've tried all directions and
        // haven't gotten below maxCost (we'll adopt the best
        // local minimum found)
        if (flagStop) {
            break;
        }

        ++iter;
    }

    painter->drawText(bestRect, textFlags, pp->label());

    // Is a line needed to connect the label to the point?
    float deltax = pos.x() - bestRect.center().x();
    float deltay = pos.y() - bestRect.center().y();
    float rbest = sqrt(deltax * deltax + deltay * deltay);
    if (rbest > 20.0) {
        // Draw a rectangle around the label
        painter->setBrush(QBrush());
        // QPen pen = painter->pen();
        // pen.setStyle( Qt::DotLine );
        // painter->setPen( pen );
        painter->drawRoundedRect(bestRect, 25, 25, Qt::RelativeSize);

        // Now connect the label to the point with a line.
        // The line is drawn from the center of the near edge of the rectangle
        float xline = bestRect.center().x();
        if (bestRect.left() > pos.x()) {
            xline = bestRect.left();
        }
        if (bestRect.right() < pos.x()) {
            xline = bestRect.right();
        }

        float yline = bestRect.center().y();
        if (bestRect.top() > pos.y()) {
            yline = bestRect.top();
        }
        if (bestRect.bottom() < pos.y()) {
            yline = bestRect.bottom();
        }

        painter->drawLine(QPointF(xline, yline), pos);
    }

    // Mask the label's rectangle so other labels won't overlap it.
    maskRect(bestRect);
}

float KPlotWidget::Private::rectCost(const QRectF &r) const
{
    if (!plotMask.rect().contains(r.toRect())) {
        return 10000.;
    }

    // Compute sum of mask values in the rect r
    QImage subMask = plotMask.copy(r.toRect());
    int cost = 0;
    for (int ix = 0; ix < subMask.width(); ++ix) {
        for (int iy = 0; iy < subMask.height(); ++iy) {
            cost += QColor(subMask.pixel(ix, iy)).red();
        }
    }

    return float(cost);
}

void KPlotWidget::paintEvent(QPaintEvent *e)
{
    // let QFrame draw its default stuff (like the frame)
    QFrame::paintEvent(e);
    QPainter p;

    p.begin(this);
    p.setRenderHint(QPainter::Antialiasing, d->useAntialias);
    p.fillRect(rect(), backgroundColor());
    p.translate(leftPadding() + 0.5, topPadding() + 0.5);

    setPixRect();
    p.setClipRect(d->pixRect);
    p.setClipping(true);

    resetPlotMask();

    for (KPlotObject *po : std::as_const(d->objectList)) {
        po->draw(&p, this);
    }

    // DEBUG: Draw the plot mask
    //    p.drawImage( 0, 0, d->plotMask );

    p.setClipping(false);
    drawAxes(&p);

    p.end();
}

void KPlotWidget::drawAxes(QPainter *p)
{
    if (d->showGrid) {
        p->setPen(gridColor());

        // Grid lines are placed at locations of primary axes' major tickmarks
        // vertical grid lines
        const QList<double> majMarks = axis(BottomAxis)->majorTickMarks();
        for (const double xx : majMarks) {
            double px = d->pixRect.width() * (xx - d->dataRect.x()) / d->dataRect.width();
            p->drawLine(QPointF(px, 0.0), QPointF(px, double(d->pixRect.height())));
        }
        // horizontal grid lines
        const QList<double> leftTickMarks = axis(LeftAxis)->majorTickMarks();
        for (const double yy : leftTickMarks) {
            double py = d->pixRect.height() * (1.0 - (yy - d->dataRect.y()) / d->dataRect.height());
            p->drawLine(QPointF(0.0, py), QPointF(double(d->pixRect.width()), py));
        }
    }

    p->setPen(foregroundColor());
    p->setBrush(Qt::NoBrush);

    // set small font for tick labels
    QFont f = p->font();
    int s = f.pointSize();
    f.setPointSize(s - 2);
    p->setFont(f);

    /*** BottomAxis ***/
    KPlotAxis *a = axis(BottomAxis);
    if (a->isVisible()) {
        // Draw axis line
        p->drawLine(0, d->pixRect.height(), d->pixRect.width(), d->pixRect.height());

        // Draw major tickmarks
        const QList<double> majMarks = a->majorTickMarks();
        for (const double xx : majMarks) {
            double px = d->pixRect.width() * (xx - d->dataRect.x()) / d->dataRect.width();
            if (px > 0 && px < d->pixRect.width()) {
                p->drawLine(QPointF(px, double(d->pixRect.height() - TICKOFFSET)), //
                            QPointF(px, double(d->pixRect.height() - BIGTICKSIZE - TICKOFFSET)));

                // Draw ticklabel
                if (a->areTickLabelsShown()) {
                    QRect r(int(px) - BIGTICKSIZE, d->pixRect.height() + BIGTICKSIZE, 2 * BIGTICKSIZE, BIGTICKSIZE);
                    p->drawText(r, Qt::AlignCenter | Qt::TextDontClip, a->tickLabel(xx));
                }
            }
        }

        // Draw minor tickmarks
        const QList<double> minTickMarks = a->minorTickMarks();
        for (const double xx : minTickMarks) {
            double px = d->pixRect.width() * (xx - d->dataRect.x()) / d->dataRect.width();
            if (px > 0 && px < d->pixRect.width()) {
                p->drawLine(QPointF(px, double(d->pixRect.height() - TICKOFFSET)), //
                            QPointF(px, double(d->pixRect.height() - SMALLTICKSIZE - TICKOFFSET)));
            }
        }

        // Draw BottomAxis Label
        if (!a->label().isEmpty()) {
            QRect r(0, d->pixRect.height() + 2 * YPADDING, d->pixRect.width(), YPADDING);
            p->drawText(r, Qt::AlignCenter, a->label());
        }
    } // End of BottomAxis

    /*** LeftAxis ***/
    a = axis(LeftAxis);
    if (a->isVisible()) {
        // Draw axis line
        p->drawLine(0, 0, 0, d->pixRect.height());

        // Draw major tickmarks
        const QList<double> majMarks = a->majorTickMarks();
        for (const double yy : majMarks) {
            double py = d->pixRect.height() * (1.0 - (yy - d->dataRect.y()) / d->dataRect.height());
            if (py > 0 && py < d->pixRect.height()) {
                p->drawLine(QPointF(TICKOFFSET, py), QPointF(double(TICKOFFSET + BIGTICKSIZE), py));

                // Draw ticklabel
                if (a->areTickLabelsShown()) {
                    QRect r(-2 * BIGTICKSIZE - SMALLTICKSIZE, int(py) - SMALLTICKSIZE, 2 * BIGTICKSIZE, 2 * SMALLTICKSIZE);
                    p->drawText(r, Qt::AlignRight | Qt::AlignVCenter | Qt::TextDontClip, a->tickLabel(yy));
                }
            }
        }

        // Draw minor tickmarks
        const QList<double> minTickMarks = a->minorTickMarks();
        for (const double yy : minTickMarks) {
            double py = d->pixRect.height() * (1.0 - (yy - d->dataRect.y()) / d->dataRect.height());
            if (py > 0 && py < d->pixRect.height()) {
                p->drawLine(QPointF(TICKOFFSET, py), QPointF(double(TICKOFFSET + SMALLTICKSIZE), py));
            }
        }

        // Draw LeftAxis Label.  We need to draw the text sideways.
        if (!a->label().isEmpty()) {
            // store current painter translation/rotation state
            p->save();

            // translate coord sys to left corner of axis label rectangle, then rotate 90 degrees.
            p->translate(-3 * XPADDING, d->pixRect.height());
            p->rotate(-90.0);

            QRect r(0, 0, d->pixRect.height(), XPADDING);
            p->drawText(r, Qt::AlignCenter, a->label()); // draw the label, now that we are sideways

            p->restore(); // restore translation/rotation state
        }
    } // End of LeftAxis

    // Prepare for top and right axes; we may need the secondary data rect
    double x0 = d->dataRect.x();
    double y0 = d->dataRect.y();
    double dw = d->dataRect.width();
    double dh = d->dataRect.height();
    if (secondaryDataRect().isValid()) {
        x0 = secondaryDataRect().x();
        y0 = secondaryDataRect().y();
        dw = secondaryDataRect().width();
        dh = secondaryDataRect().height();
    }

    /*** TopAxis ***/
    a = axis(TopAxis);
    if (a->isVisible()) {
        // Draw axis line
        p->drawLine(0, 0, d->pixRect.width(), 0);

        // Draw major tickmarks
        const QList<double> majMarks = a->majorTickMarks();
        for (const double xx : majMarks) {
            double px = d->pixRect.width() * (xx - x0) / dw;
            if (px > 0 && px < d->pixRect.width()) {
                p->drawLine(QPointF(px, TICKOFFSET), QPointF(px, double(BIGTICKSIZE + TICKOFFSET)));

                // Draw ticklabel
                if (a->areTickLabelsShown()) {
                    QRect r(int(px) - BIGTICKSIZE, (int)-1.5 * BIGTICKSIZE, 2 * BIGTICKSIZE, BIGTICKSIZE);
                    p->drawText(r, Qt::AlignCenter | Qt::TextDontClip, a->tickLabel(xx));
                }
            }
        }

        // Draw minor tickmarks
        const QList<double> minMarks = a->minorTickMarks();
        for (const double xx : minMarks) {
            double px = d->pixRect.width() * (xx - x0) / dw;
            if (px > 0 && px < d->pixRect.width()) {
                p->drawLine(QPointF(px, TICKOFFSET), QPointF(px, double(SMALLTICKSIZE + TICKOFFSET)));
            }
        }

        // Draw TopAxis Label
        if (!a->label().isEmpty()) {
            QRect r(0, 0 - 3 * YPADDING, d->pixRect.width(), YPADDING);
            p->drawText(r, Qt::AlignCenter, a->label());
        }
    } // End of TopAxis

    /*** RightAxis ***/
    a = axis(RightAxis);
    if (a->isVisible()) {
        // Draw axis line
        p->drawLine(d->pixRect.width(), 0, d->pixRect.width(), d->pixRect.height());

        // Draw major tickmarks
        const QList<double> majMarks = a->majorTickMarks();
        for (const double yy : majMarks) {
            double py = d->pixRect.height() * (1.0 - (yy - y0) / dh);
            if (py > 0 && py < d->pixRect.height()) {
                p->drawLine(QPointF(double(d->pixRect.width() - TICKOFFSET), py), //
                            QPointF(double(d->pixRect.width() - TICKOFFSET - BIGTICKSIZE), py));

                // Draw ticklabel
                if (a->areTickLabelsShown()) {
                    QRect r(d->pixRect.width() + SMALLTICKSIZE, int(py) - SMALLTICKSIZE, 2 * BIGTICKSIZE, 2 * SMALLTICKSIZE);
                    p->drawText(r, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextDontClip, a->tickLabel(yy));
                }
            }
        }

        // Draw minor tickmarks
        const QList<double> minMarks = a->minorTickMarks();
        for (const double yy : minMarks) {
            double py = d->pixRect.height() * (1.0 - (yy - y0) / dh);
            if (py > 0 && py < d->pixRect.height()) {
                p->drawLine(QPointF(double(d->pixRect.width() - 0.0), py), QPointF(double(d->pixRect.width() - 0.0 - SMALLTICKSIZE), py));
            }
        }

        // Draw RightAxis Label.  We need to draw the text sideways.
        if (!a->label().isEmpty()) {
            // store current painter translation/rotation state
            p->save();

            // translate coord sys to left corner of axis label rectangle, then rotate 90 degrees.
            p->translate(d->pixRect.width() + 2 * XPADDING, d->pixRect.height());
            p->rotate(-90.0);

            QRect r(0, 0, d->pixRect.height(), XPADDING);
            p->drawText(r, Qt::AlignCenter, a->label()); // draw the label, now that we are sideways

            p->restore(); // restore translation/rotation state
        }
    } // End of RightAxis
}

int KPlotWidget::leftPadding() const
{
    if (d->leftPadding >= 0) {
        return d->leftPadding;
    }
    const KPlotAxis *a = axis(LeftAxis);
    if (a && a->isVisible() && a->areTickLabelsShown()) {
        return !a->label().isEmpty() ? 3 * XPADDING : 2 * XPADDING;
    }
    return XPADDING;
}

int KPlotWidget::rightPadding() const
{
    if (d->rightPadding >= 0) {
        return d->rightPadding;
    }
    const KPlotAxis *a = axis(RightAxis);
    if (a && a->isVisible() && a->areTickLabelsShown()) {
        return !a->label().isEmpty() ? 3 * XPADDING : 2 * XPADDING;
    }
    return XPADDING;
}

int KPlotWidget::topPadding() const
{
    if (d->topPadding >= 0) {
        return d->topPadding;
    }
    const KPlotAxis *a = axis(TopAxis);
    if (a && a->isVisible() && a->areTickLabelsShown()) {
        return !a->label().isEmpty() ? 3 * YPADDING : 2 * YPADDING;
    }
    return YPADDING;
}

int KPlotWidget::bottomPadding() const
{
    if (d->bottomPadding >= 0) {
        return d->bottomPadding;
    }
    const KPlotAxis *a = axis(BottomAxis);
    if (a && a->isVisible() && a->areTickLabelsShown()) {
        return !a->label().isEmpty() ? 3 * YPADDING : 2 * YPADDING;
    }
    return YPADDING;
}

void KPlotWidget::setLeftPadding(int padding)
{
    d->leftPadding = padding;
}

void KPlotWidget::setRightPadding(int padding)
{
    d->rightPadding = padding;
}

void KPlotWidget::setTopPadding(int padding)
{
    d->topPadding = padding;
}

void KPlotWidget::setBottomPadding(int padding)
{
    d->bottomPadding = padding;
}

void KPlotWidget::setDefaultPaddings()
{
    d->leftPadding = -1;
    d->rightPadding = -1;
    d->topPadding = -1;
    d->bottomPadding = -1;
}

#include "moc_kplotwidget.cpp"
