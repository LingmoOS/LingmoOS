/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <QString>

namespace Globals
{
QString dirPath();
/**
 * Tries to find the specified file realtive to dirPath(). Also considers presets if there is no
 * existing file under dirPath() yet.
 * @returns The abosolute path to a matching file if on exists or an empty string
 */
QString findFile(const QString &filePath);
}
