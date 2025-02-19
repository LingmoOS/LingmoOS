#ifndef RIPPLE_BUTTON_H
#define RIPPLE_BUTTON_H

#include <QPushButton>
#include <QPropertyAnimation>

class RippleButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(qreal rippleOpacity READ rippleOpacity WRITE setRippleOpacity)
    Q_PROPERTY(qreal rippleSize READ rippleSize WRITE setRippleSize)

public:
    explicit RippleButton(const QString &text, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    qreal rippleOpacity() const { return m_opacity; }
    void setRippleOpacity(qreal opacity);
    
    qreal rippleSize() const { return m_rippleSize; }
    void setRippleSize(qreal size);

    QPropertyAnimation *m_opacityAnimation;
    QPropertyAnimation *m_sizeAnimation;
    QPoint m_rippleCenter;
    qreal m_opacity;
    qreal m_rippleSize;
};

#endif // RIPPLE_BUTTON_H 