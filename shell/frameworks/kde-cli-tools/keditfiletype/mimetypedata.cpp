/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>
   SPDX-FileCopyrightText: 2003, 2007 David Faure <faure@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
*/

#include "mimetypedata.h"
#include "mimetypewriter.h"
#include <KApplicationTrader>
#include <KConfigGroup>
#include <KParts/PartLoader>
#include <KProtocolManager>
#include <KService>
#include <KSharedConfig>
#include <QDebug>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QXmlStreamReader>

MimeTypeData::MimeTypeData(const QString &major)
    : m_askSave(AskSaveDefault)
    , m_bNewItem(false)
    , m_bFullInit(true)
    , m_isGroup(true)
    , m_appServicesModified(false)
    , m_embedServicesModified(false)
    , m_userSpecifiedIconModified(false)
    , m_major(major)
{
    m_autoEmbed = readAutoEmbed();
}

MimeTypeData::MimeTypeData(const QMimeType &mime)
    : m_mimetype(mime)
    , m_askSave(AskSaveDefault)
    , // TODO: the code for initializing this is missing. FileTypeDetails initializes the checkbox instead...
    m_bNewItem(false)
    , m_bFullInit(false)
    , m_isGroup(false)
    , m_appServicesModified(false)
    , m_embedServicesModified(false)
    , m_userSpecifiedIconModified(false)
{
    const QString mimeName = m_mimetype.name();
    Q_ASSERT(!mimeName.isEmpty());
    const int index = mimeName.indexOf(QLatin1Char('/'));
    if (index != -1) {
        m_major = mimeName.left(index);
        m_minor = mimeName.mid(index + 1);
    } else {
        m_major = mimeName;
    }
    initFromQMimeType();
}

MimeTypeData::MimeTypeData(const QString &mimeName, bool)
    : m_askSave(AskSaveDefault)
    , m_bNewItem(true)
    , m_bFullInit(false)
    , m_isGroup(false)
    , m_appServicesModified(false)
    , m_embedServicesModified(false)
    , m_userSpecifiedIconModified(false)
{
    const int index = mimeName.indexOf(QLatin1Char('/'));
    if (index != -1) {
        m_major = mimeName.left(index);
        m_minor = mimeName.mid(index + 1);
    } else {
        m_major = mimeName;
    }
    m_autoEmbed = UseGroupSetting;
    // all the rest is empty by default
}

void MimeTypeData::initFromQMimeType()
{
    m_comment = m_mimetype.comment();
    setPatterns(m_mimetype.globPatterns());
    m_autoEmbed = readAutoEmbed();

    // Parse XML file to find out if the user specified a custom icon name
    QString file = name().toLower() + QLatin1String(".xml");
    QStringList mimeFiles = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("mime/") + file);
    if (mimeFiles.isEmpty()) {
        // This is for shared-mime-info < 1.3 that did not lowecase mime names
        file = name() + QLatin1String(".xml");
        mimeFiles = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("mime/") + file);
        if (mimeFiles.isEmpty()) {
            qWarning() << "No file found for" << file << ", even though the file appeared in a directory listing.";
            qWarning() << "Either it was just removed, or the directory doesn't have executable permission...";
            qWarning() << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("mime"), QStandardPaths::LocateDirectory);
            return;
        }
    }

    // Reverse iterator to get global first, then local.
    for (auto rIt = mimeFiles.crbegin(); rIt != mimeFiles.crend(); ++rIt) {
        const QString fullPath = *rIt;
        QFile qfile(fullPath);
        if (!qfile.open(QFile::ReadOnly)) {
            continue;
        }

        QXmlStreamReader xml(&qfile);
        if (xml.readNextStartElement()) {
            if (xml.name() != QLatin1String("mime-type")) {
                continue;
            }
            const QString mimeName = xml.attributes().value(QLatin1String("type")).toString();
            if (mimeName.isEmpty()) {
                continue;
            }
            if (QString::compare(mimeName, name(), Qt::CaseInsensitive) != 0) {
                qWarning() << "Got name" << mimeName << "in file" << file << "expected" << name();
            }

            while (xml.readNextStartElement()) {
                const auto tag = xml.name();
                if (tag == QLatin1String("icon")) {
                    m_userSpecifiedIcon = xml.attributes().value(QLatin1String("name")).toString();
                }
                xml.skipCurrentElement();
            }
        }
    }
}

MimeTypeData::AutoEmbed MimeTypeData::readAutoEmbed() const
{
    const KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("filetypesrc"), KConfig::NoGlobals);
    const QString key = QStringLiteral("embed-") + name();
    const KConfigGroup group(config, "EmbedSettings");
    if (m_isGroup) {
        // embedding is false by default except for image/*, multipart/* and inode/* (hardcoded in konq)
        const bool defaultValue = (m_major == QLatin1String("image") || m_major == QLatin1String("multipart") || m_major == QLatin1String("inode"));
        return group.readEntry(key, defaultValue) ? Yes : No;
    } else {
        if (group.hasKey(key)) {
            return group.readEntry(key, false) ? Yes : No;
        }
        // TODO if ( !mimetype.property( "X-KDE-LocalProtocol" ).toString().isEmpty() )
        // TODO    return MimeTypeData::Yes; // embed by default for zip, tar etc.
        return MimeTypeData::UseGroupSetting;
    }
}

void MimeTypeData::writeAutoEmbed()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("filetypesrc"), KConfig::NoGlobals);
    if (!config->isConfigWritable(true)) {
        return;
    }

    const QString key = QStringLiteral("embed-") + name();
    KConfigGroup group(config, "EmbedSettings");
    if (m_isGroup) {
        group.writeEntry(key, m_autoEmbed == Yes);
    } else {
        if (m_autoEmbed == UseGroupSetting) {
            group.deleteEntry(key);
        } else {
            group.writeEntry(key, m_autoEmbed == Yes);
        }
    }
}

bool MimeTypeData::isEssential() const
{
    // Keep in sync with KMimeType::checkEssentialMimeTypes
    const QString n = name();
    if (n == QLatin1String("application/octet-stream")) {
        return true;
    }
    if (n == QLatin1String("inode/directory")) {
        return true;
    }
    if (n == QLatin1String("inode/blockdevice")) {
        return true;
    }
    if (n == QLatin1String("inode/chardevice")) {
        return true;
    }
    if (n == QLatin1String("inode/socket")) {
        return true;
    }
    if (n == QLatin1String("inode/fifo")) {
        return true;
    }
    if (n == QLatin1String("application/x-shellscript")) {
        return true;
    }
    if (n == QLatin1String("application/x-executable")) {
        return true;
    }
    if (n == QLatin1String("application/x-desktop")) {
        return true;
    }
    return false;
}

void MimeTypeData::setUserSpecifiedIcon(const QString &icon)
{
    if (icon == m_userSpecifiedIcon) {
        return;
    }
    m_userSpecifiedIcon = icon;
    m_userSpecifiedIconModified = true;
}

QStringList MimeTypeData::getAppOffers() const
{
    QStringList serviceIds;
    const KService::List offerList = KApplicationTrader::queryByMimeType(name());
    for (const auto &servicePtr : offerList) {
        serviceIds.append(servicePtr->storageId());
    }
    return serviceIds;
}

QStringList MimeTypeData::getPartOffers() const
{
    QStringList servicesIds;
    // We do not do any sorting, because KParts uses the order in which the entries are saved
    const auto partOfferList = KParts::PartLoader::partsForMimeType(name());
    for (const auto &metaData : partOfferList) {
        servicesIds.append(metaData.pluginId());
    }
    return servicesIds;
}

void MimeTypeData::getMyServiceOffers() const
{
    m_appServices = getAppOffers();
    m_embedParts = getPartOffers();
    m_bFullInit = true;
}

QStringList MimeTypeData::appServices() const
{
    if (!m_bFullInit) {
        getMyServiceOffers();
    }
    return m_appServices;
}

QStringList MimeTypeData::embedParts() const
{
    if (!m_bFullInit) {
        getMyServiceOffers();
    }
    return m_embedParts;
}

bool MimeTypeData::isMimeTypeDirty() const
{
    Q_ASSERT(!m_isGroup);
    if (m_bNewItem) {
        return true;
    }

    if (!m_mimetype.isValid()) {
        qWarning() << "MimeTypeData for" << name() << "says 'not new' but is without a mimetype? Should not happen.";
        return true;
    }

    if (m_mimetype.comment() != m_comment) {
        qDebug() << "Mimetype Comment Dirty: old=" << m_mimetype.comment() << "m_comment=" << m_comment;
        return true;
    }
    if (m_userSpecifiedIconModified) {
        qDebug() << "m_userSpecifiedIcon has changed. Now set to" << m_userSpecifiedIcon;
        return true;
    }

    QStringList storedPatterns = m_mimetype.globPatterns();
    storedPatterns.sort(); // see ctor
    if (storedPatterns != m_patterns) {
        qDebug() << "Mimetype Patterns Dirty: old=" << storedPatterns << "m_patterns=" << m_patterns;
        return true;
    }

    if (readAutoEmbed() != m_autoEmbed) {
        return true;
    }
    return false;
}

bool MimeTypeData::isServiceListDirty() const
{
    return !m_isGroup && (m_appServicesModified || m_embedServicesModified);
}

bool MimeTypeData::isDirty() const
{
    if (m_bNewItem) {
        qDebug() << "New item, need to save it";
        return true;
    }

    if (!m_isGroup) {
        if (isServiceListDirty()) {
            return true;
        }
        if (isMimeTypeDirty()) {
            return true;
        }
    } else { // is a group
        if (readAutoEmbed() != m_autoEmbed) {
            return true;
        }
    }

    if (m_askSave != AskSaveDefault) {
        return true;
    }

    // nothing seems to have changed, it's not dirty.
    return false;
}

bool MimeTypeData::sync()
{
    if (m_isGroup) {
        writeAutoEmbed();
        return false;
    }

    if (m_askSave != AskSaveDefault) {
        KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("filetypesrc"), KConfig::NoGlobals);
        if (!config->isConfigWritable(true)) {
            return false;
        }
        KConfigGroup cg = config->group("Notification Messages");
        if (m_askSave == AskSaveYes) {
            // Ask
            cg.deleteEntry(QStringLiteral("askSave") + name());
            cg.deleteEntry(QStringLiteral("askEmbedOrSave") + name());
        } else {
            // Do not ask, open
            cg.writeEntry(QStringLiteral("askSave") + name(), QStringLiteral("no"));
            cg.writeEntry(QStringLiteral("askEmbedOrSave") + name(), QStringLiteral("no"));
        }
    }

    writeAutoEmbed();

    bool needUpdateMimeDb = false;
    if (isMimeTypeDirty()) {
        MimeTypeWriter mimeTypeWriter(name());
        mimeTypeWriter.setComment(m_comment);
        if (!m_userSpecifiedIcon.isEmpty()) {
            mimeTypeWriter.setIconName(m_userSpecifiedIcon);
        }
        mimeTypeWriter.setPatterns(m_patterns);
        if (!mimeTypeWriter.write()) {
            return false;
        }
        m_userSpecifiedIconModified = false;
        needUpdateMimeDb = true;
    }

    syncServices();

    return needUpdateMimeDb;
}

static const char s_DefaultApplications[] = "Default Applications";
static const char s_AddedAssociations[] = "Added Associations";
static const char s_RemovedAssociations[] = "Removed Associations";

void MimeTypeData::syncServices()
{
    if (!m_bFullInit) {
        return;
    }

    KSharedConfig::Ptr profile = KSharedConfig::openConfig(QStringLiteral("mimeapps.list"), KConfig::NoGlobals, QStandardPaths::GenericConfigLocation);

    if (!profile->isConfigWritable(true)) { // warn user if mimeapps.list is root-owned (#155126/#94504)
        return;
    }

    const QStringList oldAppServices = getAppOffers();
    if (oldAppServices != m_appServices) {
        // Save the default application according to mime-apps-spec 1.0
        KConfigGroup defaultApp(profile, s_DefaultApplications);
        saveDefaultApplication(defaultApp, m_appServices);
        // Save preferred services
        KConfigGroup addedApps(profile, s_AddedAssociations);
        saveServices(addedApps, m_appServices);
        KConfigGroup removedApps(profile, s_RemovedAssociations);
        saveRemovedServices(removedApps, m_appServices, oldAppServices);
    }

    KSharedConfig::Ptr kpartsProfile = KSharedConfig::openConfig(QStringLiteral("kpartsrc"), KConfig::NoGlobals);
    const QStringList oldPartServices = getPartOffers();
    if (oldPartServices != m_embedParts) {
        KConfigGroup addedParts(kpartsProfile, "Added KDE Part Associations");
        if (m_embedParts.isEmpty()) {
            addedParts.deleteEntry(name());
        } else {
            addedParts.writeXdgListEntry(name(), m_embedParts);
        }
        KConfigGroup removedParts(kpartsProfile, "Removed KDE Part Associations");
        saveRemovedServices(removedParts, m_embedParts, oldPartServices);
    }

    // Clean out any kde-mimeapps.list which would take precedence any cancel our changes.
    const QString desktops = QString::fromLocal8Bit(qgetenv("XDG_CURRENT_DESKTOP"));
    const auto desktopsSplit = desktops.split(QLatin1Char(':'), Qt::SkipEmptyParts);
    for (const QString &desktop : desktopsSplit) {
        const QString file =
            QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/') + desktop.toLower() + QLatin1String("-mimeapps.list");
        if (QFileInfo::exists(file)) {
            qDebug() << "Cleaning up" << file;
            KConfig conf(file, KConfig::NoGlobals);
            KConfigGroup(&conf, s_DefaultApplications).deleteEntry(name());
            KConfigGroup(&conf, s_AddedAssociations).deleteEntry(name());
            KConfigGroup(&conf, s_RemovedAssociations).deleteEntry(name());
        }
    }

    m_appServicesModified = false;
    m_embedServicesModified = false;
}

static QStringList collectStorageIds(const QStringList &services)
{
    QStringList storageIds;

    for (const QString &service : services) {
        KService::Ptr pService = KService::serviceByStorageId(service);
        if (!pService) {
            qWarning() << "service with storage id" << service << "not found";
            continue; // Where did that one go?
        }

        storageIds.append(pService->storageId());
    }

    return storageIds;
}

void MimeTypeData::saveRemovedServices(KConfigGroup &config, const QStringList &services, const QStringList &oldServices)
{
    QStringList removedServiceList = config.readXdgListEntry(name());

    for (const QString &service : services) {
        // If removedServiceList.contains(service), then it was previously removed but has been added back
        removedServiceList.removeAll(service);
    }
    for (const QString &oldService : oldServices) {
        if (!services.contains(oldService)) {
            // The service was in m_appServices (or m_embedServices) but has been removed
            removedServiceList.append(oldService);
        }
    }
    if (removedServiceList.isEmpty()) {
        config.deleteEntry(name());
    } else {
        config.writeXdgListEntry(name(), removedServiceList);
    }
}

void MimeTypeData::saveServices(KConfigGroup &config, const QStringList &services)
{
    if (services.isEmpty()) {
        config.deleteEntry(name());
    } else {
        config.writeXdgListEntry(name(), collectStorageIds(services));
    }
}

void MimeTypeData::saveDefaultApplication(KConfigGroup &config, const QStringList &services)
{
    if (services.isEmpty()) {
        config.deleteEntry(name());
        return;
    }

    const QStringList storageIds = collectStorageIds(services);
    if (!storageIds.isEmpty()) {
        const QString firstStorageId = storageIds.at(0);
        config.writeXdgListEntry(name(), {firstStorageId});
    }
}

void MimeTypeData::refresh()
{
    if (m_isGroup) {
        return;
    }
    QMimeDatabase db;
    m_mimetype = db.mimeTypeForName(name());
    if (m_mimetype.isValid()) {
        if (m_bNewItem) {
            qDebug() << "OK, created" << name();
            m_bNewItem = false; // if this was a new mimetype, we just created it
        }
        if (!isMimeTypeDirty()) {
            // Update from the xml, in case something was changed from out of this kcm
            // (e.g. using KOpenWithDialog, or keditfiletype + kcmshell filetypes)
            initFromQMimeType();
        }
        if (!m_appServicesModified && !m_embedServicesModified) {
            m_bFullInit = false; // refresh services too
        }
    }
}

void MimeTypeData::getAskSave(bool &_askSave)
{
    if (m_askSave == AskSaveYes) {
        _askSave = true;
    }
    if (m_askSave == AskSaveNo) {
        _askSave = false;
    }
}

void MimeTypeData::setAskSave(bool _askSave)
{
    m_askSave = _askSave ? AskSaveYes : AskSaveNo;
}

bool MimeTypeData::canUseGroupSetting() const
{
    // "Use group settings" isn't available for zip, tar etc.; those have a builtin default...
    if (!m_mimetype.isValid()) { // e.g. new mimetype
        return true;
    }
    const bool hasLocalProtocolRedirect = !KProtocolManager::protocolForArchiveMimetype(name()).isEmpty();
    return !hasLocalProtocolRedirect;
}

void MimeTypeData::setPatterns(const QStringList &p)
{
    m_patterns = p;
    // Sort them, since update-mime-database doesn't respect order (order of globs file != order of xml),
    // and this code says things like if (m_mimetype.patterns() == m_patterns).
    // We could also sort in KMimeType::setPatterns but this would just slow down the
    // normal use case (anything else than this KCM) for no good reason.
    m_patterns.sort();
}

bool MimeTypeData::matchesFilter(const QString &filter) const
{
    if (name().contains(filter, Qt::CaseInsensitive)) {
        return true;
    }

    if (m_comment.contains(filter, Qt::CaseInsensitive)) {
        return true;
    }

    if (!m_patterns.filter(filter, Qt::CaseInsensitive).isEmpty()) {
        return true;
    }

    return false;
}

void MimeTypeData::setAppServices(const QStringList &dsl)
{
    if (!m_bFullInit) {
        getMyServiceOffers(); // so that m_bFullInit is true
    }
    m_appServices = dsl;
    m_appServicesModified = true;
}

void MimeTypeData::setEmbedParts(const QStringList &dsl)
{
    if (!m_bFullInit) {
        getMyServiceOffers(); // so that m_bFullInit is true
    }
    m_embedParts = dsl;
    m_embedServicesModified = true;
}

QString MimeTypeData::icon() const
{
    if (!m_userSpecifiedIcon.isEmpty()) {
        return m_userSpecifiedIcon;
    }
    if (m_mimetype.isValid()) {
        return m_mimetype.iconName();
    }
    return QString();
}
