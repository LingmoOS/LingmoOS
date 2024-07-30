/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2008 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kmimeassociations_p.h"
#include "sycocadebug.h"
#include <KConfig>
#include <KConfigGroup>
#include <QDebug>
#include <QFile>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <kservice.h>
#include <kservicefactory_p.h>

KMimeAssociations::KMimeAssociations(KOfferHash &offerHash, KServiceFactory *serviceFactory)
    : m_offerHash(offerHash)
    , m_serviceFactory(serviceFactory)
{
}

/*

The goal of this class is to parse mimeapps.list files, which are used to
let users configure the application-MIME type associations.

Example file:

[Added Associations]
text/plain=gnome-gedit.desktop;gnu-emacs.desktop;

[Removed Associations]
text/plain=gnome-gedit.desktop;gnu-emacs.desktop;

[Default Applications]
text/plain=kate.desktop;
*/

QStringList KMimeAssociations::mimeAppsFiles()
{
    QStringList mimeappsFileNames;
    // make the list of possible filenames from the spec ($desktop-mimeapps.list, then mimeapps.list)
    const QString desktops = QString::fromLocal8Bit(qgetenv("XDG_CURRENT_DESKTOP"));
    const auto list = desktops.split(QLatin1Char(':'), Qt::SkipEmptyParts);
    for (const QString &desktop : list) {
        mimeappsFileNames.append(desktop.toLower() + QLatin1String("-mimeapps.list"));
    }
    mimeappsFileNames.append(QStringLiteral("mimeapps.list"));
    const QStringList mimeappsDirs = mimeAppsDirs();
    QStringList mimeappsFiles;
    // collect existing files
    for (const QString &dir : mimeappsDirs) {
        for (const QString &file : std::as_const(mimeappsFileNames)) {
            const QString filePath = dir + QLatin1Char('/') + file;
            if (QFile::exists(filePath) && !mimeappsFiles.contains(filePath)) {
                mimeappsFiles.append(filePath);
            }
        }
    }
    return mimeappsFiles;
}

QStringList KMimeAssociations::mimeAppsDirs()
{
    // list the dirs in the order of the spec (XDG_CONFIG_HOME, XDG_CONFIG_DIRS, XDG_DATA_HOME, XDG_DATA_DIRS)
    return QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation) + QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
}

void KMimeAssociations::parseAllMimeAppsList()
{
    int basePreference = 1000; // start high :)
    const QStringList files = KMimeAssociations::mimeAppsFiles();
    // Global first, then local
    auto it = files.crbegin();
    auto endIt = files.crend();
    for (; it != endIt; ++it) {
        // qDebug() << "Parsing" << mimeappsFile;
        parseMimeAppsList(*it, basePreference);
        basePreference += 50;
    }
}

void KMimeAssociations::parseMimeAppsList(const QString &file, int basePreference)
{
    KConfig profile(file, KConfig::SimpleConfig);
    if (file.endsWith(QLatin1String("/mimeapps.list"))) { // not for $desktop-mimeapps.list
        parseAddedAssociations(KConfigGroup(&profile, QStringLiteral("Added Associations")), file, basePreference);
        parseRemovedAssociations(KConfigGroup(&profile, QStringLiteral("Removed Associations")), file);

        // KDE extension for parts and plugins, see settings/filetypes/mimetypedata.cpp
        parseAddedAssociations(KConfigGroup(&profile, QStringLiteral("Added KDE Service Associations")), file, basePreference);
        parseRemovedAssociations(KConfigGroup(&profile, QStringLiteral("Removed KDE Service Associations")), file);
    }

    // Default Applications is preferred over Added Associations.
    // Other than that, they work the same...
    // add 25 to the basePreference to make sure those service offers will have higher preferences
    // 25 is arbitrary half of the allocated preference indices for the current parsed mimeapps.list file, defined line 86
    parseAddedAssociations(KConfigGroup(&profile, QStringLiteral("Default Applications")), file, basePreference + 25);
}

void KMimeAssociations::parseAddedAssociations(const KConfigGroup &group, const QString &file, int basePreference)
{
    Q_UNUSED(file) // except in debug statements
    QMimeDatabase db;
    const auto keyList = group.keyList();
    for (const QString &mimeName : keyList) {
        const QStringList services = group.readXdgListEntry(mimeName);
        const QString resolvedMimeName = mimeName.startsWith(QLatin1String("x-scheme-handler/")) ? mimeName : db.mimeTypeForName(mimeName).name();
        if (resolvedMimeName.isEmpty()) {
            qCDebug(SYCOCA) << file << "specifies unknown MIME type" << mimeName << "in" << group.name();
        } else {
            int pref = basePreference;
            for (const QString &service : services) {
                KService::Ptr pService = m_serviceFactory->findServiceByStorageId(service);
                if (!pService) {
                    qCDebug(SYCOCA) << file << "specifies unknown service" << service << "in" << group.name();
                } else {
                    // qDebug() << "adding mime" << resolvedMimeName << "to service" << pService->entryPath() << "pref=" << pref;
                    m_offerHash.addServiceOffer(resolvedMimeName, KServiceOffer(pService, pref, 0));
                    --pref;
                }
            }
        }
    }
}

void KMimeAssociations::parseRemovedAssociations(const KConfigGroup &group, const QString &file)
{
    Q_UNUSED(file) // except in debug statements
    const auto keyList = group.keyList();
    for (const QString &mime : keyList) {
        const QStringList services = group.readXdgListEntry(mime);
        for (const QString &service : services) {
            KService::Ptr pService = m_serviceFactory->findServiceByStorageId(service);
            if (!pService) {
                // qDebug() << file << "specifies unknown service" << service << "in" << group.name();
            } else {
                // qDebug() << "removing mime" << mime << "from service" << pService.data() << pService->entryPath();
                m_offerHash.removeServiceOffer(mime, pService);
            }
        }
    }
}

void KOfferHash::addServiceOffer(const QString &serviceType, const KServiceOffer &offer)
{
    KService::Ptr service = offer.service();
    // qDebug() << "Adding" << service->entryPath() << "to" << serviceType << offer.preference();
    ServiceTypeOffersData &data = m_serviceTypeData[serviceType]; // find or create
    QList<KServiceOffer> &offers = data.offers;
    QSet<KService::Ptr> &offerSet = data.offerSet;
    if (!offerSet.contains(service)) {
        offers.append(offer);
        offerSet.insert(service);
    } else {
        const int initPref = offer.preference();
        // qDebug() << service->entryPath() << "already in" << serviceType;
        // This happens when mimeapps.list mentions a service (to make it preferred)
        // Update initialPreference to std::max(existing offer, new offer)
        for (KServiceOffer &servOffer : data.offers) {
            if (servOffer.service() == service) { // we can compare KService::Ptrs because they are from the memory hash
                servOffer.setPreference(std::max(servOffer.preference(), initPref));
            }
        }
    }
}

void KOfferHash::removeServiceOffer(const QString &serviceType, const KService::Ptr &service)
{
    ServiceTypeOffersData &data = m_serviceTypeData[serviceType]; // find or create
    data.removedOffers.insert(service);
    data.offerSet.remove(service);

    const QString id = service->storageId();

    auto &list = data.offers;
    auto it = std::remove_if(list.begin(), list.end(), [&id](const KServiceOffer &offer) {
        return offer.service()->storageId() == id;
    });
    list.erase(it, list.end());
}

bool KOfferHash::hasRemovedOffer(const QString &serviceType, const KService::Ptr &service) const
{
    auto it = m_serviceTypeData.constFind(serviceType);
    if (it != m_serviceTypeData.cend()) {
        return it.value().removedOffers.contains(service);
    }
    return false;
}
