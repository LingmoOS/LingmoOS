/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_BSSIDCOMBOBOX_H
#define PLASMA_NM_BSSIDCOMBOBOX_H

#include "plasmanm_editor_export.h"

#include <QComboBox>

#include <NetworkManagerQt/AccessPoint>
#include <NetworkManagerQt/Device>

class PLASMANM_EDITOR_EXPORT BssidComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit BssidComboBox(QWidget *parent = nullptr);

    QString bssid() const;
    bool isValid() const;

Q_SIGNALS:
    void bssidChanged();

public Q_SLOTS:
    void init(const QString &bssid, const QString &ssid);

private Q_SLOTS:
    void slotEditTextChanged(const QString &);
    void slotCurrentIndexChanged(int);

private:
    void addBssidsToCombo(const QList<NetworkManager::AccessPoint::Ptr> &aps);

    QString m_initialBssid;
    bool m_dirty = false;
};

#endif // PLASMA_NM_BSSIDCOMBOBOX_H
