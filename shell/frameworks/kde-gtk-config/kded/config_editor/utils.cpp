// SPDX-FileCopyrightText: 2019, 2022 Mikhail Zolotukhin <zomial@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "utils.h"

#include <QStandardPaths>

namespace Utils
{

QString configDirPath(int gtkVersion)
{
    auto configLocation = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    return QStringLiteral("%1/gtk-%2.0").arg(configLocation).arg(gtkVersion);
};

QString readFileContents(QFile &file)
{
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return file.readAll();
    } else {
        return QString();
    }
}

}
