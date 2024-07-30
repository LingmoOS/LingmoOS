/*
    SPDX-FileCopyrightText: 2009 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "pptp.h"

#include <KPluginFactory>

#include "pptpauth.h"
#include "pptpwidget.h"

K_PLUGIN_CLASS_WITH_JSON(PptpUiPlugin, "plasmanetworkmanagement_pptpui.json")

PptpUiPlugin::PptpUiPlugin(QObject *parent, const QVariantList &)
    : VpnUiPlugin(parent)
{
}

PptpUiPlugin::~PptpUiPlugin() = default;

SettingWidget *PptpUiPlugin::widget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent)
{
    return new PptpSettingWidget(setting, parent);
}

SettingWidget *PptpUiPlugin::askUser(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent)
{
    return new PptpAuthWidget(setting, hints, parent);
}

QString PptpUiPlugin::suggestedFileName(const NetworkManager::ConnectionSettings::Ptr &connection) const
{
    Q_UNUSED(connection);
    return {};
}

#include "pptp.moc"

#include "moc_pptp.cpp"
