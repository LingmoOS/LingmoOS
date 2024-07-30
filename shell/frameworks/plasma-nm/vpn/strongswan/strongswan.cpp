/*
    SPDX-FileCopyrightText: 2009 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2010 Maurus Rohrer <maurus.rohrer@gmail.com>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "strongswan.h"
#include "strongswanauth.h"
#include "strongswanwidget.h"

#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(StrongswanUiPlugin, "plasmanetworkmanagement_strongswanui.json")

StrongswanUiPlugin::StrongswanUiPlugin(QObject *parent, const QVariantList &)
    : VpnUiPlugin(parent)
{
}

StrongswanUiPlugin::~StrongswanUiPlugin() = default;

SettingWidget *StrongswanUiPlugin::widget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent)
{
    return new StrongswanSettingWidget(setting, parent);
}

SettingWidget *StrongswanUiPlugin::askUser(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent)
{
    return new StrongswanAuthWidget(setting, hints, parent);
}

QString StrongswanUiPlugin::suggestedFileName(const NetworkManager::ConnectionSettings::Ptr &connection) const
{
    Q_UNUSED(connection);

    // TODO : implement suggested file name
    return {};
}

#include "strongswan.moc"

// vim: sw=4 sts=4 et tw=100

#include "moc_strongswan.cpp"
