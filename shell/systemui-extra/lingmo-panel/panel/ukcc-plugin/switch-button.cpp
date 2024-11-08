/*
 *
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "switch-button.h"

SwitchButton::SwitchButton(QWidget *parent, const QString &name) : QFrame(parent)
{
    this->setFixedHeight(56);
    m_Layout = new QHBoxLayout(this);
    m_Layout->setContentsMargins(16, 0, 16, 0);
    m_Label = new QLabel(this);
    m_Label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_Label->setText(name);
    m_button = new kdk::KSwitchButton(this);
    m_button->setFixedSize(48,24);
    m_button->setEnabled(false);
    m_Layout->addWidget(m_Label);
    m_Layout->addWidget(m_button);
    connect(m_button, &kdk::KSwitchButton::stateChanged, this, &SwitchButton::stateChanged);
}

void SwitchButton::setChecked(bool checked)
{
    m_button->setChecked(checked);
}

void SwitchButton::setEnabled(bool enable)
{
    m_button->setEnabled(enable);
}
