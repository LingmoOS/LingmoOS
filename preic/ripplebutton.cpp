#include "ripplebutton.h"
#include <QPainter>
#include <QMouseEvent>
#include <QParallelAnimationGroup>

RippleButton::RippleButton(QWidget *parent)
    : QPushButton(parent), m_rippleOpacity(0.0), m_rippleRadius(0.0)
{
    init();
}

RippleButton::RippleButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent), m_rippleOpacity(0.0), m_rippleRadius(0.0)
{
    init();
}

void RippleButton::init()
{
    setStyleSheet(
        "RippleButton {"
        "    background-color: #1976D2;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    font-size: 14px;"
        "    font-weight: 500;"
        "    text-transform: uppercase;"
        "    padding: 0 16px;"
        "    min-height: 36px;"
        "}"
        "RippleButton:hover {"
        "    background-color: #1E88E5;"
        "}"
        "RippleButton:pressed {"
        "    background-color: #1565C0;"
        "}"
    );

    m_rippleAnimation = new QPropertyAnimation(this, "rippleRadius", this);
    m_opacityAnimation = new QPropertyAnimation(this, "rippleOpacity", this);
}

void RippleButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);
    
    if (m_rippleRadius > 0) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        
        QColor rippleColor(255, 255, 255, int(m_rippleOpacity * 80));
        painter.setBrush(rippleColor);
        painter.setPen(Qt::NoPen);
        
        painter.drawEllipse(m_rippleCenter, m_rippleRadius, m_rippleRadius);
    }
}

void RippleButton::mousePressEvent(QMouseEvent *event)
{
    QPushButton::mousePressEvent(event);
    
    m_rippleCenter = event->pos();
    qreal endRadius = width() + height();
    
    m_rippleAnimation->setStartValue(0);
    m_rippleAnimation->setEndValue(endRadius);
    m_rippleAnimation->setDuration(400);
    m_rippleAnimation->setEasingCurve(QEasingCurve::OutQuad);
    
    m_opacityAnimation->setStartValue(1.0);
    m_opacityAnimation->setEndValue(0.0);
    m_opacityAnimation->setDuration(400);
    m_opacityAnimation->setEasingCurve(QEasingCurve::OutQuad);
    
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(m_rippleAnimation);
    group->addAnimation(m_opacityAnimation);
    group->start(QAbstractAnimation::DeleteWhenStopped);
} 