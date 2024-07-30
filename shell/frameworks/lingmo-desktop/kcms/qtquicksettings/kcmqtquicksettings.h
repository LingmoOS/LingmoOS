/*  This file is part of the KDE's Lingmo desktop
    SPDX-FileCopyrightText: 2017 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KCModule>
#include <KConfigGroup>
#include <KSharedConfig>
#include <memory>

namespace Ui
{
class KCMQtQuickSettingsWidget;
}

namespace LingmoQtQuickSettings
{
class RendererSettings;
}

/**
 * @short A KCM to configure Lingmo QtQuick settings
 */
class KCMQtQuickSettingsModule : public KCModule
{
    Q_OBJECT

public:
    explicit KCMQtQuickSettingsModule(QObject *parent, const KPluginMetaData &data);
    ~KCMQtQuickSettingsModule() override;

private:
    std::unique_ptr<Ui::KCMQtQuickSettingsWidget> m_ui;
    std::unique_ptr<LingmoQtQuickSettings::RendererSettings> m_settings;
};
