/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2024 zhang yu <zhangyud@uniontech.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include <xcb/xcb.h>
#include <QObject>

namespace KWin
{
class Utils : public QObject
{
public:
    static xcb_atom_t internAtom(const char *name, bool only_if_exists = true);
    static QByteArray readWindowProperty(xcb_window_t win, xcb_atom_t atom, xcb_atom_t type);
};
}