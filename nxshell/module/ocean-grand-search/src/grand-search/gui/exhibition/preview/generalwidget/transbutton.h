#ifndef TRANSBUTTON
#define TRANSBUTTON

#include <DPushButton>

namespace GrandSearch {

// 透明背景 icon可左可右
class TransButton : public DTK_WIDGET_NAMESPACE::DPushButton
{
    Q_OBJECT
public:
    explicit TransButton(QWidget * parent = nullptr);
    TransButton(const QString& text, QWidget * parent = nullptr);
    TransButton(const QIcon &icon, const QString& text, QWidget * parent = nullptr);

    QSize sizeHint() const override;
    void changeText(const QString &text);
    void setIconRight(bool isTrue) { m_isIconRight = isTrue; }
    void setIconNoPressColor(bool isTrue = true) { m_isIconNoPressColor = isTrue; }
    void setAlpha(double alpha) { m_alpha = alpha; }
    void setNotAcceptFocus(bool isTrue) { m_isNotAcceptFocus = isTrue; }

protected:
    void paintEvent(QPaintEvent* e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void updateRectSize();
    QIcon setIconColor(QIcon icon, QColor color);

private:
    bool m_isHover            = false;
    bool m_isPress            = false;
    bool m_isIconRight        = false;
    bool m_isIconNoPressColor = false;
    bool m_isNotAcceptFocus   = false;
    QIcon m_iconOrig;
    double m_alpha = 1.0;
};
}

#endif // TRANSBUTTON
