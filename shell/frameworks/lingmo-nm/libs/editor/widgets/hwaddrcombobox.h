/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef LINGMO_NM_HWADDRCOMBOBOX_H
#define LINGMO_NM_HWADDRCOMBOBOX_H

#include "lingmonm_editor_export.h"

#include <QComboBox>

#include <NetworkManagerQt/Device>

class LINGMONM_EDITOR_EXPORT HwAddrComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit HwAddrComboBox(QWidget *parent = nullptr);

    void init(NetworkManager::Device::Type deviceType, const QString &address);

    bool isValid() const;
    QString hwAddress() const;

Q_SIGNALS:
    void hwAddressChanged();

private:
    void slotEditTextChanged(const QString &);
    void slotCurrentIndexChanged(int);
    void addAddressToCombo(const NetworkManager::Device::Ptr &device);
    QVariant hwAddressFromDevice(const NetworkManager::Device::Ptr &device);
    QString m_initialAddress;
    bool m_dirty = false;
};

#endif // LINGMO_NM_HWADDRCOMBOBOX_H
