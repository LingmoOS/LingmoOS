// SPDX-FileCopyrightText: 2019, 2022 Mikhail Zolotukhin <zomial@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QString>
#include <QVariant>

namespace XSettingsEditor
{
void setValue(const QString &paramName, const QVariant &paramValue);
void unsetValue(const QString &paramName);
}
