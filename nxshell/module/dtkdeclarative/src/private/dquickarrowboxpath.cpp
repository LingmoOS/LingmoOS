// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dquickarrowboxpath_p.h"

#include <QtMath>

DQUICK_BEGIN_NAMESPACE

DQuickArrowBoxPath::DQuickArrowBoxPath(QObject *parent)
    : QQuickCurve{parent}
    , m_arrowDirection(Up)
    , m_width(100)
    , m_height(100)
    , m_arrowX(50)
    , m_arrowY(0)
    , m_arrowWidth(30)
    , m_arrowHeight(20)
    , m_roundedRadius(0)
    , m_spread(0)
{

}

qreal DQuickArrowBoxPath::width() const
{
    return m_width;
}

void DQuickArrowBoxPath::setWidth(qreal newWidth)
{
    if (qFuzzyCompare(m_width, newWidth))
        return;
    m_width = newWidth;
    Q_EMIT widthChanged();
    Q_EMIT changed();
}

qreal DQuickArrowBoxPath::height() const
{
    return m_height;
}

void DQuickArrowBoxPath::setHeight(qreal newHeight)
{
    if (qFuzzyCompare(m_height, newHeight))
        return;
    m_height = newHeight;
    Q_EMIT heightChanged();
    Q_EMIT changed();
}

const DQuickArrowBoxPath::Direction &DQuickArrowBoxPath::arrowDirection() const
{
    return m_arrowDirection;
}

void DQuickArrowBoxPath::setArrowDirection(const Direction &newArrowDirection)
{
    if (m_arrowDirection == newArrowDirection)
        return;
    m_arrowDirection = newArrowDirection;
    Q_EMIT arrowDirectionChanged();
    Q_EMIT changed();
}

qreal DQuickArrowBoxPath::arrowX() const
{
    return m_arrowX;
}

void DQuickArrowBoxPath::setArrowX(qreal newArrowX)
{
    if (qFuzzyCompare(m_arrowX, newArrowX))
        return;
    m_arrowX = newArrowX;
    Q_EMIT arrowXChanged();
    Q_EMIT changed();
}

qreal DQuickArrowBoxPath::arrowY() const
{
    return m_arrowY;
}

void DQuickArrowBoxPath::setArrowY(qreal newArrowY)
{
    if (qFuzzyCompare(m_arrowY, newArrowY))
        return;
    m_arrowY = newArrowY;
    Q_EMIT arrowYChanged();
    Q_EMIT changed();
}

qreal DQuickArrowBoxPath::arrowWidth() const
{
    return m_arrowWidth;
}

void DQuickArrowBoxPath::setArrowWidth(qreal newArrowWidth)
{
    if (qFuzzyCompare(m_arrowWidth, newArrowWidth))
        return;
    m_arrowWidth = newArrowWidth;
    Q_EMIT arrowWidthChanged();
    Q_EMIT changed();
}

qreal DQuickArrowBoxPath::arrowHeight() const
{
    return m_arrowHeight;
}

void DQuickArrowBoxPath::setArrowHeight(qreal newArrowHeight)
{
    if (qFuzzyCompare(m_arrowHeight, newArrowHeight))
        return;
    m_arrowHeight = newArrowHeight;
    Q_EMIT arrowHeightChanged();
    Q_EMIT changed();
}

qreal DQuickArrowBoxPath::roundedRadius() const
{
    return m_roundedRadius;
}

void DQuickArrowBoxPath::setRoundedRadius(qreal newRoundedRadius)
{
    if (qFuzzyCompare(m_roundedRadius, newRoundedRadius))
        return;
    m_roundedRadius = newRoundedRadius;
    Q_EMIT roundedRadiusChanged();
    Q_EMIT changed();
}

qreal DQuickArrowBoxPath::spread() const
{
    return m_spread;
}

void DQuickArrowBoxPath::setSpread(qreal newSpread)
{
    if (qFuzzyCompare(m_spread, newSpread))
        return;
    m_spread = newSpread;
    Q_EMIT spreadChanged();
    Q_EMIT changed();
}

// 线段ab的夹角大小
static inline qreal abAngle(qreal a, qreal b, qreal c) {
    const qreal ab = (pow(a, 2) + pow(b, 2) - pow(c, 2)) / (2 * a * b);
    return 180 / M_PI * qAcos(ab);
}

// 两点之间的距离
static inline qreal lineLength(const QPointF &p1, const QPointF &p2)
{
    return sqrt(pow(qAbs(p2.x() - p1.x()), 2) + pow(qAbs(p2.y() - p1.y()), 2));
}

/*
 * 根据传入的三角形顶点和圆的半径，计算三角形两条边线与圆的两个内切点坐标
 *
 *                        A
 *                        ^
 *                       /.\
 *                      / . \
 *                     /  .  \
 *             P1 --->╭-------╮<--- P2
 *                   /| r |   |\
 *                  / |   |O  | \
 *                 /  ╰-------╯  \
 *                /       .       \
 *               /        .        \
 *              B         h         C
 *
 * A B C 为三角形三个顶点
 * r 为圆的半径
 * O 为圆心
 * h 为经过点 A 和点 R 的线段
 * P1 为线段 AB 和圆的切点
 * P2 为线段 AC 和圆的切点
 */
QPair<QPointF, QPointF> getPointsOfContact(qreal r, const QPointF &A, const QPointF &B, const QPointF &C)
{
    const qreal AB = lineLength(A, B);
    const qreal AC = lineLength(A, C);
    // 三角形两条边中的最小值
    const qreal min = qMin(AB, AC);
    // 起点为A，在线段AB上长度为min的点
    const QPointF B1(B.x() + (A.x() - B.x()) * (1.0 - min / AB),
                     B.y() + (A.y() - B.y()) * (1.0 - min / AB));
    // 起点为A，在线段AB上长度为min的点
    const QPointF C1(C.x() + (A.x() - C.x()) * (1.0 - min / AC),
                     C.y() + (A.y() - C.y()) * (1.0 - min / AC));

    /* A B1 C1 形成一个等腰三角形 */

    // 线段B1 C1的中心点
    const QPointF center(B1.x() + (C1.x() - B1.x()) / 2,
                         B1.y() + (C1.y() - B1.y()) / 2);
    // 点A到线段B1 C1的垂线长度
    const qreal H1 = lineLength(A, center);
    // 线段A P1的长度
    const qreal AP1 = H1 * (r / lineLength(B1, center));
    const qreal &AP2 = AP1; // 因为是等腰的

    QPointF P1, P2;

    if (AP1 < AB && AP2 < AC) {
        // 如果点P1和P2均分别落在线段AB和AC上
        P1.setX(B.x() + (AP1 / AB) * (A.x() - B.x()));
        P1.setY(B.y() + (AP1 / AB) * (A.y() - B.y()));
        P2.setX(C.x() + (AP1 / AC) * (A.x() - C.x()));
        P2.setY(C.y() + (AP1 / AC) * (A.y() - C.y()));
        return qMakePair(P1, P2);
    } else if (AB < AP1 && AC < AP2) {
        // 如果P1不在线段AB上，也就是在AB的延长线上，则圆应该直接与B点相接
        // 如果P2不在线段AC上，也就是在AC的延长线上，则圆应该直接与C点相接
        return qMakePair(B, C);
    }

    // A 的角度
    const qreal AAngle = abAngle(AB, AC, lineLength(B, C));

    if (AB < AP1) {
        P1 = B;
        /* 已知点P1，计算点P2。设点B做垂线到线段AC的交点为 B2*/
        // 线段 B B2 的长度
        const qreal BB2 = sin(AAngle) * AB;
        // 线段 A B2 的长度
        const qreal AB2 = cos(AAngle) * AB;
        // 线段 B2 P2 的长度
        const qreal B2P2 = sqrt(pow(r, 2) - pow(r - BB2, 2));
        // 线段 A P2 的长度
        const qreal AP2 = AB2 + B2P2;

        P2.setX(A.x() - (AP2 / AC) * (A.x() - C.x()));
        P2.setX(A.y() - (AP2 / AC) * (A.y() - C.y()));
    } else {
        P2 = C;
        /* 已知点P2，计算点P1。设点B做垂线到线段AB的交点为 C2*/
        // 线段 C C2 的长度
        const qreal CC2 = sin(AAngle) * AC;
        // 线段 A C2 的长度
        const qreal AC2 = cos(AAngle) * AC;
        // 线段 C2 P1 的长度
        const qreal C2P1 = sqrt(pow(r, 2) - pow(r - CC2, 2));
        // 线段 A P1 的长度
        const qreal AP1 = AC2 + C2P1;

        P1.setX(A.x() - (AP1 / AB) * (A.x() - B.x()));
        P1.setX(A.y() - (AP1 / AB) * (A.y() - B.y()));
    }

    return qMakePair(P1, P2);
}

// Copy from QQuickContext2D begin
void arc(QPainterPath &path, qreal xc, qreal yc, qreal radius, qreal sar, qreal ear, bool antiClockWise)
{
    if (sar == ear)
        return;


    //### HACK

    // In Qt we don't switch the coordinate system for degrees
    // and still use the 0,0 as bottom left for degrees so we need
    // to switch
    sar = -sar;
    ear = -ear;
    antiClockWise = !antiClockWise;
    //end hack

    float sa = qRadiansToDegrees(sar);
    float ea = qRadiansToDegrees(ear);

    double span = 0;

    double xs     = xc - radius;
    double ys     = yc - radius;
    double width  = radius*2;
    double height = radius*2;
    if ((!antiClockWise && (ea - sa >= 360)) || (antiClockWise && (sa - ea >= 360)))
        // If the anticlockwise argument is false and endAngle-startAngle is equal to or greater than 2*PI, or, if the
        // anticlockwise argument is true and startAngle-endAngle is equal to or greater than 2*PI, then the arc is the whole
        // circumference of this circle.
        span = 360;
    else {
        if (!antiClockWise && (ea < sa)) {
            span += 360;
        } else if (antiClockWise && (sa < ea)) {
            span -= 360;
        }
        //### this is also due to switched coordinate system
        // we would end up with a 0 span instead of 360
        if (!(qFuzzyCompare(span + (ea - sa) + 1, 1) &&
              qFuzzyCompare(qAbs(span), 360))) {
            span   += ea - sa;
        }
    }

    // If the path is empty, move to where the arc will start to avoid painting a line from (0,0)
    if (!path.elementCount())
        path.arcMoveTo(xs, ys, width, height, sa);
    else if (!radius) {
        path.lineTo(xc, yc);
        return;
    }

    if (qIsNaN(span))
        return;
    path.arcTo(xs, ys, width, height, sa, span);
}

void addArcTo(QPainterPath &path, const QPointF &p1, const QPointF &p2, qreal r)
{
    QPointF p0(path.currentPosition());

    QPointF p1p0((p0.x() - p1.x()), (p0.y() - p1.y()));
    QPointF p1p2((p2.x() - p1.x()), (p2.y() - p1.y()));
    float p1p0_length = std::sqrt(p1p0.x() * p1p0.x() + p1p0.y() * p1p0.y());
    float p1p2_length = std::sqrt(p1p2.x() * p1p2.x() + p1p2.y() * p1p2.y());

    double cos_phi = (p1p0.x() * p1p2.x() + p1p0.y() * p1p2.y()) / (p1p0_length * p1p2_length);

    // The points p0, p1, and p2 are on the same straight line (HTML5, 4.8.11.1.8)
    // We could have used areCollinear() here, but since we're reusing
    // the variables computed above later on we keep this logic.
    if (qFuzzyCompare(std::abs(cos_phi), 1.0)) {
        path.lineTo(p1);
        return;
    }

    float tangent = r / std::tan(std::acos(cos_phi) / 2);
    float factor_p1p0 = tangent / p1p0_length;
    QPointF t_p1p0((p1.x() + factor_p1p0 * p1p0.x()), (p1.y() + factor_p1p0 * p1p0.y()));

    QPointF orth_p1p0(p1p0.y(), -p1p0.x());
    float orth_p1p0_length = std::sqrt(orth_p1p0.x() * orth_p1p0.x() + orth_p1p0.y() * orth_p1p0.y());
    float factor_ra = r / orth_p1p0_length;

    // angle between orth_p1p0 and p1p2 to get the right vector orthographic to p1p0
    double cos_alpha = (orth_p1p0.x() * p1p2.x() + orth_p1p0.y() * p1p2.y()) / (orth_p1p0_length * p1p2_length);
    if (cos_alpha < 0.f)
        orth_p1p0 = QPointF(-orth_p1p0.x(), -orth_p1p0.y());

    QPointF p((t_p1p0.x() + factor_ra * orth_p1p0.x()), (t_p1p0.y() + factor_ra * orth_p1p0.y()));

    // calculate angles for addArc
    orth_p1p0 = QPointF(-orth_p1p0.x(), -orth_p1p0.y());
    float sa = std::acos(orth_p1p0.x() / orth_p1p0_length);
    if (orth_p1p0.y() < 0.f)
        sa = 2 * M_PI - sa;

    // anticlockwise logic
    bool anticlockwise = false;

    float factor_p1p2 = tangent / p1p2_length;
    QPointF t_p1p2((p1.x() + factor_p1p2 * p1p2.x()), (p1.y() + factor_p1p2 * p1p2.y()));
    QPointF orth_p1p2((t_p1p2.x() - p.x()), (t_p1p2.y() - p.y()));
    float orth_p1p2_length = std::sqrt(orth_p1p2.x() * orth_p1p2.x() + orth_p1p2.y() * orth_p1p2.y());
    float ea = std::acos(orth_p1p2.x() / orth_p1p2_length);
    if (orth_p1p2.y() < 0)
        ea = 2 * M_PI - ea;
    if ((sa > ea) && ((sa - ea) < M_PI))
        anticlockwise = true;
    if ((sa < ea) && ((ea - sa) > M_PI))
        anticlockwise = true;

    arc(path, p.x(), p.y(), r, sa, ea, anticlockwise);
}
// Copy from QQuickContext2D end

void DQuickArrowBoxPath::addToPath(QPainterPath &path, const QQuickPathData &)
{
    QRectF rect(0, 0, m_width, m_height);
    qreal arrowX = m_arrowX;
    qreal arrowY = m_arrowY;
    const qreal &arrowWidth = m_arrowWidth;
    const qreal &arrowHeight = m_arrowHeight;
    const qreal &r = m_roundedRadius;
    qreal rotate = 0;
    QPointF translate(0, 0);

    // 先将其它方向的坐标都转换成Up的坐标，将箭头统一按照Up的方式计算，再通过旋转生成其它方向的path
    switch (m_arrowDirection) {
    case Right:
        arrowX = m_arrowY;
        arrowY = m_width - m_arrowX;
        rect.setWidth(m_height);
        rect.setHeight(m_width);
        rotate = 90;
        translate.ry() = -m_width;
        break;
    case Down:
        arrowX = rect.right() - m_arrowX;
        arrowY = m_height - m_arrowY;
        rotate = 180;
        translate.rx() = -m_width;
        translate.ry() = -m_height;
        break;
    case Left:
        arrowX = m_height - m_arrowY;
        arrowY = m_arrowX;
        rect.setWidth(m_height);
        rect.setHeight(m_width);
        rotate = 270;
        translate.rx() = -m_height;
        break;
    default: break;
    }

    /* 设点A为箭头顶点， 点B和点C分别为箭头与矩形的两个交点 */

    const QPointF A(arrowX, arrowY);
    path.setFillRule(Qt::WindingFill);
    path.moveTo(A);

    rect.setTop(arrowY + arrowHeight);
    QPointF B(qMax(arrowX - arrowWidth / 2, rect.left()), rect.top());
    QPointF C(qMin(arrowX + arrowWidth / 2, rect.right()), rect.top());

    if (r > 0) {
        QRectF rectRounded(0, 0, qMin(2 * r, rect.width() / 2), qMin(2 * r, rect.height() / 2));

        rectRounded.moveBottomLeft(rect.bottomLeft());
        path.moveTo(rectRounded.topLeft());
        // 左下角圆角
        path.arcTo(rectRounded, 180, 90);
        rectRounded.moveBottomRight(rect.bottomRight());
        // 右下角圆角
        path.arcTo(rectRounded, -90, 90);

        auto rr = qMin(r, (C.x() - B.x()) / 2);

        rectRounded.setWidth(qMin(rectRounded.width(), rect.right() - C.x()));
        if (rectRounded.width() > 0) {
            rectRounded.moveTopRight(rect.topRight());
            // 右上角
            path.arcTo(rectRounded, 0, 90);
        }
        if (qIsNull(rectRounded.width()) || path.currentPosition().x() - C.x() >= rr) {
            // 箭头右下角
            addArcTo(path, C, A, rr);
        }
        // 箭头自身
        addArcTo(path, A, B, qMin(rr, r / 2));

        rectRounded.setWidth(qMin(qMin(2 * r, rect.width() / 2), B.x() - rect.left()));
        if (rectRounded.width() > 0) {
            rectRounded.moveTopLeft(rect.topLeft());
            if (B != rectRounded.topRight()) {
                // 箭头左下角
                addArcTo(path, B, rectRounded.topRight(), rr);
            }
            // 左上角
            path.arcTo(rectRounded, 90, 90);
        } else {
            // 左上角
            addArcTo(path, B, rect.bottomLeft(), qMin(r, rectRounded.height()));
        }
    } else {
        path.lineTo(C);
        path.lineTo(rect.topRight());
        path.lineTo(rect.bottomRight());
        path.lineTo(rect.bottomLeft());
        path.lineTo(rect.topLeft());
        path.lineTo(B);
    }
    path.closeSubpath();

    QTransform tf;
    tf.rotate(rotate);
    tf.translate(translate.x(), translate.y());
    path = tf.map(path);

    qreal spread = qAbs(m_spread);
    if (spread > 0) {
        QPainterPathStroker pps;
        pps.setWidth(m_spread * 2);
        pps.setJoinStyle(Qt::MiterJoin);
        pps.setCapStyle(Qt::SquareCap);
        if (m_spread > 0) {
            path |= pps.createStroke(path);
        } else {
            path -= pps.createStroke(path);
        }
    }
}

DQUICK_END_NAMESPACE
