/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Torben Weis <weis@kde.org>
    SPDX-FileCopyrightText: 2006-2020 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kapplicationtrader.h"

#include "kmimetypefactory_p.h"
#include "kservicefactory_p.h"
#include "ksycoca.h"
#include "ksycoca_p.h"
#include "servicesdebug.h"

#include <QMimeDatabase>

#include <KConfigGroup>
#include <KSharedConfig>

static KService::List mimeTypeSycocaServiceOffers(const QString &mimeType)
{
    KService::List lst;
    QMimeDatabase db;
    QString mime = db.mimeTypeForName(mimeType).name();
    if (mime.isEmpty()) {
        if (!mimeType.startsWith(QLatin1String("x-scheme-handler/"))) { // don't warn for unknown scheme handler mimetypes
            qCWarning(SERVICES) << "KApplicationTrader: mimeType" << mimeType << "not found";
            return lst; // empty
        }
        mime = mimeType;
    }
    KSycoca::self()->ensureCacheValid();
    KMimeTypeFactory *factory = KSycocaPrivate::self()->mimeTypeFactory();
    const int offset = factory->entryOffset(mime);
    if (!offset) {
        if (!mimeType.startsWith(QLatin1String("x-scheme-handler/"))) { // don't warn for unknown scheme handler mimetypes
            qCWarning(SERVICES) << "KApplicationTrader: mimeType" << mimeType << "not found";
        }
        return lst; // empty
    }
    const int serviceOffersOffset = factory->serviceOffersOffset(mime);
    if (serviceOffersOffset > -1) {
        lst = KSycocaPrivate::self()->serviceFactory()->serviceOffers(offset, serviceOffersOffset);
    }
    return lst;
}

static void applyFilter(KService::List &list, KApplicationTrader::FilterFunc filterFunc, bool mustShowInCurrentDesktop)
{
    if (list.isEmpty()) {
        return;
    }

    // Find all services matching the constraint
    // and remove the other ones
    auto removeFunc = [&](const KService::Ptr &serv) {
        return (filterFunc && !filterFunc(serv)) || (mustShowInCurrentDesktop && !serv->showInCurrentDesktop());
    };
    list.erase(std::remove_if(list.begin(), list.end(), removeFunc), list.end());
}

KService::List KApplicationTrader::query(FilterFunc filterFunc)
{
    // Get all applications
    KSycoca::self()->ensureCacheValid();
    KService::List lst = KSycocaPrivate::self()->serviceFactory()->allServices();

    applyFilter(lst, filterFunc, true); // true = filter out service with NotShowIn=KDE or equivalent

    qCDebug(SERVICES) << "query returning" << lst.count() << "offers";
    return lst;
}

KService::List KApplicationTrader::queryByMimeType(const QString &mimeType, FilterFunc filterFunc)
{
    // Get all services of this MIME type.
    KService::List lst = mimeTypeSycocaServiceOffers(mimeType);

    applyFilter(lst, filterFunc, false); // false = allow NotShowIn=KDE services listed in mimeapps.list

    qCDebug(SERVICES) << "query for mimeType" << mimeType << "returning" << lst.count() << "offers";
    return lst;
}

KService::Ptr KApplicationTrader::preferredService(const QString &mimeType)
{
    const KService::List offers = queryByMimeType(mimeType);
    if (!offers.isEmpty()) {
        return offers.at(0);
    }
    return KService::Ptr();
}

void KApplicationTrader::setPreferredService(const QString &mimeType, const KService::Ptr service)
{
    if (mimeType.isEmpty() || !(service && service->isValid())) {
        return;
    }
    KSharedConfig::Ptr profile = KSharedConfig::openConfig(QStringLiteral("mimeapps.list"), KConfig::NoGlobals, QStandardPaths::GenericConfigLocation);

    // Save the default application according to mime-apps-spec 1.0
    KConfigGroup defaultApp(profile, QStringLiteral("Default Applications"));
    defaultApp.writeXdgListEntry(mimeType, QStringList(service->storageId()));

    KConfigGroup addedApps(profile, QStringLiteral("Added Associations"));
    QStringList apps = addedApps.readXdgListEntry(mimeType);
    apps.removeAll(service->storageId());
    apps.prepend(service->storageId()); // make it the preferred app
    addedApps.writeXdgListEntry(mimeType, apps);

    profile->sync();

    // Also make sure the "auto embed" setting for this MIME type is off
    KSharedConfig::Ptr fileTypesConfig = KSharedConfig::openConfig(QStringLiteral("filetypesrc"), KConfig::NoGlobals);
    fileTypesConfig->group(QStringLiteral("EmbedSettings")).writeEntry(QStringLiteral("embed-") + mimeType, false);
    fileTypesConfig->sync();
}

bool KApplicationTrader::isSubsequence(const QString &pattern, const QString &text, Qt::CaseSensitivity cs)
{
    if (pattern.isEmpty()) {
        return false;
    }
    const bool chk_case = cs == Qt::CaseSensitive;

    auto textIt = text.cbegin();
    auto patternIt = pattern.cbegin();
    for (; textIt != text.cend() && patternIt != pattern.cend(); ++textIt) {
        if ((chk_case && *textIt == *patternIt) || (!chk_case && textIt->toLower() == patternIt->toLower())) {
            ++patternIt;
        }
    }
    return patternIt == pattern.cend();
}
