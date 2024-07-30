/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "ssh.h"
#include "sshauth.h"
#include "sshwidget.h"

#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(SshUiPlugin, "plasmanetworkmanagement_sshui.json")

SshUiPlugin::SshUiPlugin(QObject *parent, const QVariantList &)
    : VpnUiPlugin(parent)
{
}

SshUiPlugin::~SshUiPlugin() = default;

SettingWidget *SshUiPlugin::widget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent)
{
    return new SshSettingWidget(setting, parent);
}

SettingWidget *SshUiPlugin::askUser(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent)
{
    return new SshAuthWidget(setting, hints, parent);
}

QString SshUiPlugin::suggestedFileName(const NetworkManager::ConnectionSettings::Ptr &connection) const
{
    Q_UNUSED(connection);

    // TODO : implement suggested file name
    return {};
}

#include "ssh.moc"

#include "moc_ssh.cpp"
