// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file notificationssettiingwidget.cpp
 *
 * @brief 通知提醒
 *
 * @date 2020-06-09 10:52
 *
 * Author: zhaoyue  <zhaoyue@uniontech.com>
 *
 * Maintainer: zhaoyue  <zhaoyue@uniontech.com>
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

#include "notificationssettiingwidget.h"

#include <QPushButton>
#include <qlayout.h>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCall>

NotificationsSettiingWidget::NotificationsSettiingWidget(QWidget *parent)
    : QWidget(parent)
{
    QFont lableFont;
    //lableFont.setBold(QFont::DemiBold);
    lableFont.setFamily("T5");
    DLabel *pTextLabel = new DLabel(tr("Notify me when downloading finished or failed"), this);
    pTextLabel->setFont(lableFont);
    QFont tipFont;
    tipFont.setFamily("T6");
    DPalette palette;
    QColor fontColor("#6d7c88");
    palette.setColor(DPalette::Text, fontColor);
    DLabel *pTipLabel = new DLabel(tr("Change settings in \"Control Center - Notification - Downloader\""), this);
    pTipLabel->setFont(tipFont);
    pTipLabel->setPalette(palette);
    pTipLabel->setWindowOpacity(0.7);
    QFont btnFont;
    btnFont.setFamily("T6");
    QPushButton *btn = new QPushButton(tr("Settings"), this);
    btn->setFont(btnFont);
    btn->setMaximumWidth(150);
    QVBoxLayout *pVLayout = new QVBoxLayout();
    pVLayout->addWidget(pTextLabel);
    pVLayout->addSpacing(4);
    pVLayout->addWidget(pTipLabel);
    QHBoxLayout *pLayout = new QHBoxLayout(this);
    pLayout->addLayout(pVLayout);
    pLayout->addWidget(btn);
    connect(btn, &QPushButton::clicked, this, []() {
        QDBusInterface iface("com.deepin.dde.ControlCenter",
                             "/com/deepin/dde/ControlCenter",
                             "com.deepin.dde.ControlCenter",
                             QDBusConnection::sessionBus());
        iface.asyncCall("ShowPage", "notification", "下载器");
    });
}
