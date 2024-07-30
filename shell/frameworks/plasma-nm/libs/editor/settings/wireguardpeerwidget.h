/*
    SPDX-FileCopyrightText: 2019 Bruce Anderson <banderson19com@san.rr.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_WIREGUARD_PEER_WIDGET_H
#define PLASMA_NM_WIREGUARD_PEER_WIDGET_H

#include "plasmanm_editor_export.h"

#include <QDialog>

#include "settingwidget.h"
#include <NetworkManagerQt/WireguardSetting>

class PLASMANM_EDITOR_EXPORT WireGuardPeerWidget : public QDialog
{
    Q_OBJECT

public:
    explicit WireGuardPeerWidget(const QVariantMap &peerData, QWidget *parent = nullptr, Qt::WindowFlags f = {});
    ~WireGuardPeerWidget() override;

    QVariantMap setting() const;
    bool isValid();
    enum EndPointValid { BothValid, AddressValid, PortValid, BothInvalid };
    static WireGuardPeerWidget::EndPointValid isEndpointValid(QString &, QString &);

Q_SIGNALS:
    void notifyValid();

private:
    void setBackground(QWidget *w, bool result) const;
    void checkPublicKeyValid();
    void checkPresharedKeyValid();
    void checkAllowedIpsValid();
    void checkEndpointValid();
    void updatePeerWidgets();
    void saveKeepAlive();
    void saveKeyFlags();
    void slotWidgetChanged();

    class Private;
    Private *const d;
};

#endif // PLASMA_NM_WIREGUARD_PEER_WIDGET_H
