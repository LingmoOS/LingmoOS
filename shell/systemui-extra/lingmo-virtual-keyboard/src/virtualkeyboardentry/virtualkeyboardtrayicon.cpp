/*
 * Copyright 2022 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "virtualkeyboardentry/virtualkeyboardtrayicon.h"

#include <QIcon>

VirtualKeyboardTrayIcon::VirtualKeyboardTrayIcon(
    VirtualKeyboardManager &virtualKeyboardManager,
    const FcitxVirtualKeyboardService &fcitxVirtualKeyboardService)
    : virtualKeyboardManager_(virtualKeyboardManager),
      fcitxVirtualKeyboardService_(fcitxVirtualKeyboardService) {
    initTrayIcon();
}

void VirtualKeyboardTrayIcon::setContextMenu(QMenu *contextMenu) {
    trayIcon_->setContextMenu(contextMenu);
}

void VirtualKeyboardTrayIcon::hideContextMenu() {
    trayIcon_->contextMenu()->hide();
}

void VirtualKeyboardTrayIcon::initTrayIcon() {
    trayIcon_ = new QSystemTrayIcon(this);
    trayIcon_->setIcon(QIcon::fromTheme("lingmo-virtual-keyboard-symbolic"));
    trayIcon_->setToolTip(tr("lingmo-virtual-keyboard"));
    connect(trayIcon_, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(onTrayIconActivated(QSystemTrayIcon::ActivationReason)));
    trayIcon_->setVisible(true);
}

void VirtualKeyboardTrayIcon::toggleVirtualKeyboard() {
    if (virtualKeyboardManager_.isVirtualKeyboardVisible()) {
        fcitxVirtualKeyboardService_.hideVirtualKeyboard();
    } else {
        fcitxVirtualKeyboardService_.showVirtualKeyboard();
    }
}

void VirtualKeyboardTrayIcon::onTrayIconActivated(
    QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
    case QSystemTrayIcon::Trigger: {
        toggleVirtualKeyboard();
        break;
    };
    default:
        break;
    }
}
