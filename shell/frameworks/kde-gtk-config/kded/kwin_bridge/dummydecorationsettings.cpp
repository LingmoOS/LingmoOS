/*
 * SPDX-FileCopyrightText: 2020 Mikhail Zolotukhin <zomial@protonmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#include "dummydecorationsettings.h"

namespace KDecoration2
{
DummyDecorationSettings::DummyDecorationSettings(DecorationSettings *parent)
    : DecorationSettingsPrivate(parent)
{
}

bool DummyDecorationSettings::isOnAllDesktopsAvailable() const
{
    return false;
}

bool DummyDecorationSettings::isAlphaChannelSupported() const
{
    return true;
}

bool DummyDecorationSettings::isCloseOnDoubleClickOnMenu() const
{
    return false;
}

QList<DecorationButtonType> DummyDecorationSettings::decorationButtonsLeft() const
{
    return {};
}

QList<DecorationButtonType> DummyDecorationSettings::decorationButtonsRight() const
{
    return {};
}

BorderSize DummyDecorationSettings::borderSize() const
{
    return BorderSize::None;
}

}
