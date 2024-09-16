// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KEYBOARDMONITOR_H
#define KEYBOARDMONITOR_H

#include <QThread>
#include "keyboardplantform_x11.h"
#include "keyboardplantform_wayland.h"

class KeyboardMonitor : public QThread
{
    Q_OBJECT
public:
    static KeyboardMonitor *instance();

    bool isCapslockOn();
    bool isNumlockOn();
    bool setNumlockStatus(const bool &on);

signals:
    void capslockStatusChanged(bool on);
    void numlockStatusChanged(bool on);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    KeyboardMonitor();
    KeyBoardPlatform* keyBoardPlatform = nullptr;
};

#endif // KEYBOARDMONITOR_H
