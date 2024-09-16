// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "equalbutton.h"

#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>

#include <DGuiApplicationHelper>

const qreal BLURRADIUS = 4; //阴影模糊半径
const int FONT_PIXELSIZE = 36; //字号
const qreal ROUND_XRADIUS = 8; //按钮圆角x轴半径
const qreal ROUND_YRADIUS = 8; //按钮圆角y轴半径

EqualButton::EqualButton(const QString &text, QWidget *parent)
    : DSuggestButton(text, parent),
      m_effect(new QGraphicsDropShadowEffect(this))
{
    setFocusPolicy(Qt::TabFocus);
    setObjectName("TextButton");

    init();
    m_isHover = m_isPress = false;
    m_effect->setOffset(0, 4); //阴影偏移
    m_effect->setBlurRadius(BLURRADIUS); //阴影模糊半径
}

EqualButton::~EqualButton()
{

}

/**
 * @brief 设置字体
 */
void EqualButton::init()
{
    m_font.setPixelSize(FONT_PIXELSIZE);
    m_font.setFamily("Noto Sans");
    m_font.setStyleName("Light");
}

/**
 * @brief 动画效果
 * @param msec 100ms
 */
void EqualButton::animate(bool isspace, int msec)
{
    if (m_isPress == false) { //edit for bug-20492 20200416
        m_isHover = false;  //edit for bug-20508 20200414
        if (!isspace)
            setDown(true);
        m_isPress = true;

        QTimer::singleShot(msec, this, [ = ] {
            if (!isspace)
                setDown(false);
            m_isPress = false;
            update();
        });
    }
}

/**
 * @brief 设置点击下的标置位
 */
void EqualButton::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::RightButton)
        return;
//    setFocus();
    m_isPress = true;
    m_isHover = false; //20200722删除foucus状态
    DPushButton::mousePressEvent(e);
}

/**
 * @brief 设置鼠标松开时的标志位
 */
void EqualButton::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::RightButton)
        return;
    this->setPalette(m_palette);
    m_isPress = false;
    if (this->rect().contains(e->pos())) {
        m_isacting = true;
        m_isHover = true;
    } else
        m_isacting = false;
    DPushButton::mouseReleaseEvent(e);
}

void EqualButton::enterEvent(QEvent *e)
{
    //m_font.setPixelSize(20);
    //m_font.setStyleName("");
    m_isHover = true;
    DPushButton::enterEvent(e);
}

void EqualButton::leaveEvent(QEvent *e)
{
    //m_font.setPixelSize(18);
    //m_font.setStyleName("Light");
    m_isHover = false;
    m_isacting = false;
    DPushButton::leaveEvent(e);
}

void EqualButton::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QRectF rect = this->rect();
    QRectF focusBase(rect.left() + 1, rect.top() + 1, rect.width() - 2, rect.height() - 2);
    QRectF normal(rect.left() + 2, rect.top() + 2, rect.width() - 4, rect.height() - 4);
    QLinearGradient linearGradient(rect.width() / 2, 0, rect.width() / 2, rect.height()); //线性渐变，起始点坐标rect.width() / 2, 0
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true); //反锯齿
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true); //光滑的象素映射变换算法
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setFont(m_font);
    QRectF textRect = painter.fontMetrics().boundingRect("=");
    textRect.moveCenter(rect.center());
    QColor actcolor = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color().name();//活动色
    QColor base, text, pressText, hover0, hover1, press0, press1, frame;
    QColor shadow;

    base = actcolor;
    base.setAlphaF(1);
    text = Qt::white;
    pressText = QColor(255, 255, 255);
    pressText.setAlphaF(0.6);
    press0 = actcolor;
    press1 = actcolor;
    hover0 = actcolor;
    hover1 = actcolor;
    int type = DGuiApplicationHelper::instance()->themeType();
    if (type == 1) { //浅色主题设置
        frame = Qt::white;

        press0.setAlphaF(0.8);
        press1.setAlphaF(1);
        hover0.setAlphaF(0.6);
        hover1.setAlphaF(0.8);
    } else { //深色主题设置
        frame = QColor(0, 0, 0);
        frame.setAlphaF(0.6);

        press0.setAlphaF(0.7);
        press1.setAlphaF(0.5);
        hover0.setAlphaF(1);
        hover1.setAlphaF(0.8);
    }
    shadow = actcolor;
    shadow.setAlphaF(0.2);

    if (hasFocus()) {
        painter.setPen(Qt::NoPen);
        if (m_isPress) {
            linearGradient.setColorAt(0, press0);
            linearGradient.setColorAt(1, press1);

            QPen pen;

            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(base));
            painter.drawRoundedRect(rect, ROUND_XRADIUS, ROUND_YRADIUS); //圆角半径单位为像素
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(frame));
            painter.drawRoundedRect(focusBase, ROUND_XRADIUS, ROUND_YRADIUS); //圆角半径单位为像素
            painter.setBrush(QBrush(linearGradient));
            painter.drawRoundedRect(normal, ROUND_XRADIUS, ROUND_YRADIUS); //圆角半径单位为像素
            pen.setColor(pressText);
            painter.setPen(pen);
            painter.setFont(m_font);
            painter.drawText(textRect, "=");
            m_effect->setColor(shadow);
            this->setGraphicsEffect(m_effect);
        } else {
            QPen pen;
//            if (m_isacting) {
//                painter.setPen(Qt::NoPen);
//                painter.setBrush(QBrush(base));
//                painter.drawRoundedRect(normal, 8, 8); //圆角半径单位为像素
//            } else {
            base.setAlphaF(0.8);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(base));
            painter.drawRoundedRect(rect, ROUND_XRADIUS, ROUND_YRADIUS); //圆角半径单位为像素
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(frame));
            painter.drawRoundedRect(focusBase, ROUND_XRADIUS, ROUND_YRADIUS); //圆角半径单位为像素
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(base));
            painter.drawRoundedRect(normal, ROUND_XRADIUS, ROUND_YRADIUS); //圆角半径单位为像素
//            }

            //painter.drawRoundRect(rect,10,10);
            pen.setColor(text);
            painter.setPen(pen);
            painter.setFont(m_font);
            painter.drawText(textRect, "=");
            m_effect->setColor(shadow);
            this->setGraphicsEffect(m_effect);
        }
    } else {
        painter.setPen(Qt::NoPen);
        if (m_isHover) { //hover的绘制
            linearGradient.setColorAt(0, hover0);
            linearGradient.setColorAt(1, hover1);
            painter.setBrush(linearGradient);
            painter.drawRoundedRect(normal, ROUND_XRADIUS, ROUND_YRADIUS); //圆角半径单位为像素
            QPen pen;
            pen.setColor(text);
            painter.setPen(pen);
            painter.setFont(m_font);
            painter.drawText(textRect, "=");
            m_effect->setColor(shadow);
            this->setGraphicsEffect(m_effect);
        } else if (m_isPress) { //press的绘制
            linearGradient.setColorAt(0, press0);
            linearGradient.setColorAt(1, press1);
            painter.setBrush(QBrush(linearGradient));
            painter.drawRoundedRect(normal, ROUND_XRADIUS, ROUND_YRADIUS); //圆角半径单位为像素
            QPen pen;
            pen.setColor(pressText);
            painter.setPen(pen);
            painter.setFont(m_font);
            painter.drawText(textRect, "=");
            m_effect->setColor(shadow);
            this->setGraphicsEffect(m_effect);
        } else { //normal的绘制
            base.setAlphaF(0.8);
            painter.setBrush(QBrush(base));
            painter.drawRoundedRect(normal, ROUND_XRADIUS, ROUND_YRADIUS); //圆角半径单位为像素
            QPen pen;
            pen.setColor(text);
            painter.setPen(pen);
            painter.setFont(m_font);
            painter.drawText(textRect, "=");
            m_effect->setColor(shadow);
            this->setGraphicsEffect(m_effect);
        }
    }
}

void EqualButton::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Up:
        emit focus(0);
        break;
    case Qt::Key_Down:
        emit focus(1);
        break;
    case Qt::Key_Left:
        emit focus(2);
        break;
    case Qt::Key_Right:
        emit focus(3);
        break;
    case Qt::Key_Space:
        emit space();
        break;
    default:
        DPushButton::keyPressEvent(e);
        break;
    }
}

