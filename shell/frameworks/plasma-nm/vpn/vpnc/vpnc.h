/*
    SPDX-FileCopyrightText: 2008 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2011-2012 Rajeesh K Nambiar <rajeeshknambiar@gmail.com>
    SPDX-FileCopyrightText: 2011-2012 Lamarque V. Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PLASMA_NM_VPNC_H
#define PLASMA_NM_VPNC_H

#include "vpnuiplugin.h"

#include <QVariant>

#include <KConfigGroup>
#include <KProcess>

class VpncUiPluginPrivate : public QObject
{
    Q_OBJECT
public:
    VpncUiPluginPrivate();
    ~VpncUiPluginPrivate() override;
    QString readStringKeyValue(const KConfigGroup &configGroup, const QString &key);
    KProcess *ciscoDecrypt = nullptr;
    QString decryptedPasswd;

public Q_SLOTS:
    void gotCiscoDecryptOutput();
    void ciscoDecryptError(QProcess::ProcessError pError);
    void ciscoDecryptFinished(int exitCode, QProcess::ExitStatus exitStatus);
};

class Q_DECL_EXPORT VpncUiPlugin : public VpnUiPlugin
{
    Q_OBJECT

public:
    explicit VpncUiPlugin(QObject *parent = nullptr, const QVariantList & = QVariantList());
    ~VpncUiPlugin() override;
    SettingWidget *widget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent) override;
    SettingWidget *askUser(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent) override;

    QString suggestedFileName(const NetworkManager::ConnectionSettings::Ptr &connection) const override;
    QStringList supportedFileExtensions() const override;
    ImportResult importConnectionSettings(const QString &fileName) override;
    ExportResult exportConnectionSettings(const NetworkManager::ConnectionSettings::Ptr &connection, const QString &fileName) override;
};

#endif // PLASMA_NM_VPNC_H
