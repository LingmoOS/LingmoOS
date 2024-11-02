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
#include "itemframe.h"
#include <QPainter>

#define LAYOUT_MARGINS 0,0,0,0
#define MAIN_LAYOUT_MARGINS 0,0,0,0
ItemFrame::ItemFrame(QWidget *parent)
    :QFrame(parent)
{
    m_mainVLayout = new QVBoxLayout(this);
    m_mainVLayout->setContentsMargins(MAIN_LAYOUT_MARGINS);
    m_vpnFrame = new QFrame(this);
    m_vpnFrame->setFrameShape(QFrame::Shape::NoFrame);
    m_vpnFrame->setContentsMargins(LAYOUT_MARGINS);

    m_vpnVLayout = new QVBoxLayout(this);
    m_vpnVLayout->setContentsMargins(LAYOUT_MARGINS);
    m_vpnVLayout->setSpacing(1);
    m_addVpnWidget = new AddNetBtn(false, this);
    m_addVpnWidget->setTextLabel(tr("Add VPN"));

    m_mainVLayout->setSpacing(1);
    setLayout(m_mainVLayout);
    m_vpnFrame->setLayout(m_vpnVLayout);

    m_mainVLayout->addWidget(m_vpnFrame);
    m_mainVLayout->addWidget(m_addVpnWidget);
}

void ItemFrame::filletStyleChange()
{
    if (m_vpnVLayout->isEmpty()) {
        return;
    }

    for (int i = 0; i < m_vpnVLayout->count(); ++i) {
        QLayoutItem *it = m_vpnVLayout->itemAt(i);
        VpnItem *itemFrame = (VpnItem*)(it->widget());
        if (i != m_vpnVLayout->count()-1) {
            itemFrame->setHalfFillet(false);
        } else {
            itemFrame->setHalfFillet(true);
        }
    }
}
