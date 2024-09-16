// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bitbutton.h"

const QSize PROGRAMMER_BITBTNSIZE = QSize(16, 26);

BitButton::BitButton(QWidget *parent)
    : DPushButton(parent)
{
    setText("0");
    setFlat(true);
    setFixedSize(PROGRAMMER_BITBTNSIZE);
    setFocusPolicy(Qt::TabFocus);
    init();
}

BitButton::~BitButton()
{
}

void BitButton::init()
{
    m_font.setPixelSize(18);
    m_font.setFamily("Noto Sans");
    m_font.setStyleName("Light");
    m_font.setWeight(2);
}

void BitButton::animate(bool isspace, int msec)
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
 * @brief BitButton::setButtonState
 * @param b:false--0 true--1
 * 设置按键的状态 0/1
 */
void BitButton::setButtonState(bool b)
{
    if (b)
        setText("1");
    else
        setText("0");
    m_btnState = b;
}

void BitButton::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::RightButton)
        return;
//    setFocus();

    m_isPress = true;
    m_isHover = false; //20200722删除foucus状态
    DPushButton::mousePressEvent(e);
}

void BitButton::mouseReleaseEvent(QMouseEvent *e)
{
    //    clearFocus();
    if (e->button() == Qt::RightButton)
        return;
    if (text() == QString("0")) {
        setText("1");
        m_btnState = true;
    } else {
        setText("0");
        m_btnState = false;
    }
    m_isPress = false;
    if (this->rect().contains(e->pos())) {
        m_isHover = true;
    }
    DPushButton::mouseReleaseEvent(e);
}

void BitButton::enterEvent(QEvent *e)
{
    m_isHover = true;
    DPushButton::enterEvent(e);
}

void BitButton::leaveEvent(QEvent *e)
{
    m_isHover = false;
    DPushButton::leaveEvent(e);
}

void BitButton::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QColor press = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color().name();//活动色
    QColor text, disabletext;
    QPainter painter(this);
    QPen pen;
    QRect focusrect(rect().left(), rect().top(), rect().width() - 1, rect().height() - 1);
    int type = DGuiApplicationHelper::instance()->themeType();
    if (type == 1) { //浅色主题设置
        text = QColor("#303030");
        disabletext = QColor("#555555");
        disabletext.setAlphaF(0.4);
    } else {
        text = QColor("#C0C6D4");
        disabletext = QColor("#C0C6D4");
        disabletext.setAlphaF(0.4);
    }
    if (m_isPress || m_btnState) {
        if (m_isPress)
            press.setAlphaF(0.7);
        else
            press.setAlphaF(1);
        pen.setColor(press);
        painter.setPen(pen);
    } else {
        if (this->isEnabled())
            pen.setColor(text);
        else
            pen.setColor(disabletext);
        painter.setPen(pen);
    }
    painter.setFont(m_font);
    QTextOption opt(Qt::AlignCenter);
    painter.drawText(this->rect(), this->text(), opt);
    if (hasFocus()) {
        painter.setPen(Qt::NoPen);
        pen.setWidth(1);
        pen.setColor(press);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(focusrect);
    }
//    DPushButton::paintEvent(e);
}

void BitButton::focusOutEvent(QFocusEvent *e)
{
    QPushButton::focusOutEvent(e);
}

void BitButton::keyPressEvent(QKeyEvent *e)
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
