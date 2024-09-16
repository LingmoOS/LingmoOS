// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ICONBUTTON_H
#define ICONBUTTON_H
#include <DWidget>

DWIDGET_USE_NAMESPACE
/*!
 * \~chinese \class IconButton
 * \~chinese \brief 自定义按钮类
 */
class IconButton : public DWidget
{
    Q_OBJECT
public:
    enum ButtonState {
        HOVER,
        PRESS,
        RELEASE,
    };

    explicit IconButton(QWidget *parent = nullptr);
    explicit IconButton(const QString &text, QWidget *parent = nullptr);

    void setText(const QString &text);
    void setIcon(const QString &iconPath);
    void setFocusState(bool has);
    void setOpacity(const ButtonState state, int opacity);
    void setRadius(int radius);

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;

Q_SIGNALS:
    void clicked();

private:
    QString m_text;
    bool m_hasFocus;
    bool m_hover;
    int m_hoverOpacity;
    int m_pressOpacity;
    int m_releaseOpacity;
    int m_radius;
    ButtonState m_currentState;
    QPoint m_pressPos;
    QString m_iconPath;
};

#endif // ICONBUTTON_
