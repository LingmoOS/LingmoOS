/*
    SPDX-FileCopyrightText: 2019 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "colorschemes.h"

#include <KColorSchemeManager>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

using namespace CuttleFish;

ColorSchemes::ColorSchemes(QObject *parent)
    : QObject(parent)
    , m_colorManager(new KColorSchemeManager(this))
{
}

QAbstractItemModel *CuttleFish::ColorSchemes::colorSchemes()
{
    return m_colorManager->model();
}

void ColorSchemes::activateColorScheme(const QString &name)
{
    m_colorManager->activateScheme(m_colorManager->indexForScheme(name));
}

#include "moc_colorschemes.cpp"
