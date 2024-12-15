/*
    SPDX-FileCopyrightText: 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "waylandshellintegration.h"

namespace KWaylandServer
{
class XdgToplevelV6Interface;
class XdgPopupV6Interface;
}

namespace KWin
{

class XdgShellV6Integration : public WaylandShellIntegration
{
    Q_OBJECT

public:
    explicit XdgShellV6Integration(QObject *parent = nullptr);

private:
    void registerXdgToplevelV6(KWaylandServer::XdgToplevelV6Interface *toplevel);
    void registerXdgPopupV6(KWaylandServer::XdgPopupV6Interface *popup);
    void createXdgToplevelV6Window(KWaylandServer::XdgToplevelV6Interface *surface);
};

} // namespace KWin
