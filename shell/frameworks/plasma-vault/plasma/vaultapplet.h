/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef PLASMAVAULT_PLASMA_VAULT_APPLET_H
#define PLASMAVAULT_PLASMA_VAULT_APPLET_H

#include <QObject>

#include <Plasma/Applet>

class VaultsModel;

class VaultApplet : public Plasma::Applet
{
    Q_OBJECT
    Q_PROPERTY(QObject *vaultsModel READ vaultsModel CONSTANT)

public:
    explicit VaultApplet(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
    ~VaultApplet() override;

    QObject *vaultsModel();

private:
    // Applet will be the parent of this object
    QObject *m_vaultsModel = nullptr;
};

#endif // include guard
