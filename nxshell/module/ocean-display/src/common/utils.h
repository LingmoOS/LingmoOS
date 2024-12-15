// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H

#include <QSize>
#include <QString>

#include <kscreen/output.h>
#include <kscreen/types.h>

namespace Utils
{
QString outputName(const KScreen::Output *output, bool shouldShowSerialNumber = false, bool shouldShowConnector = false);
QString outputName(const KScreen::OutputPtr &output, bool shouldShowSerialNumber = false, bool shouldShowConnector = false);

QString sizeToString(const QSize &size);
}

#endif // UTILS_H
