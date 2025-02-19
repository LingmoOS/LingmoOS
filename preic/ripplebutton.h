#ifndef RIPPLEBUTTON_H
#define RIPPLEBUTTON_H

#include <QPushButton>
#include <QPropertyAnimation>

class RippleButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(qreal rippleOpacity READ rippleOpacity WRITE setRippleOpacity)
    Q_PROPERTY(qreal rippleRadius READ rippleRadius WRITE setRippleRadius)

public:
    explicit RippleButton(QWidget *parent = nullptr);
    explicit RippleButton(const QString &text, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    qreal m_rippleOpacity;
    qreal m_rippleRadius;
    QPointF m_rippleCenter;
    QPropertyAnimation *m_rippleAnimation;
    QPropertyAnimation *m_opacityAnimation;

    qreal rippleOpacity() const { return m_rippleOpacity; }
    void setRippleOpacity(qreal opacity) { m_rippleOpacity = opacity; update(); }
    qreal rippleRadius() const { return m_rippleRadius; }
    void setRippleRadius(qreal radius) { m_rippleRadius = radius; update(); }
    void init();
};

#endif // RIPPLEBUTTON_H 