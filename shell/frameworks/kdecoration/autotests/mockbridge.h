/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#pragma once

#include "../src/private/decorationbridge.h"
#include <QObject>

class MockClient;
class MockSettings;

class MockBridge : public KDecoration2::DecorationBridge
{
    Q_OBJECT
public:
    std::unique_ptr<KDecoration2::DecoratedClientPrivate> createClient(KDecoration2::DecoratedClient *client, KDecoration2::Decoration *decoration) override;
    std::unique_ptr<KDecoration2::DecorationSettingsPrivate> settings(KDecoration2::DecorationSettings *parent) override;

    MockClient *lastCreatedClient() const
    {
        return m_lastCreatedClient;
    }
    MockSettings *lastCreatedSettings() const
    {
        return m_lastCreatedSettings;
    }

private:
    MockClient *m_lastCreatedClient = nullptr;
    MockSettings *m_lastCreatedSettings = nullptr;
};
