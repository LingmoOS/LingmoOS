// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef APPLICATIONCHECKER_H
#define APPLICATIONCHECKER_H

#include "desktopentry.h"
#include <memory>

namespace ApplicationFilter {

bool hiddenCheck(const std::unique_ptr<DesktopEntry> &entry) noexcept;
bool tryExecCheck(const std::unique_ptr<DesktopEntry> &entry) noexcept;
bool showInCheck(const std::unique_ptr<DesktopEntry> &entry) noexcept;
bool hiddenCheck(const DesktopEntry &entry) noexcept;
bool tryExecCheck(const DesktopEntry &entry) noexcept;
bool showInCheck(const DesktopEntry &entry) noexcept;

}  // namespace ApplicationFilter
#endif
