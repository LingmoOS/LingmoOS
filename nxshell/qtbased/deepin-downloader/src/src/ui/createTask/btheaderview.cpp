// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file btheaderview.cpp
 *
 * @brief bt表格头
 *
 * @date 2020-06-09 10:48
 *
 * Author: bulongwei  <bulongwei@uniontech.com>
 *
 * Maintainer: bulongwei  <bulongwei@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "btheaderview.h"
#include <QPainter>
#include <QStyleOptionButton>
#include <QLabel>

headerView::headerView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
{
    if(DGuiApplicationHelper::instance()->themeType() == 2) {
        onPalettetypechanged(DGuiApplicationHelper::ColorType::DarkType);
    } else {
        onPalettetypechanged(DGuiApplicationHelper::ColorType::LightType);
    }
    //set
}

void headerView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();

    QStyleOptionButton option;
    option.rect = checkBoxRect(rect);
    style()->drawControl(QStyle::CE_Header, &option, painter);

    // QPixmap pixmap;
    //  pixmap.load(":/icons/icon/headerIcon.png");
    //  style()->drawItemPixmap(painter, option.rect, logicalIndex, pixmap);
}

QRect headerView::checkBoxRect(const QRect &sourceRect) const
{
    QStyleOptionButton checkBoxStyleOption;
    QRect checkBoxRect = style()->subElementRect(QStyle::SE_CheckBoxIndicator,
                                                 &checkBoxStyleOption);
    m_curWidget += sourceRect.width();
    QPoint checkBoxPoint(m_curWidget - 20, 14);

    if (m_curWidget > 400) {
        m_curWidget = 0;
    }
    return QRect(checkBoxPoint, checkBoxRect.size());
}

void headerView::onPalettetypechanged(DGuiApplicationHelper::ColorType type)
{
    Q_UNUSED(type);
    QPalette p;


    //palette.setColor(DPalette::Background, c);

    if(DGuiApplicationHelper::instance()->themeType() == 2) {
        p.setColor(QPalette::Base, QColor(0,0,0,20));
        p.setBrush(QPalette::Base, DGuiApplicationHelper::instance()->applicationPalette().base());
        
       // QColor c = DGuiApplicationHelper::instance()->applicationPalette().base();
       // c.setAlpha(70);
       // p.setColor(DPalette::Background, c);

    } else {
       // p.setColor(QPalette::Base, QColor(0,0,0,20));
        QColor c = DGuiApplicationHelper::instance()->applicationPalette().base().color();
        c.setAlpha(70);
        p.setColor(DPalette::Background, c);
        //p.setColor(QPalette::Base, DGuiApplicationHelper::instance()->applicationPalette().base().color());
    }
    setPalette(p);
}
