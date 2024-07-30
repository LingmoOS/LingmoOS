/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#pragma once

#include "../src/private/decorationsettingsprivate.h"

class MockSettings : public KDecoration2::DecorationSettingsPrivate
{
public:
    explicit MockSettings(KDecoration2::DecorationSettings *parent);

    KDecoration2::BorderSize borderSize() const override;
    QList<KDecoration2::DecorationButtonType> decorationButtonsLeft() const override;
    QList<KDecoration2::DecorationButtonType> decorationButtonsRight() const override;
    bool isAlphaChannelSupported() const override;
    bool isCloseOnDoubleClickOnMenu() const override;
    bool isOnAllDesktopsAvailable() const override;

    void setOnAllDesktopsAvailabe(bool set);
    void setCloseOnDoubleClickOnMenu(bool set);

private:
    bool m_onAllDesktopsAvailable = false;
    bool m_closeDoubleClickOnMenu = false;
};
