// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//#include "blureframe.h"
//#include "utils/baseutils.h"
//#include <QApplication>
//#include <QDebug>
//#include <QPainter>
//#include <QGraphicsScene>
//#include <QGraphicsPixmapItem>
//#include <QGraphicsBlurEffect>
//#include <QPropertyAnimation>
//#include <QMouseEvent>

//const int ANIMATION_DURATION = 500;
//const QEasingCurve ANIMATION_EASING_CURVE = QEasingCurve::InOutCubic;

//BlurFrame::BlurFrame(QWidget *parent)
//    : DBlurEffectWidget(parent),
//      m_borderColor(Qt::transparent),
//      m_borderRadius(0),
//      m_borderWidth(0),
//      m_coverBrush(QBrush(QColor(0, 0, 0, 200)))
//{
//    setRadius(50);
//}

////void BlurFrame::setCoverBrush(const QBrush &brush)
////{
////    m_coverBrush = brush;
////    update();
////}

////void BlurFrame::setPos(const QPoint &pos)
////{
////    DBlurEffectWidget::move(pos);
////}

//void BlurFrame::paintEvent(QPaintEvent *e)
//{
//    DBlurEffectWidget::paintEvent(e);

//    QPainter p(this);
//    p.setRenderHint(QPainter::Antialiasing);

//    QPainterPath path;
//    // Draw outside border
//    path.addRoundedRect(this->rect(), m_borderRadius, m_borderRadius);
//    QPen pen(m_borderColor, m_borderWidth);
//    p.setPen(pen);
//    p.drawPath(path);

//    // Draw content
//    QRect insideRect;
//    insideRect.setRect(this->rect().x() + m_borderWidth,
//                       this->rect().y() + m_borderWidth,
//                       this->rect().width() - m_borderWidth * 2,
//                       this->rect().height() - m_borderWidth * 2);
//    QPainterPath ip;
//    ip.addRoundedRect(insideRect, m_borderRadius, m_borderRadius);
//    p.setClipPath(ip);

//    p.fillRect(0, 0, width(), height(), m_coverBrush);

//    p.end();
//}

////void BlurFrame::moveWithAnimation(int x, int y)
////{
////    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");
////    animation->setDuration(ANIMATION_DURATION);
////    animation->setEasingCurve(ANIMATION_EASING_CURVE);
////    animation->setStartValue(pos());
////    animation->setEndValue(QPoint(x, y));
////    animation->start();
////    connect(this, &BlurFrame::requestStopAnimation,
////            animation, &QPropertyAnimation::stop);
////    connect(this, &BlurFrame::requestStopAnimation,
////            animation, &QPropertyAnimation::deleteLater);
////    connect(animation, &QPropertyAnimation::finished,
////            animation, &QPropertyAnimation::deleteLater);
////}

////QColor BlurFrame::getBorderColor() const
////{
////    return m_borderColor;
////}

////void BlurFrame::setBorderColor(const QColor &borderColor)
////{
////    m_borderColor = borderColor;
////}

//void BlurFrame::keyPressEvent(QKeyEvent *e)
//{
//    if (e->key() == Qt::Key_Escape) {
//        this->close();
//        return;
//    }
//    DBlurEffectWidget::keyPressEvent(e);
//}

//void BlurFrame::mousePressEvent(QMouseEvent *event) {
//    m_dragPos = event->globalPos() - mapToGlobal(QPoint(0, 0));

//    DBlurEffectWidget::mousePressEvent(event);
//}

////void BlurFrame::setMoveEnable(bool move) {
////    m_moveEnable = move;
////}
//void BlurFrame::mouseMoveEvent(QMouseEvent *event) {
//    if (m_moveEnable)
//        this->move(event->globalPos() - m_dragPos);
//    DBlurEffectWidget::mouseMoveEvent(event);
//}

//void BlurFrame::mouseReleaseEvent(QMouseEvent *event) {
//    DBlurEffectWidget::mouseReleaseEvent(event);
//}

////int BlurFrame::getBorderWidth() const
////{
////    return m_borderWidth;
////}

////void BlurFrame::setBorderWidth(int borderWidth)
////{
////    m_borderWidth = borderWidth;
////}

////int BlurFrame::getBorderRadius() const
////{
////    return m_borderRadius;
////}

////void BlurFrame::setBorderRadius(int borderRadius)
////{
////    m_borderRadius = borderRadius;
////}
