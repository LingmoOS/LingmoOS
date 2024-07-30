/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <KPluginFactory>

#include <NetworkManagerQt/VpnSetting>

#include "l2tp.h"
#include "l2tpauth.h"
#include "l2tpwidget.h"

K_PLUGIN_CLASS_WITH_JSON(L2tpUiPlugin, "plasmanetworkmanagement_l2tpui.json")

L2tpUiPlugin::L2tpUiPlugin(QObject *parent, const QVariantList &)
    : VpnUiPlugin(parent)
{
}

L2tpUiPlugin::~L2tpUiPlugin() = default;

SettingWidget *L2tpUiPlugin::widget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent)
{
    return new L2tpWidget(setting, parent);
}

SettingWidget *L2tpUiPlugin::askUser(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent)
{
    return new L2tpAuthWidget(setting, hints, parent);
}

QString L2tpUiPlugin::suggestedFileName(const NetworkManager::ConnectionSettings::Ptr &connection) const
{
    Q_UNUSED(connection);
    return {};
}

#include "l2tp.moc"

#include "moc_l2tp.cpp"
