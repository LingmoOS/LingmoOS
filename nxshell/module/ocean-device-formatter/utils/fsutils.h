// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FSUTILS_H
#define FSUTILS_H

#include <QString>
#include <QStringList>

namespace FsUtils
{
    int maxLabelLength(const QString &fs);
    QStringList supportedFilesystems();
};

#endif // FSUTILS_H
