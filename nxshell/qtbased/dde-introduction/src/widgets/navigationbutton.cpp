// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "navigationbutton.h"

NavigationButton::NavigationButton(QString text, DWidget *parent)
    : DPushButton(text, parent)
{
    setCheckable(true);
    setFocusPolicy(Qt::NoFocus);
    this->setFlat(true);
    this->setText("");
    isEnter = false;

    m_label = new DLabel(this);
    m_label->setElideMode(Qt::ElideRight);
    m_label->setText(text);
    m_label->setFixedSize(95, 30);
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setContentsMargins(5, 0, 0, 0);
    layout->addWidget(m_label);
    setLayout(layout);
    initButton();
    needFrame = false;
    m_label->setForegroundRole(DPalette::WindowText);

    //主题变化时，导航按钮跟着变化颜色
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this,
            &NavigationButton::initButton);
    //导航按钮点击时，颜色也跟随主题变化
    connect(this, &NavigationButton::clicked, this, &NavigationButton::initButton);
}

void NavigationButton::mousePressEvent(QMouseEvent *event)
{
    needFrame = false;
    isEnter = false;
    DPushButton::mousePressEvent(event);
}


/*******************************************************************************
 1. @函数:    initButton
 2. @作者:
 3. @日期:    2020-12-09
 4. @说明:    根据主题设置m_label的Palette颜色
*******************************************************************************/
void NavigationButton::initButton()
{
    int type = DGuiApplicationHelper::instance()->themeType();
    DPalette pl = m_label->palette();
    if (type == DGuiApplicationHelper::LightType) {
        if (isChecked()) {
            pl.setColor(DPalette::WindowText, Qt::white);
        } else {
            pl.setColor(DPalette::WindowText, QColor(65, 77, 104));
        }
    } else {
        if (isChecked()) {
            pl.setColor(DPalette::WindowText, Qt::white);
        } else {
            pl.setColor(DPalette::WindowText, QColor(192, 198, 212));
        }
    }
    m_label->setPalette(pl);
}

void NavigationButton::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    isEnter = true;
}

void NavigationButton::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    isEnter = false;
}

void NavigationButton::paintEvent(QPaintEvent *event)
{
    //灰色背景绘制
    if (isEnter) {
        QRect rect = this->rect();
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        DPalette pa;
        painter.setBrush(pa.light());
        QColor penColor = pa.base().color();
        painter.setPen(QPen(penColor));

        QPainterPath painterPath;
        painterPath.addRoundedRect(rect, 8, 8);
        painter.drawPath(painterPath);
    }

    DPushButton::paintEvent(event);

    //画框 tab切换时
    if (needFrame) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        QPainterPath path;
        path.addRoundedRect(rect().adjusted(4, 4, -4, -4), 5, 5);
        painter.setClipRect(QRect(), Qt::NoClip);

        int type = DGuiApplicationHelper::instance()->themeType();
        if (type == 1) {
            QColor color(Qt::white);
            QPen pen(color);
            pen.setWidth(1);
            painter.setPen(pen);
            painter.drawPath(path);
        } else {
            QColor color(QColor("#282828"));
            QPen pen(color);
            pen.setWidth(1);
            painter.setPen(pen);
            painter.drawPath(path);
        }
    }

    initButton();
}


/*QSize NavigationButton::setHint()
{
    QFontMetrics fm(this->font());
    QRect rec = fm.boundingRect(m_label->text());
    return rec.size();
}*/

/*void NavigationButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QFont font;
    font.setFamily("SourceHanSansSC-Medium");
    font.setPixelSize(14);
    painter.setFont(font);
    QRect r = rect();
    r.setLeft(rect().left() + 10);
    r.setTop(rect().top() + 4);
    if (!isChecked()) {
        painter.fillRect(rect(),QColor(248,248,248));
        painter.setPen(QPen(QColor(65,77,104)));
        painter.drawText(r, m_text);
    } else {
        QRect bacrRect = rect();
        painter.setRenderHint(QPainter :: Antialiasing);
        QPainterPath path;
        path.addRoundedRect(bacrRect, 8, 8);
        painter.setPen(Qt::NoPen);
        painter.fillPath(path, QColor(0,129,255));
        painter.drawPath(path);
        painter.setPen(QPen(QColor(255,255,255)));
        painter.drawText(r, m_text);
    }
}*/
