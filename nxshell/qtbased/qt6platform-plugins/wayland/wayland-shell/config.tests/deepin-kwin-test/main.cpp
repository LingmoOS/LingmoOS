// SPDX-FileCopyrightText: 2020 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef D_DEEPIN_IS_DWAYLAND
#include <KWayland/Client/plasmashell.h>
#else
#include <DWayland/Client/plasmashell.h>
#endif

int main()
{
    KWayland::Client::PlasmaShellSurface::Role::Override;
    return 0;
}
