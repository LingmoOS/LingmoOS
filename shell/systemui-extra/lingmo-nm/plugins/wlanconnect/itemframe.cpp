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
ItemFrame::ItemFrame(QString devName, QWidget *parent)
{
    deviceLanLayout = new QVBoxLayout(this);
    deviceLanLayout->setContentsMargins(MAIN_LAYOUT_MARGINS);
    lanItemFrame = new QFrame(this);
    lanItemFrame->setFrameShape(QFrame::Shape::NoFrame);
    lanItemFrame->setContentsMargins(LAYOUT_MARGINS);

    lanItemLayout = new QVBoxLayout(this);
    lanItemLayout->setContentsMargins(LAYOUT_MARGINS);
    lanItemLayout->setSpacing(1);
    addWlanWidget = new AddNetBtn(true, this);

    deviceLanLayout->setSpacing(1);
    setLayout(deviceLanLayout);
    lanItemFrame->setLayout(lanItemLayout);

    deviceFrame = new DeviceFrame(devName, this);
    deviceLanLayout->addWidget(deviceFrame);
    deviceLanLayout->addWidget(lanItemFrame);
    deviceLanLayout->addWidget(addWlanWidget);

    //下拉按钮
    connect(deviceFrame->dropDownLabel, &DrownLabel::labelClicked, this, &ItemFrame::onDrownLabelClicked);
}

ItemFrame::~ItemFrame()
{

}

void ItemFrame::onDrownLabelClicked()
{
    if (!deviceFrame->dropDownLabel->isChecked) {
        lanItemFrame->show();
        deviceFrame->dropDownLabel->setDropDownStatus(true);
    } else {
        lanItemFrame->hide();
        deviceFrame->dropDownLabel->setDropDownStatus(false);
    }
}

void ItemFrame::filletStyleChange()
{
    if (lanItemLayout->isEmpty()) {
        return;
    }

    for (int i = 0; i < lanItemLayout->count(); ++i) {
        QLayoutItem *it = lanItemLayout->itemAt(i);
        WlanItem *itemFrame = (WlanItem*)(it->widget());
        if (i != lanItemLayout->count()-1) {
            itemFrame->setHalfFillet(false);
        } else {
            itemFrame->setHalfFillet(true);
        }
    }
}
