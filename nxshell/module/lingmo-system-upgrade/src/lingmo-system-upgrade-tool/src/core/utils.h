// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE

inline bool isDarkMode()
{
    return DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;
}
