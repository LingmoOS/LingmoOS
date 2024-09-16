// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMON_GLOBALS_H
#define COMMON_GLOBALS_H

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

#endif // COMMON_GLOBALS_H
