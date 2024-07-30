/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_SSIDCOMBOBOX_H
#define PLASMA_NM_SSIDCOMBOBOX_H

#include "plasmanm_editor_export.h"

#include <KComboBox>

#include <NetworkManagerQt/WirelessNetwork>

class PLASMANM_EDITOR_EXPORT SsidComboBox : public KComboBox
{
    Q_OBJECT
public:
    explicit SsidComboBox(QWidget *parent = nullptr);

    void init(const QString &ssid);

    QString ssid() const;

Q_SIGNALS:
    void ssidChanged();

private Q_SLOTS:
    void slotEditTextChanged(const QString &text);
    void slotCurrentIndexChanged(int);

private:
    void addSsidsToCombo(const QList<NetworkManager::WirelessNetwork::Ptr> &networks);
    QString m_initialSsid;
};

#endif // PLASMA_NM_SSIDCOMBOBOX_H
