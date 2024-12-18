// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "licenceInterface.h"
/*
 * Implementation of interface class ComLingmoLicenseInterface
 */

ComLingmoLicenseInterface::ComLingmoLicenseInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

ComLingmoLicenseInterface::~ComLingmoLicenseInterface()
{
}
