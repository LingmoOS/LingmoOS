/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "vaultinfo.h"

#include <QDBusMetaType>

auto static_init_VaultInfo = [] {
    qDBusRegisterMetaType<PlasmaVault::VaultInfo>();
    qDBusRegisterMetaType<PlasmaVault::VaultInfoList>();
    return true;
}();
