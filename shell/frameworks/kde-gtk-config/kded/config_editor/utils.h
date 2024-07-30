// SPDX-FileCopyrightText: 2019, 2022 Mikhail Zolotukhin <zomial@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QFile>
#include <QString>

namespace Utils
{
QString configDirPath(int gtkVersion);
QString readFileContents(QFile &file);

[[maybe_unused]] static inline const auto s_gtkVersions = {3, 4};
}
