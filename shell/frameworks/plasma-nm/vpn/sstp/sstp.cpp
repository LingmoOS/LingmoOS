/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "sstp.h"
#include "sstpauth.h"
#include "sstpwidget.h"

#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(SstpUiPlugin, "plasmanetworkmanagement_sstpui.json")

SstpUiPlugin::SstpUiPlugin(QObject *parent, const QVariantList &)
    : VpnUiPlugin(parent)
{
}

SstpUiPlugin::~SstpUiPlugin() = default;

SettingWidget *SstpUiPlugin::widget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent)
{
    return new SstpSettingWidget(setting, parent);
}

SettingWidget *SstpUiPlugin::askUser(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent)
{
    return new SstpAuthWidget(setting, hints, parent);
}

QString SstpUiPlugin::suggestedFileName(const NetworkManager::ConnectionSettings::Ptr &connection) const
{
    Q_UNUSED(connection);

    // TODO : implement suggested file name
    return {};
}

#include "sstp.moc"

#include "moc_sstp.cpp"
