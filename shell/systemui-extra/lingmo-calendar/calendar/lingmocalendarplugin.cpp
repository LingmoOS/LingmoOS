/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "lingmocalendarplugin.h"

LINGMOCalendarPlugin::LINGMOCalendarPlugin(const ILINGMOPanelPluginStartupInfo &startupInfo):
    QObject(),
    ILINGMOPanelPlugin(startupInfo),
    m_widget(new LINGMOCalendarWidget(this))
{
    m_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

LINGMOCalendarPlugin::~LINGMOCalendarPlugin()
{
    delete m_widget;
}

QWidget *LINGMOCalendarPlugin::widget()
{
    return m_widget;
}

void LINGMOCalendarPlugin::realign()
{
    m_widget->realign();
}
