#include "ripple_button.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

RippleButton::RippleButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
    , m_opacity(0)
    , m_rippleSize(0)
{
    // 创建不透明度动画
    m_opacityAnimation = new QPropertyAnimation(this, "rippleOpacity", this);
    m_opacityAnimation->setDuration(1200);  // 增加到1.2秒
    m_opacityAnimation->setStartValue(0.3);  // 降低初始不透明度
    m_opacityAnimation->setEndValue(0.0);
    m_opacityAnimation->setEasingCurve(QEasingCurve::InOutCubic);  // 使用更平滑的双向缓动
    
    // 创建大小动画
    m_sizeAnimation = new QPropertyAnimation(this, "rippleSize", this);
    m_sizeAnimation->setDuration(1500);  // 增加到1.5秒
    m_sizeAnimation->setEasingCurve(QEasingCurve::OutQuint);  // 使用五次方缓动，更加平缓
}

void RippleButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    if (m_opacity > 0) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

        // 使用更柔和的颜色
        QColor rippleColor(255, 255, 255, m_opacity * 255);
        
        painter.setBrush(rippleColor);
        painter.setPen(Qt::NoPen);

        QRectF rippleRect(m_rippleCenter.x() - m_rippleSize/2,
                         m_rippleCenter.y() - m_rippleSize/2,
                         m_rippleSize, m_rippleSize);
        painter.drawEllipse(rippleRect);
    }
}

void RippleButton::mousePressEvent(QMouseEvent *event)
{
    m_rippleCenter = event->pos();
    
    // 计算最大波纹大小（稍微减小最大尺寸）
    qreal maxSize = 2.8 * qMax(width(), height());
    
    // 设置大小动画，从较小的初始值开始
    m_sizeAnimation->setStartValue(width() * 0.15);  // 从按钮宽度的15%开始
    m_sizeAnimation->setEndValue(maxSize);
    
    // 停止并重启动画
    m_opacityAnimation->stop();
    m_sizeAnimation->stop();
    m_opacityAnimation->start();
    m_sizeAnimation->start();
    
    QPushButton::mousePressEvent(event);
}

void RippleButton::setRippleOpacity(qreal opacity)
{
    m_opacity = opacity;
    update();
}

void RippleButton::setRippleSize(qreal size)
{
    m_rippleSize = size;
    update();
} 