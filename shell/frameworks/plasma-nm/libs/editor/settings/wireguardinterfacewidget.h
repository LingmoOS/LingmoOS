/*
    SPDX-FileCopyrightText: 2019 Bruce Anderson <banderson19com@san.rr.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_WIREGUARD_INTERFACE_WIDGET_H
#define PLASMA_NM_WIREGUARD_INTERFACE_WIDGET_H

#include "plasmanm_editor_export.h"

#include <QWidget>

#include "settingwidget.h"
#include "ui_wireguardinterfacewidget.h"
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/WireguardSetting>

class PLASMANM_EDITOR_EXPORT WireGuardInterfaceWidget : public SettingWidget
{
    Q_OBJECT

public:
    explicit WireGuardInterfaceWidget(const NetworkManager::ConnectionSettings::Ptr &connectionSettings, QWidget *parent = nullptr, Qt::WindowFlags f = {});
    ~WireGuardInterfaceWidget() override;

    void loadConfig(const NetworkManager::Setting::Ptr &setting) override;
    void loadSecrets(const NetworkManager::Setting::Ptr &setting) override;

    QVariantMap setting() const override;

    bool isValid() const override;
    static QString supportedFileExtensions();
    static NMVariantMapMap importConnectionSettings(const QString &fileName);

    QString interfaceName() const;

private Q_SLOTS:
    void showPeers();

private:
    void setBackground(QWidget *w, bool result) const;
    void checkInterfaceNameValid();
    void checkPrivateKeyValid();
    void checkFwmarkValid();
    void checkListenPortValid();
    static QString sanitizeInterfaceName(const QString &interfaceName);
    class Private;
    Private *const d;
};

#endif // PLASMA_NM_WIREGUARD_INTERFACE_WIDGET_H
