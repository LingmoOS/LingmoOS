/*
 * SPDX-FileCopyrightText: 2020 Mikhail Zolotukhin <zomial@protonmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <KDecoration2/DecorationSettings>
#include <KDecoration2/Private/DecorationSettingsPrivate>

namespace KDecoration2
{
class DummyDecorationSettings : public KDecoration2::DecorationSettingsPrivate
{
public:
    explicit DummyDecorationSettings(DecorationSettings *parent);

    virtual bool isOnAllDesktopsAvailable() const override;
    virtual bool isAlphaChannelSupported() const override;
    virtual bool isCloseOnDoubleClickOnMenu() const override;
    virtual QList<DecorationButtonType> decorationButtonsLeft() const override;
    virtual QList<DecorationButtonType> decorationButtonsRight() const override;
    virtual BorderSize borderSize() const override;
};

}
