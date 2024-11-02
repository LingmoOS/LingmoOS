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

#include "fcitxvirtualkeyboardserviceproxy.h"

// static
const QString FcitxVirtualKeyboardServiceProxy::serviceName_ =
    "org.fcitx.Fcitx5";
// static
const QString FcitxVirtualKeyboardServiceProxy::servicePath_ =
    "/virtualkeyboard";
// static
const QString FcitxVirtualKeyboardServiceProxy::serviceInterface_ =
    "org.fcitx.Fcitx.VirtualKeyboard1";

FcitxVirtualKeyboardServiceProxy::FcitxVirtualKeyboardServiceProxy() {
    virtualKeyboardService.reset(
        new QDBusInterface(serviceName_, servicePath_, serviceInterface_,
                           QDBusConnection::sessionBus()));
}

void FcitxVirtualKeyboardServiceProxy::showVirtualKeyboard() const {
    virtualKeyboardService->call("ShowVirtualKeyboard");
}

void FcitxVirtualKeyboardServiceProxy::hideVirtualKeyboard() const {
    virtualKeyboardService->call("HideVirtualKeyboard");
}
