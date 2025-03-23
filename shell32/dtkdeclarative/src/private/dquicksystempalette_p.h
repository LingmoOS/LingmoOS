// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKSYSTEMPALETTE_P_H
#define DQUICKSYSTEMPALETTE_P_H

#include "dquicksystempalette.h"

#include <QPointer>
#include <DPalette>
#include <DObjectPrivate>

DGUI_USE_NAMESPACE
DQUICK_BEGIN_NAMESPACE

class DQuickSystemPalettePrivate : public DTK_CORE_NAMESPACE::DObjectPrivate
{
public:
    DQuickSystemPalettePrivate(DQuickSystemPalette *qq);

    DPalette palette;
    QPalette::ColorGroup colorGroup = QPalette::Active;

private:
    D_DECLARE_PUBLIC(DQuickSystemPalette)
};

DQUICK_END_NAMESPACE

#endif // DQUICKSYSTEMPALETTE_P_H
