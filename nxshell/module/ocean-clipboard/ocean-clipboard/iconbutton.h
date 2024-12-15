// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ICONBUTTON_H
#define ICONBUTTON_H
#include <DWidget>

DWIDGET_USE_NAMESPACE

/*!
 * \~chinese \class IconButton
 * \~chinese \brief 自定义一个控件,继承于DWidget
 */
class IconButton : public DWidget
{
    Q_OBJECT
public:
    explicit IconButton(QWidget *parent = nullptr);

    explicit IconButton(const QString &text, QWidget *parent = nullptr);

    inline const QString &text() { return m_text; }
    void setText(const QString &text);

    inline bool focusState() { return m_hasFocus; }
    void setFocusState(bool has);

    inline int backOpacity() { return m_opacity; }
    void setBackOpacity(int opacity);

    inline int radius() { return m_radius; }
    void setRadius(int radius);

Q_SIGNALS:
    void clicked();

private:
    QString m_text;
    bool m_hasFocus;
    bool m_hover;
    int m_opacity;
    int m_radius;
    bool m_hasBackColor;

protected:
    /*!
     * \~chinese \name paintEvent
     * \~chinese \brief 绘制控件的样式
     * \~chinese \param event 事件
     */
    virtual void paintEvent(QPaintEvent *event) override;
    /*!
     * \~chinese \name mousePressEvent
     * \~chinese \brief 鼠标点击事件
     * \~chinese \param event 事件
     */
    // TODO 鼠标事件就不要传递给父对象了，包括mouseReleaseEvent
    virtual void mousePressEvent(QMouseEvent *event) override;
    /*!
     * \~chinese \name enterEvent
     * \~chinese \brief 鼠标移动到控件上产生的事件,产生悬停的效果
     * \~chinese \param event 事件
     */
    virtual void enterEvent(QEnterEvent *event) override;
    /*!
     * \~chinese \name leaveEvent
     * \~chinese \brief 鼠标移出控件产生的事件,消除悬停的效果
     * \~chinese \param event 事件
     */
    virtual void leaveEvent(QEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual QSize sizeHint() const override;
};

#endif // ICONBUTTON_
