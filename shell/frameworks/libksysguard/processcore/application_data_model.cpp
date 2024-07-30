/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "application_data_model.h"
#include <KUser>

#include <QDebug>

using namespace KSysGuard;

ApplicationDataModel::ApplicationDataModel(QObject *parent)
    : CGroupDataModel(QStringLiteral("/user.slice/user-%1.slice/user@%1.service").arg(KUserId::currentEffectiveUserId().toString()), parent)
{
}

bool ApplicationDataModel::filterAcceptsCGroup(const QString &id)
{
    if (!CGroupDataModel::filterAcceptsCGroup(id)) {
        return false;
    }
    // this class is all temporary. In the future as per https://systemd.io/DESKTOP_ENVIRONMENTS/
    // all apps will have a managed by a drop-in that puts apps in the app.slice
    // when this happens adjust the root above and drop this filterAcceptsCGroup line
    return id.contains(QLatin1String("/app-")) || (id.contains(QLatin1String("/flatpak")) && id.endsWith(QLatin1String("scope")));
}
