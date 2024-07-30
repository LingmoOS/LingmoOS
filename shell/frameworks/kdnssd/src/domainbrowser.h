/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDNSSDDOMAINBROWSER_H
#define KDNSSDDOMAINBROWSER_H

#include "remoteservice.h"
#include <QObject>
#include <QtContainerFwd>
#include <memory>

namespace KDNSSD
{
class DomainBrowserPrivate;

/**
 * @class DomainBrowser domainbrowser.h KDNSSD/DomainBrowser
 * @short Browses recommended domains for browsing or publishing to.
 *
 * Usage of this class is very simple.  If you are interested in
 * browsing for services, simple do
 * @code
 * KDNSSD::DomainBrowser *browser =
 *     new KDNSSD::DomainBrowser(KDNSSD::DomainBrowser::Browsing, this);
 * connect(browser, SIGNAL(domainAdded(QString)),
 *         this, SLOT(browsingDomainAdded(QString));
 * connect(browser, SIGNAL(domainRemoved(QString)),
 *         this, SLOT(browsingDomainRemove(QString));
 * browser->startBrowse();
 * @endcode
 *
 * If you are interested in domains where you can register services,
 * usage is identical except that you should pass
 * <tt>KDNSSD::DomainBrowser::Registering</tt> to the constructor.
 *
 * @author Jakub Stachowski
 */
class KDNSSD_EXPORT DomainBrowser : public QObject
{
    Q_OBJECT
public:
    /**
     * A type of domain recommendation
     */
    enum DomainType {
        /** Domains recommended for browsing for services on (using ServiceBrowser) */
        Browsing,
        /** Domains recommended for publishing to (using PublicService) */
        Publishing,
    };
    /**
     * Standard constructor
     *
     * The global DNS-SD configuration (for example, the global Avahi
     * configuration for the Avahi backend) will be used.
     *
     * @param type   the type of domain to search for
     * @param parent parent object (see QObject documentation)
     *
     * @see startBrowse() and ServiceBrowser::isAvailable()
     */
    explicit DomainBrowser(DomainType type, QObject *parent = nullptr);

    ~DomainBrowser() override;

    /**
     * The current known list of domains of the requested DomainType
     *
     * @return a list of currently known domain names
     */
    QStringList domains() const;

    /**
     * Starts browsing
     *
     * Only the first call to this function will have any effect.
     *
     * Browsing stops when the DomainBrowser object is destroyed.
     *
     * @warning The domainAdded() signal may be emitted before this
     *          function returns.
     *
     * @see domainAdded() and domainRemoved()
     */
    void startBrowse();

    /**
     * Whether the browsing has been started
     *
     * @return @c true if startBrowse() has been called, @c false otherwise
     */
    bool isRunning() const;

Q_SIGNALS:
    /**
     * A domain has disappeared from the browsed list
     *
     * Emitted when domain has been removed from browsing list
     * or the publishing list (depending on which list was
     * requested in the constructor).
     *
     * @param domain the name of the domain
     *
     * @see domainAdded()
     */
    void domainRemoved(const QString &domain);

    /**
     * A new domain has been discovered
     *
     * If the requested DomainType is Browsing, this will
     * also be emitted for the domains specified in the
     * global configuration.
     *
     * @param domain the name of the domain
     *
     * @see domainRemoved()
     */
    void domainAdded(const QString &domain);

private:
    friend class DomainBrowserPrivate;
    std::unique_ptr<DomainBrowserPrivate> const d;
    Q_DECLARE_PRIVATE_D(d, DomainBrowser)
};

}

#endif
