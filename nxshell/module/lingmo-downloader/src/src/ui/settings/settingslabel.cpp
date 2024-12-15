// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file settingslabel.cpp
 *
 * @brief 设置页面文本显示类
 *
 *@date 2020-06-30 10:10
 *
 * Author: yuandandan  <yuandandan@uniontech.com>
 *
 * Maintainer: yuandandan  <yuandandan@uniontech.com>
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
#include "settingslabel.h"

#include <QHBoxLayout>

SettingsLabel::SettingsLabel(QWidget *parent)
    : QWidget(parent)
{
    m_label = new DLabel;

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_label);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(mainLayout);
}

void SettingsLabel::setLabelText(const QString &text)
{
    m_label->setText(text);
}

void SettingsLabel::setLabelFont(QFont font)
{
    m_label->setFont(font);
}

void SettingsLabel::setLabelPalette(DPalette palette)
{
    m_label->setPalette(palette);
}
