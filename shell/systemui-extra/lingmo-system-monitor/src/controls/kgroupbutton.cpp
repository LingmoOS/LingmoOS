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

#include "kgroupbutton.h"

#include <QStylePainter>
#include <QDebug>
#include <QStyle>

KGroupButton::KGroupButton(QWidget *parent)
    : QPushButton(parent)
{
    mPosition = GroupButtonStyleOption::OnlyOne;
    this->setProperty("useButtonPalette", true);
    connect(this,&KGroupButton::toggled,this,[=](bool isChecked){
        if (isChecked) {
            this->setProperty("useButtonPalette", false);
        } else {
            this->setProperty("useButtonPalette", true);
        }
    });
}

KGroupButton::KGroupButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
    mPosition = GroupButtonStyleOption::OnlyOne;
}

KGroupButton::KGroupButton(const QIcon& icon, const QString &text, QWidget *parent)
    : QPushButton(icon, text, parent)
{
    mPosition = GroupButtonStyleOption::OnlyOne;
}

void KGroupButton::paintEvent(QPaintEvent *paintEvent)
{
    Q_UNUSED(paintEvent);
    QStylePainter p(this);
    GroupButtonStyleOption option;
    option.position = mPosition;
    initStyleOption(&option);
    p.drawControl(QStyle::CE_PushButton, option);
}
