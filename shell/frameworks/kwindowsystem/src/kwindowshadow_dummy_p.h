/*
    SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KWINDOWSHADOW_DUMMY_P_H
#define KWINDOWSHADOW_DUMMY_P_H

#include "kwindowshadow_p.h"

class KWindowShadowTilePrivateDummy final : public KWindowShadowTilePrivate
{
public:
    bool create() override;
    void destroy() override;
};

class KWindowShadowPrivateDummy final : public KWindowShadowPrivate
{
public:
    bool create() override;
    void destroy() override;
};

#endif // KWINDOWSHADOW_DUMMY_P_H
