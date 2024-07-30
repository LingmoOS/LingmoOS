/*
    SPDX-FileCopyrightText: 2008 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2011 Rajeesh K Nambiar <rajeeshknambiar@gmail.com>
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>
    SPDX-FileCopyrightText: 2014 Lamarque V. Souza <lamarque@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PLASMANM_OPENVPN_H
#define PLASMANM_OPENVPN_H

#include "vpnuiplugin.h"

#include <QTextStream>
#include <QVariant>

class Q_DECL_EXPORT OpenVpnUiPlugin : public VpnUiPlugin
{
    Q_OBJECT
public:
    explicit OpenVpnUiPlugin(QObject *parent = nullptr, const QVariantList & = QVariantList());
    ~OpenVpnUiPlugin() override;
    SettingWidget *widget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent) override;
    SettingWidget *askUser(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent) override;

    QString suggestedFileName(const NetworkManager::ConnectionSettings::Ptr &connection) const override;
    QStringList supportedFileExtensions() const override;
    ImportResult importConnectionSettings(const QString &fileName) override;
    ExportResult exportConnectionSettings(const NetworkManager::ConnectionSettings::Ptr &connection, const QString &fileName) override;

private:
    QString saveFile(QTextStream &in, const QString &endTag, const QString &connectionName, const QString &fileName);
    QString tryToCopyToCertificatesDirectory(const QString &connectionName, const QString &sourceFilePath);
};

#endif //  PLASMANM_OPENVPN_H
