// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KEYBOARDPLANTFORM_WAYLAND_H
#define KEYBOARDPLANTFORM_WAYLAND_H

#include "keyboardplatform.h"

class KeyboardPlantformWayland : public KeyBoardPlatform
{
    Q_OBJECT

public:
    KeyboardPlantformWayland(QObject *parent = nullptr);

    bool isCapslockOn() override;
    bool isNumlockOn() override;
    bool setNumlockStatus(const bool &on) override;

signals:
    void capslockStatusChanged(bool on);
    void numlockStatusChanged(bool on);

protected:
    void run() Q_DECL_OVERRIDE;
};

#endif // KEYBOARDPLANTFORM_WAYLAND_H
