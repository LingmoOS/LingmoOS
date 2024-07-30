/*
    SPDX-FileCopyrightText: 2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef OPDSPROVIDER_H
#define OPDSPROVIDER_H

#include "provider.h"
#include "xmlloader_p.h"
#include <QMap>
#include <memory>

/**
 * OPDS provider.
 *
 * The OPDS provider loads OPDS feeds:
 * https://specs.opds.io/opds-1.2
 *
 * These feeds are most common with online book providers, but the format itself is agnostic.
 * For loading feeds, these, as with other providers, need to have a KNSRC file pointed
 * at a Provider.xml, with the "type" element containing "opds" as text.
 *
 * Supports:
 * - Loads a given feed, it's images, and loads it's download links.
 * - Opensearch for the search, if available.
 * - Should load full entries, if possible.
 * - Navigation feed entries can be selected.
 *
 * TODO:
 * - We need a better handling of non-free items (requires authentication).
 * - entry navigation links are not supported.
 * - pagination support (together with the navigation links)
 * - No Sorting
 *
 * Would-be-nice, but requires a lot of rewiring in knewstuff:
 * - We could get authenticated feeds going by using basic http authentiation(in spec), or have bearer token uris (oauth bearcaps).
 * - Autodiscovery or protocol based discovery of opds catalogs, this does not gel with the provider xml system used by knewstuff.
 *
 * @since 5.83
 */

namespace KNSCore
{
class OPDSProviderPrivate;
class OPDSProvider : public Provider
{
    Q_OBJECT
public:
    typedef QList<Provider *> List;

    OPDSProvider();
    ~OPDSProvider() override;

    // Unique ID, url of the feed.
    QString id() const override;

    // Name of the feed.
    QString name() const override;

    // Feed icon
    QUrl icon() const override;

    void loadEntries(const KNSCore::Provider::SearchRequest &request) override;
    void loadEntryDetails(const KNSCore::Entry &entry) override;
    void loadPayloadLink(const KNSCore::Entry &entry, int linkNumber) override;

    bool setProviderXML(const QDomElement &xmldata) override;
    bool isInitialized() const override;
    void setCachedEntries(const KNSCore::Entry::List &cachedEntries) override;

    const std::unique_ptr<OPDSProviderPrivate> d;

    Q_DISABLE_COPY(OPDSProvider)
};

}

#endif // OPDSPROVIDER_H
