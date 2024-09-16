// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "keyboardplantform_wayland.h"

#include <stdio.h>
#include <stdlib.h>

KeyboardPlantformWayland::KeyboardPlantformWayland(QObject *parent)
    : KeyBoardPlatform(parent)
{

}

bool KeyboardPlantformWayland::isCapslockOn()
{
    return false;
}

bool KeyboardPlantformWayland::isNumlockOn()
{
    return false;
}

bool KeyboardPlantformWayland::setNumlockStatus(const bool &on)
{
    Q_UNUSED(on);
    return false;
}

void KeyboardPlantformWayland::run()
{
}
