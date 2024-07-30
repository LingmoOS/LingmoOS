/*
    SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "iodine.h"
#include "iodineauth.h"
#include "iodinewidget.h"

#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(IodineUiPlugin, "plasmanetworkmanagement_iodineui.json")

IodineUiPlugin::IodineUiPlugin(QObject *parent, const QVariantList &)
    : VpnUiPlugin(parent)
{
}

IodineUiPlugin::~IodineUiPlugin() = default;

SettingWidget *IodineUiPlugin::widget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent)
{
    return new IodineWidget(setting, parent);
}

SettingWidget *IodineUiPlugin::askUser(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent)
{
    return new IodineAuthDialog(setting, hints, parent);
}

QString IodineUiPlugin::suggestedFileName(const NetworkManager::ConnectionSettings::Ptr &connection) const
{
    Q_UNUSED(connection);
    return {};
}

#include "iodine.moc"

#include "moc_iodine.cpp"
