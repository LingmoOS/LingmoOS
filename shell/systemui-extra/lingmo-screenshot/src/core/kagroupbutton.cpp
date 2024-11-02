/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kagroupbutton.h"

#include <QPainter>
#include <QPainterPath>
#include <QStylePainter>
#include <QDebug>

KAGroupButton::KAGroupButton(QWidget *parent) :
    QPushButton(parent)
{
    mPosition = KAButtonStyleOption::OnlyOne;
    this->setAttribute(Qt::WA_TranslucentBackground);
}

KAGroupButton::~KAGroupButton()
{
}

void KAGroupButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStylePainter p(this);
    KAButtonStyleOption option;
    initStyleOption(&option);
    option.position = mPosition;
    option.katype = KAButtonStyleOption::BT_GROUP;
    p.drawControl(QStyle::CE_PushButton, option);
}
