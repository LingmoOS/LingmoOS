// SPDX-FileCopyrightText: 2019, 2022 Mikhail Zolotukhin <zomial@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QVariant>

namespace GSettingsEditor
{
void setValue(const char *paramName, const QVariant &paramValue, const char *category = "org.gnome.desktop.interface");
void setValueAsEnum(const char *paramName, int paramValue, const char *category = "org.gnome.desktop.interface");
}
