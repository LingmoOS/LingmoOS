// SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "lingmopassplugin.h"
#include "passwordfiltermodel.h"
#include "passwordprovider.h"
#include "otpprovider.h"
#include "passwordsmodel.h"
#include "passwordsortproxymodel.h"

#include <QJSEngine>
#include <QQmlContext>
#include <QQmlEngine>

void LingmoPassPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.lingmo.private.lingmopass"));

    qmlRegisterType<LingmoPass::PasswordsModel>(uri, 1, 0, "PasswordsModel");
    qmlRegisterType<LingmoPass::PasswordSortProxyModel>(uri, 1, 0, "PasswordSortProxyModel");
    qmlRegisterType<LingmoPass::PasswordFilterModel>(uri, 1, 0, "PasswordFilterModel");
    qmlRegisterUncreatableType<LingmoPass::ProviderBase>(uri, 1, 0, "ProviderBase", QString());
    qmlRegisterUncreatableType<LingmoPass::PasswordProvider>(uri, 1, 0, "PasswordProvider", QString());
    qmlRegisterUncreatableType<LingmoPass::OTPProvider>(uri, 1, 0, "OTPProvider", QString());

    qmlProtectModule("org.kde.lingmo.private.lingmopass", 1);
}

#include "moc_lingmopassplugin.cpp"
