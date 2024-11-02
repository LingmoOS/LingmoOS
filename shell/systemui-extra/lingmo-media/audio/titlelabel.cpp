/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include "titlelabel.h"
#include <QFont>

#include <QGSettings>
#include <QVariant>
#include <QDebug>

TitleLabel::TitleLabel(QWidget *parent):
    QLabel(parent)
{
    /*初始化字体*/
//    QFont font;
//    QGSettings *m_fontSetting = new QGSettings("org.lingmo.style");
//    font.setFamily(m_fontSetting->get("systemFont").toString());
//    font.setPixelSize(m_fontSetting->get("systemFontSize").toInt() * 18 / 11);  //设置的是pt，按照公式计算为px,标题默认字为18px
//    font.setWeight(QFont::Medium);
//    this->setFont(font);
        this ->setContentsMargins(16,0,0,0);
//    delete m_fontSetting;
//    m_fontSetting = nullptr;
}

TitleLabel::~TitleLabel()
{

}
