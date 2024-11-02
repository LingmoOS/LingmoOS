/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "customtabstyle.h"


CustomTabStyle::CustomTabStyle()
{

}

QSize CustomTabStyle::sizeFromContents(QStyle::ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const
{
    QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
    if (type == QStyle::CT_TabBarTab) {
        s.transpose();
        s.rwidth() = 60; // 设置每个tabBar中item的大小
        s.rheight() = 36;
    }
    return s;
}

void CustomTabStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == CE_TabBarTab) {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            QRect allRect = tab->rect;
            if (tab->state & QStyle::State_Selected) {  //选中状态：tab的Qlabel为矩形，底色为白色，边框淡灰色，文字为淡蓝色且加粗居中（具体颜色值由拾色器提取）
                painter->save();
                painter->setPen(0x3790FA);  //设置画笔颜色为淡灰色
                painter->setBrush(QBrush(0x3790FA));  //设置画刷为白色
                painter->drawRoundedRect(allRect.adjusted(0,0,0,0),6,6);  //重绘tab的矩形边框
                painter->restore();  //还原为默认

                painter->save();
                painter->setPen(0xffffff);  //重新设置画笔颜色为淡蓝色
                QTextOption option;
                option.setAlignment(Qt::AlignCenter);  //设置文字居中
                painter->drawText(allRect, tab->text, option);  //重绘文字
                painter->restore();
            }
            else if(tab->state & QStyle::State_MouseOver) { //hover状态：tab的Qlabel为矩形，底色为灰色，边框仍未淡灰色，文字加粗居中
                painter->save();
                painter->setPen(tab->palette.color(QPalette::Window));  //设置画笔颜色为淡灰色
                painter->setBrush(tab->palette.color(QPalette::Window));  //设置画刷为灰色
                painter->drawRoundedRect(allRect.adjusted(0,0,0,0),6,6);  //重绘tab的矩形边框
                painter->restore();  //还原

                painter->save();
                QTextOption option;
                option.setAlignment(Qt::AlignCenter);  //设置文字居中
                painter->drawText(allRect, tab->text, option);  //重绘文字
                painter->restore();
            }
            else //其它的：tab的Qlabel为矩形，底色为灰色，边框为淡灰色不变，文字不加粗但居中
            {
                painter->save();
                painter->setPen(tab->palette.color(QPalette::Button));
                painter->setBrush(tab->palette.color(QPalette::Button));
                painter->drawRoundedRect(allRect.adjusted(0,0,0,0),6,6);  //重绘tab的矩形边框
                painter->restore();
                painter->save();
                QTextOption option;
                option.setAlignment(Qt::AlignCenter);
                painter->drawText(allRect, tab->text, option);
                painter->restore();
            }
            return;
        }
    }

    if (element == CE_TabBarTab) {
        QProxyStyle::drawControl(element, option, painter, widget);
    }
}

