// SPDX-FileCopyrightText: 2019, 2022 Mikhail Zolotukhin <zomial@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QColor>
#include <QString>

namespace CustomCssEditor
{

void addGtkModule(const QString &moduleName);

/**
 * @note Make sure "colorreload-gtk-module" is added to the module list before calling this function
 * @param colorsDefinitions a list of color definitions for GTK CSS
 * @see GtkConfig::setColors
 */
void setColors(const QMap<QString, QColor> &colorsDefinitions);

void setCustomClientSideDecorations(const QStringList &windowDecorationsButtonsImages);
void disableCustomClientSideDecorations();

}
