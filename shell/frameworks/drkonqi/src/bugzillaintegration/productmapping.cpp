/*******************************************************************
 * productmapping.cpp
 * SPDX-FileCopyrightText: 2009 Dario Andres Rodriguez <andresbajotierra@gmail.com>
 * SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 ******************************************************************/

#include "productmapping.h"

#include "drkonqi_debug.h"
#include <KConfig>
#include <KConfigGroup>
#include <QStandardPaths>

#include "bugzillalib.h"
#include "crashedapplication.h"

ProductMapping::ProductMapping(const CrashedApplication *crashedApp, BugzillaManager *bzManager, QObject *parent)
    : QObject(parent)
    , m_crashedAppPtr(crashedApp)
    , m_bugzillaManagerPtr(bzManager)
    , m_bugzillaProductDisabled(false)
    , m_bugzillaVersionDisabled(false)

{
    // Default "fallback" values
    m_bugzillaProduct = crashedApp->fakeExecutableBaseName();
    m_bugzillaComponent = QStringLiteral("general");
    m_bugzillaVersionString = QStringLiteral("unspecified");
    m_relatedBugzillaProducts = QStringList() << m_bugzillaProduct;

    if (!crashedApp->productName().isEmpty()) {
        const auto l = crashedApp->productName().split(QLatin1Char('/'), Qt::SkipEmptyParts);
        if (l.size() == 2) {
            m_bugzillaProduct = l[0];
            m_bugzillaComponent = l[1];
        } else {
            m_bugzillaProduct = crashedApp->productName();
        }
        m_hasExternallyProvidedProductName = true;
    }

    map(crashedApp->fakeExecutableBaseName());

    // Get valid versions
    connect(m_bugzillaManagerPtr, &BugzillaManager::productInfoFetched, this, &ProductMapping::checkProductInfo);
    // Holding the connection so we can easily disconnect in the fallback logic.
    m_productInfoErrorConnection = connect(m_bugzillaManagerPtr, &BugzillaManager::productInfoError, this, &ProductMapping::fallBackToKDE);

    m_bugzillaManagerPtr->fetchProductInfo(m_bugzillaProduct);
}

void ProductMapping::map(const QString &appName)
{
    mapUsingInternalFile(appName);
    getRelatedProductsUsingInternalFile(m_bugzillaProduct);
}

void ProductMapping::mapUsingInternalFile(const QString &appName)
{
    KConfig mappingsFile(QString::fromLatin1("mappings"), KConfig::NoGlobals, QStandardPaths::AppDataLocation);
    const KConfigGroup mappings = mappingsFile.group(QStringLiteral("Mappings"));
    if (mappings.hasKey(appName)) {
        if (m_hasExternallyProvidedProductName) {
            qCWarning(DRKONQI_LOG) << "Mapping found despite product information being provided by the application. Consider removing the mapping entry"
                                   << appName;
        }
        QString mappingString = mappings.readEntry(appName);
        if (!mappingString.isEmpty()) {
            QStringList list = mappingString.split(QLatin1Char('|'), Qt::SkipEmptyParts);
            if (list.count() == 2) {
                m_bugzillaProduct = list.at(0);
                m_bugzillaComponent = list.at(1);
                m_relatedBugzillaProducts = QStringList() << m_bugzillaProduct;
            } else {
                qCWarning(DRKONQI_LOG) << "Error while reading mapping entry. Sections found " << list.count();
            }
        } else {
            qCWarning(DRKONQI_LOG) << "Error while reading mapping entry. Entry exists but it is empty "
                                      "(or there was an error when reading)";
        }
    }
}

void ProductMapping::getRelatedProductsUsingInternalFile(const QString &bugzillaProduct)
{
    // ProductGroup ->  kontact=kdepim
    // Groups -> kdepim=kontact|kmail|korganizer|akonadi|pimlibs..etc

    KConfig mappingsFile(QString::fromLatin1("mappings"), KConfig::NoGlobals, QStandardPaths::AppDataLocation);
    const KConfigGroup productGroup = mappingsFile.group(QStringLiteral("ProductGroup"));

    // Get groups of the application
    QStringList groups;
    if (productGroup.hasKey(bugzillaProduct)) {
        QString group = productGroup.readEntry(bugzillaProduct);
        if (group.isEmpty()) {
            qCWarning(DRKONQI_LOG) << "Error while reading mapping entry. Entry exists but it is empty "
                                      "(or there was an error when reading)";
            return;
        }
        groups = group.split(QLatin1Char('|'), Qt::SkipEmptyParts);
    }

    // All KDE apps use the KDE Platform (basic libs)
    groups << QLatin1String("kdeplatform");

    // Add the product itself
    m_relatedBugzillaProducts = QStringList() << m_bugzillaProduct;

    // Get related products of each related group
    for (const QString &group : std::as_const(groups)) {
        const KConfigGroup bzGroups = mappingsFile.group(QStringLiteral("BZGroups"));
        if (bzGroups.hasKey(group)) {
            QString bzGroup = bzGroups.readEntry(group);
            if (!bzGroup.isEmpty()) {
                const QStringList relatedGroups = bzGroup.split(QLatin1Char('|'), Qt::SkipEmptyParts);
                if (!relatedGroups.isEmpty()) {
                    m_relatedBugzillaProducts.append(relatedGroups);
                }
            } else {
                qCWarning(DRKONQI_LOG) << "Error while reading mapping entry. Entry exists but it is empty "
                                          "(or there was an error when reading)";
            }
        }
    }
}

void ProductMapping::checkProductInfo(const Bugzilla::Product::Ptr product)
{
    // check whether the product itself is disabled for new reports,
    // which usually means that product/application is unmaintained.
    m_bugzillaProductDisabled = !product->isActive();

    // check whether the product on bugzilla contains the expected component
    if (!product->componentNames().contains(m_bugzillaComponent)) {
        m_bugzillaComponent = QLatin1String("general");
    }

    // find the appropriate version to use on bugzilla
    const QString version = m_crashedAppPtr->version();
    const QStringList &allVersions = product->allVersions();

    if (allVersions.contains(version)) {
        // The version the crash application provided is a valid bugzilla version: use it !
        m_bugzillaVersionString = version;
    } else if (version.endsWith(QLatin1String(".00"))) {
        // check if there is a version on bugzilla with just ".0"
        const QString shorterVersion = version.left(version.size() - 1);
        if (allVersions.contains(shorterVersion)) {
            m_bugzillaVersionString = shorterVersion;
        }
    } else if (!allVersions.contains(m_bugzillaVersionString)) {
        // No good match found, make sure the default is sound...
        // If our hardcoded fallback is not in bugzilla it was likely
        // renamed so we'll find the version with the lowest id instead
        // and that should technically have been the "default" version.
        Bugzilla::ProductVersion *lowestVersion = nullptr;
        const QList<Bugzilla::ProductVersion *> versions = product->versions();
        for (const auto &version : versions) {
            if (!lowestVersion || lowestVersion->id() > version->id()) {
                lowestVersion = version;
            }
        }
        if (lowestVersion) {
            m_bugzillaVersionString = lowestVersion->name();
        }
    }

    // check whether that versions is disabled for new reports, which
    // usually means that version is outdated and not supported anymore.
    const QStringList &inactiveVersions = product->inactiveVersions();
    m_bugzillaVersionDisabled = inactiveVersions.contains(m_bugzillaVersionString);

    Q_EMIT resolved();
}

void ProductMapping::fallBackToKDE()
{
    // Fall back to the generic kde product when we couldn't find an explicit mapping.
    // This is in an effort to make it as easy as possible to file a bug, unfortunately it means someone will
    // have to triage it accordingly.
    // Disconnect to safe-guard against infinite loop should kde also fail for some reason....
    //   An argument could be made that we should raise a user error if this fails again,
    //   'kde' not resolving shouldn't ever happen and points at a huge problem somewhere.
    disconnect(m_productInfoErrorConnection);
    m_bugzillaProductOriginal = m_bugzillaProduct;
    m_bugzillaProduct = QStringLiteral("kde");
    m_bugzillaManagerPtr->fetchProductInfo(m_bugzillaProduct);

    Q_EMIT resolved();
}

QStringList ProductMapping::relatedBugzillaProducts() const
{
    return m_relatedBugzillaProducts;
}

QString ProductMapping::bugzillaProduct() const
{
    return m_bugzillaProduct;
}

QString ProductMapping::bugzillaComponent() const
{
    return m_bugzillaComponent;
}

QString ProductMapping::bugzillaVersion() const
{
    return m_bugzillaVersionString;
}

bool ProductMapping::bugzillaProductDisabled() const
{
    return m_bugzillaProductDisabled;
}

bool ProductMapping::bugzillaVersionDisabled() const
{
    return m_bugzillaVersionDisabled;
}

QString ProductMapping::bugzillaProductOriginal() const
{
    return m_bugzillaProductOriginal;
}

#include "moc_productmapping.cpp"
