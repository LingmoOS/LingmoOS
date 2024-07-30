// SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "plasmapassplugin.h"
#include "passwordfiltermodel.h"
#include "passwordprovider.h"
#include "otpprovider.h"
#include "passwordsmodel.h"
#include "passwordsortproxymodel.h"

#include <QJSEngine>
#include <QQmlContext>
#include <QQmlEngine>

void PlasmaPassPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.plasma.private.plasmapass"));

    qmlRegisterType<PlasmaPass::PasswordsModel>(uri, 1, 0, "PasswordsModel");
    qmlRegisterType<PlasmaPass::PasswordSortProxyModel>(uri, 1, 0, "PasswordSortProxyModel");
    qmlRegisterType<PlasmaPass::PasswordFilterModel>(uri, 1, 0, "PasswordFilterModel");
    qmlRegisterUncreatableType<PlasmaPass::ProviderBase>(uri, 1, 0, "ProviderBase", QString());
    qmlRegisterUncreatableType<PlasmaPass::PasswordProvider>(uri, 1, 0, "PasswordProvider", QString());
    qmlRegisterUncreatableType<PlasmaPass::OTPProvider>(uri, 1, 0, "OTPProvider", QString());

    qmlProtectModule("org.kde.plasma.private.plasmapass", 1);
}

#include "moc_plasmapassplugin.cpp"
