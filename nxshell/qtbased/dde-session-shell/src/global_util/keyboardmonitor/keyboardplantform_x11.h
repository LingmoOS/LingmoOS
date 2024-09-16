// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KEYBOARDPLANTFORM_X11_H
#define KEYBOARDPLANTFORM_X11_H

#include "keyboardplatform.h"

typedef struct _XDisplay Display;

class KeyboardPlantformX11 : public KeyBoardPlatform
{
    Q_OBJECT
public:
    KeyboardPlantformX11(QObject *parent = nullptr);

    bool isCapslockOn() override;
    bool isNumlockOn() override;
    bool setNumlockStatus(const bool &on) override;
    void run() override;

private:
    int listen(Display *display);
    static int xinput_version(Display *display);
    static void select_events(Display* display);
};

#endif // KEYBOARDPLANTFORM_X11_H
