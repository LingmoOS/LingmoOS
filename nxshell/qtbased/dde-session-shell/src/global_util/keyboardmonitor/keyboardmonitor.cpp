// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "keyboardmonitor.h"
#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE

KeyboardMonitor::KeyboardMonitor() : QThread()
{
    if (DGuiApplicationHelper::isXWindowPlatform()) {
        keyBoardPlatform = new KeyboardPlantformX11();
    } else {
        keyBoardPlatform = new KeyboardPlantformWayland();
    }
    connect(keyBoardPlatform, &KeyBoardPlatform::capslockStatusChanged, this, &KeyboardMonitor::capslockStatusChanged);
    connect(keyBoardPlatform, &KeyBoardPlatform::numlockStatusChanged, this, &KeyboardMonitor::numlockStatusChanged);
}

KeyboardMonitor *KeyboardMonitor::instance()
{
    static KeyboardMonitor *KeyboardMonitorInstance = nullptr;

    if (!KeyboardMonitorInstance) {
        KeyboardMonitorInstance = new KeyboardMonitor;
    }

    return KeyboardMonitorInstance;
}

bool KeyboardMonitor::isCapslockOn()
{
    return keyBoardPlatform->isCapslockOn();
}

bool KeyboardMonitor::isNumlockOn()
{
    return keyBoardPlatform->isNumlockOn();
}

bool KeyboardMonitor::setNumlockStatus(const bool &on)
{
    return keyBoardPlatform->setNumlockStatus(on);
}

void KeyboardMonitor::run()
{
    keyBoardPlatform->run();
}
