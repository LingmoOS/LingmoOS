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

#include "dbusservice.h"
#include "virtualkeyboard/virtualkeyboardmanager.h"

DBusService::DBusService(VirtualKeyboardManager *virtualKeyboardManager)
    : virtualKeyboardManager_(virtualKeyboardManager),
      virtualKeyboardVisibilityRequestMerger_(
          VIRTUAL_KEYBOARD_VISIBILITY_PEROID) {
    initRequestMerger();

    startService();
}

DBusService::~DBusService() { stopService(); }

void DBusService::initRequestMerger() {
    virtualKeyboardVisibilityRequestMerger_.init(
        [this]() { virtualKeyboardManager_->showVirtualKeyboard(); },
        [this]() { virtualKeyboardManager_->hideVirtualKeyboard(); },
        [this]() {
            return !virtualKeyboardManager_->isVirtualKeyboardVisible();
        },
        [this]() {
            return virtualKeyboardManager_->isVirtualKeyboardVisible();
        });
}

bool DBusService::startService() {
    return QDBusConnection::sessionBus().registerService(serviceName_) &&
           QDBusConnection::sessionBus().registerObject(
               servicePath_, serviceInterface_, this,
               QDBusConnection::ExportAllSlots);
}

bool DBusService::stopService() {
    QDBusConnection::sessionBus().unregisterObject(servicePath_);
    return QDBusConnection::sessionBus().unregisterService(serviceName_);
}

void DBusService::ShowVirtualKeyboard() {
    virtualKeyboardVisibilityRequestMerger_.activate();
}

void DBusService::HideVirtualKeyboard() {
    virtualKeyboardVisibilityRequestMerger_.deactivate();
}

bool DBusService::IsVirtualKeyboardVisible() {
    return virtualKeyboardManager_->isVirtualKeyboardVisible();
}

void DBusService::UpdatePreeditCaret(int preeditCursor) {
    virtualKeyboardManager_->updatePreeditCaret(preeditCursor);
}

void DBusService::UpdatePreeditArea(const QString &preeditText) {
    virtualKeyboardManager_->updatePreeditArea(preeditText);
}

void DBusService::UpdateCandidateArea(const QStringList &candidateTextList,
                                      bool hasPrev, bool hasNext, int pageIndex,
                                      int globalCursorIndex /* = -1*/) {
    virtualKeyboardManager_->updateCandidateArea(
        candidateTextList, hasPrev, hasNext, pageIndex, globalCursorIndex);
}

void DBusService::NotifyIMActivated(const QString &uniqueName) {
    virtualKeyboardManager_->notifyIMActivated(uniqueName);
}

void DBusService::NotifyIMDeactivated(const QString &uniqueName) {
    virtualKeyboardManager_->notifyIMDeactivated(uniqueName);
}

void DBusService::NotifyIMListChanged() {
    virtualKeyboardManager_->notifyIMListChanged();
}
