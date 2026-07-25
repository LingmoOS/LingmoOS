/*
 * SPDX-FileCopyrightText: 2022 Aleix Pol i Gonzalez <aleixpol@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include <QKeySequence>
#include <QString>
#include <optional>

/**
 * Specifies a shortcut as defined in the xdg-specs:
 *
 * https://gitlab.freedesktop.org/xdg/xdg-specs/-/tree/master/shortcuts
 */

namespace XdgShortcut
{
std::optional<QKeySequence> parse(const QString &shortcutString);
}
