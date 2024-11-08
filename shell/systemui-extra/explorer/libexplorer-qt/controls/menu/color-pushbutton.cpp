/*
 * Peony-Qt
 *
 * Copyright (C) 2023, KylinSoft Information Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */


#include "color-pushbutton.h"
#include <QPainter>
#include <QDebug>

#include <QPainterPath>

#define BIG_LABEL_WIDTH       20
#define SMALL_LABEL_WIDTH     14

ColorPushButton::ColorPushButton(QColor color,QWidget *parent) : QPushButton(parent)
{
    m_color = color;
    setProperty("isRoundButton",true);
    setFixedSize(BIG_LABEL_WIDTH, BIG_LABEL_WIDTH);
    QRectF rect = this->rect();
    m_bigpathrect = rect.adjusted(1, 1, -1, -1);
    m_pathrect = rect.adjusted(4, 4, -4, -4);
    m_pathrectwidth = m_pathrect.width();
    m_pathrectheigth = m_pathrect.height();
    m_bigpathrectwidth = m_bigpathrect.width();
    m_bigpathrectheigth = m_bigpathrect.height();

}

void ColorPushButton::paintEvent(QPaintEvent *e)
{
    m_pathrect = this->rect().adjusted(1, 1, -1, -1);
    m_bigpathrect = m_pathrect;
    QString manager = tr("label management ...");

    if(this->underMouse()){
        if(this->isChecked() && m_checkInMultiSelect){
            QPalette pal = this->palette();
            pal.setColor(QPalette::Active,QPalette::Highlight, m_color);
            setFixedSize(BIG_LABEL_WIDTH, BIG_LABEL_WIDTH);
            this->setPalette(pal);
            QPushButton::paintEvent(e);

            QPainter painter(this);
            painter.save();
            painter.setPen(Qt::white);
            QPainterPath path;
            path.moveTo(m_bigpathrect.left() + m_bigpathrectwidth / 4, m_bigpathrect.top() + m_bigpathrectheigth / 4);
            path.lineTo(m_bigpathrect.left() + m_bigpathrectwidth * 0.75, m_bigpathrect.top() + m_bigpathrectheigth *0.75);
            path.moveTo(m_bigpathrect.left() + m_bigpathrectwidth / 4, m_bigpathrect.top() + m_bigpathrectheigth *0.75);
            path.lineTo(m_bigpathrect.left() + m_bigpathrectwidth * 0.75, m_bigpathrect.top() + m_bigpathrectheigth / 4);
            painter.drawPath(path);
            painter.restore();
            manager = QString(tr("Remove \"%1\"")).arg(m_name);
            Q_EMIT changeText(manager);
        }else{
            QPalette pal = this->palette();
            pal.setColor(QPalette::Active,QPalette::Highlight, m_color);
            this->setPalette(pal);
            setFixedSize(BIG_LABEL_WIDTH, BIG_LABEL_WIDTH);
            QPushButton::paintEvent(e);

            QPainter painter(this);
            painter.save();

            painter.setPen(Qt::white);
            QPainterPath path;
            path.moveTo(m_bigpathrect.left() + m_bigpathrectwidth / 4, m_bigpathrect.top() + m_bigpathrectheigth / 2);
            path.lineTo(m_bigpathrect.left() + m_bigpathrectwidth * 0.80, m_bigpathrect.top() + m_bigpathrectheigth / 2);
            path.moveTo(m_bigpathrect.left() + m_bigpathrectwidth / 2, m_bigpathrect.top() + m_bigpathrectheigth / 4);
            path.lineTo(m_bigpathrect.left() + m_bigpathrectwidth / 2, m_bigpathrect.top() + m_bigpathrectheigth * 0.80);
            painter.drawPath(path);
            painter.restore();
            manager = QString(tr("add  \"%1\"")).arg(m_name);
            Q_EMIT changeText(manager);
        }
    }else{
        if(this->isChecked()){
            QPalette pal = this->palette();
            pal.setColor(QPalette::Active,QPalette::Highlight, m_color);
            this->setPalette(pal);
            setFixedSize(SMALL_LABEL_WIDTH, SMALL_LABEL_WIDTH);
            QPushButton::paintEvent(e);

            QPainter painter(this);
            painter.save();

            painter.setPen(Qt::white);
            QPainterPath path;
            path.moveTo(m_pathrect.left() + m_pathrectwidth / 4, m_pathrect.top() + m_pathrectheigth / 2);
            path.lineTo(m_pathrect.left() + m_pathrectwidth * 0.45, m_pathrect.bottom() - m_pathrectheigth / 3);
            path.lineTo(m_pathrect.right() - m_pathrectwidth / 4, m_pathrect.top() + m_pathrectheigth / 3);
            painter.drawPath(path);
            painter.restore();
        }else {
            QPalette pal = this->palette();
            pal.setColor(QPalette::Button, m_color);
            this->setPalette(pal);
            setFixedSize(SMALL_LABEL_WIDTH, SMALL_LABEL_WIDTH);
            QPushButton::paintEvent(e);
            }
    }

}

void ColorPushButton::setName(const QString& name)
{
    m_name = name;
}
