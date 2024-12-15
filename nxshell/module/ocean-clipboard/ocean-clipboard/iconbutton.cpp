// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iconbutton.h"

#include <QPainter>
#include <QIcon>
#include <QStyle>
#include <QDebug>
#include <QMouseEvent>

IconButton::IconButton(QWidget *parent)
    : DWidget(parent)
    , m_text("")
    , m_hasFocus(false)
    , m_hover(false)
    , m_opacity(0)
    , m_radius(8)
    , m_hasBackColor(false)
{

}

/*!
 * \~chinese \name IconButton
 * \~chinese \brief 创建一个带文字的按钮
 * \~chinese \param text 按钮上需要显示的文字
 */
IconButton::IconButton(const QString &text, QWidget *parent)
    : IconButton(parent)
{
    setText(text);
}

/*!
 * \~chinese \name setText
 * \~chinese \brief 设置按钮上显示的文字
 * \~chinese \param text 按钮上需要显示的文字
 */
void IconButton::setText(const QString &text)
{
    m_text = text;

    update();
}

void IconButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QColor color;
    if (m_hasBackColor) {
        color = palette().color(QPalette::Base);
        color.setAlpha(m_hover ? m_opacity : (m_opacity / 2));
    } else {
        color = palette().color(QPalette::WindowText);
        color.setAlpha(m_hasFocus ? 80 : (m_hover ? 50 : 20));
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundedRect(rect(), m_radius, m_radius);

    QTextOption option;
    option.setAlignment(Qt::AlignCenter);
    painter.setPen(palette().color(QPalette::WindowText));
    painter.drawText(rect(), m_text, option);

    if (m_text.isEmpty()) {
        QPixmap pix = style()->standardIcon(QStyle::SP_TitleBarCloseButton).pixmap(width());
        painter.drawPixmap(rect(), pix);
    }
}

QSize IconButton::sizeHint() const
{
    return QSize(fontMetrics().horizontalAdvance(m_text) + 20, fontMetrics().height() + 10);
}

void IconButton::resizeEvent(QResizeEvent *event)
{
    resize(QSize(fontMetrics().horizontalAdvance(m_text) + 20, fontMetrics().height() + 10));
    DWidget::resizeEvent(event);
}

/*!
 * \~chinese \name setFocusState
 * \~chinese \brief 控制按钮被选中时,设置m_hasFocus的状态
 * \~chinese \param has 是否被选中
 */
void IconButton::setFocusState(bool has)
{
    m_hasFocus = has;

    update();
}

/*!
 * \~chinese \name setBackOpacity
 * \~chinese \brief 设置控件背景不透明度
 * \~chinese \param opacity 不透明度,取值范围为(0-255)
 */
void IconButton::setBackOpacity(int opacity)
{
    if (opacity < 0 || opacity > 255)
        return;

    m_opacity = opacity;

    m_hasBackColor = true;

    update();
}

/*!
 * \~chinese \name setRadius
 * \~chinese \brief 设置控件圆角半径
 * \~chinese \param radius 半径,当输入的半径<=0时控件显示为直角
 */
void IconButton::setRadius(int radius)
{
    m_radius = radius;

    update();
}

void IconButton::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if (event->button() == Qt::LeftButton)
        Q_EMIT clicked();
}

void IconButton::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event);
    m_hover = true;

    update();

    return DWidget::enterEvent(event);
}

void IconButton::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_hover = false;

    update();

    return DWidget::leaveEvent(event);
}
