/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "openconnectui.h"

#include <KPluginFactory>

#include "openconnectauth.h"
#include "openconnectwidget.h"

OpenconnectUiPlugin::OpenconnectUiPlugin(QObject *parent, const QVariantList &)
    : VpnUiPlugin(parent)
{
}

OpenconnectUiPlugin::~OpenconnectUiPlugin() = default;

SettingWidget *OpenconnectUiPlugin::widget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent)
{
    return new OpenconnectSettingWidget(setting, parent);
}

SettingWidget *OpenconnectUiPlugin::askUser(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent)
{
    return new OpenconnectAuthWidget(setting, hints, parent);
}

QString OpenconnectUiPlugin::suggestedFileName(const NetworkManager::ConnectionSettings::Ptr &connection) const
{
    Q_UNUSED(connection);
    return {};
}

QMessageBox::StandardButtons OpenconnectUiPlugin::suggestedAuthDialogButtons() const
{
    return QMessageBox::Close;
}

#include "moc_openconnectui.cpp"
