/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2024 zhang yu <zhangyud@uniontech.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include "window.h"

namespace KWin {

class DebugPixmap : public QObject
{
    Q_OBJECT
public:
    DebugPixmap(/* args */){};
    ~DebugPixmap(){};

    void saveImageFromTexture(xcb_window_t winid, Window *w);
    void saveImageFromPixmap(xcb_window_t winid, Window *w);
    void saveImageFromXorg(xcb_window_t winid);
    void saveCompositePixmap();

private:
    /* data */
};

}


