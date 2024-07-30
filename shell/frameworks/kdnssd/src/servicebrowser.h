/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDNSSDSERVICEBROWSER_H
#define KDNSSDSERVICEBROWSER_H

#include "remoteservice.h"
#include <QHostAddress>
#include <QObject>

#include <memory>

namespace KDNSSD
{
class DomainBrowser;
class ServiceBrowserPrivate;

/**
 * @class ServiceBrowser servicebrowser.h KDNSSD/ServiceBrowser
 * @short Browses for network services advertised over DNS-SD
 *
 * This is the central class in the KDNSSD library for applications
 * that want to discover services on network.
 *
 * Suppose that you need list of web servers running.  Then you
 * might do something like
 * @code
 * KDNSSD::ServiceBrowser *browser = new KDNSSD::ServiceBrowser(QStringLiteral("_http._tcp"));
 * connect(browser, &KDNSSD::ServiceBrowser::serviceAdded,
 *         this, [](KDNSSD::RemoteService::Ptr service) {
 *         });
 * connect(browser, &KDNSSD::ServiceBrowser::serviceRemoved,
 *         this, [](KDNSSD::RemoteService::Ptr service) {
 *         });
 * browser->startBrowse();
 * @endcode
 *
 * In above example addService() will be called for every web server
 * already running and for every web service that subsequently
 * appears on the network and delService() will be called when
 * a server previously advertised is stopped.
 *
 * Because no domain was passed to constructor, the default domain
 * will be searched.  To find other domains to browse for services on,
 * use DomainBrowser.
 *
 * @author Jakub Stachowski
 */
class KDNSSD_EXPORT ServiceBrowser : public QObject
{
    Q_OBJECT

public:
    /**
     * Availability of DNS-SD services
     */
    enum State {
        /** the service is available */
        Working,
        /** not available because mDnsd or Avahi daemon is not running */
        Stopped,
        /** not available because KDE was compiled without DNS-SD support */
        Unsupported,
    };

    /**
     * Create a ServiceBrowser for a particular service type
     *
     * DomainBrowser can be used to find other domains to browse on.
     * If no domain is given, the default domain is used.
     *
     * The service type is the high-level protocol type, followed by a dot,
     * followed by the transport protocol type (@c _tcp or @c _udp).
     * The <a href="http://www.dns-sd.org">DNS-SD website</a> maintains
     * <a href="http://www.dns-sd.org/ServiceTypes.html">a full list</a>
     * of service types.
     *
     * The @p subtype parameter allows you to do more fine-grained filtering
     * on the services you are interested in.  So you might request only
     * FTP servers that allow anonymous access by passing "_ftp._tcp" as the
     * @p type and "_anon" as the @p subtype.  Subtypes are particularly
     * important for types like _soap and _upnp, which are far too generic
     * for most applications.  In these cases, the subtype can be used to
     * specify the particular SOAP or UPnP protocol they want.
     *
     * @warning
     * Enabling @p autoResolve will increase network usage by resolving
     * all services, so this feature should be used only when necessary.
     *
     * @param type        service types to browse for (example: "_http._tcp")
     * @param autoResolve discovered services will be resolved before being
     *                    reported with the serviceAdded() signal
     * @param domain      a domain to search on instead of the default one
     * @param subtype     only browse for a specific subtype
     *
     * @see startBrowse() and isAvailable()
     */
    explicit ServiceBrowser(const QString &type, bool autoResolve = false, const QString &domain = QString(), const QString &subtype = QString());

    ~ServiceBrowser() override;

    /**
     * The currently known services of the specified type
     *
     * @returns a list of RemoteService pointers
     *
     * @see serviceAdded() and serviceRemoved()
     */
    QList<RemoteService::Ptr> services() const;

    /**
     * Starts browsing for services
     *
     * Only the first call to this function will have any effect.
     *
     * Browsing stops when the ServiceBrowser object is destroyed.
     *
     * @warning The serviceAdded() signal may be emitted before this
     *          function returns.
     *
     * @see serviceAdded(), serviceRemoved() and finished()
     */
    virtual void startBrowse();

    /**
     * Checks availability of DNS-SD services
     *
     * Although this method is part of ServiceBrowser, none of the classes
     * in this library will be able to perform their intended function
     * if this method does not return Working.
     *
     * If this method does not return Working, it is still safe to call
     * any of the methods in this library.  However, no services will be
     * found or published and no domains will be found.
     *
     * If you use this function to report an error to the user, below
     * is a suggestion on how to word the errors when publishing a
     * service.  The first line of each error message can also be
     * used for reporting errors when browsing for services.
     *
     * @code
     * switch(KDNSSD::ServiceBrowser::isAvailable()) {
     * case KDNSSD::ServiceBrowser::Working:
     *     return "";
     * case KDNSSD::ServiceBrowser::Stopped:
     *     return i18n("<p>The Zeroconf daemon is not running. See the Service"
     *                 " Discovery Handbook for more information.<br/>"
     *                 "Other users will not see the services provided by this
     *                 " system when browsing the network via zeroconf, but "
     *                 " normal access will still work.</p>");
     * case KDNSSD::ServiceBrowser::Unsupported:
     *     return i18n("<p>Zeroconf support is not available in this version of KDE."
     *                 " See the Service Discovery Handbook for more information.<br/>"
     *                 "Other users will not see the services provided by this
     *                 " application when browsing the network via zeroconf, but "
     *                 " normal access will still work.</p>");
     * default:
     *     return i18n("<p>Unknown error with Zeroconf.<br/>"
     *                 "Other users will not see the services provided by this
     *                 " application when browsing the network via zeroconf, but "
     *                 " normal access will still work.</p>");
     * }
     * @endcode
     *
     */
    static State isAvailable();

    /**
     * Whether discovered services are resolved before being reported
     *
     * @return the value of the @p autoResolve parameter passed to the constructor
     *
     * @since 4.1
     */
    bool isAutoResolving() const;

    /**
     * Resolves an mDNS hostname into an IP address
     *
     * This function is very rarely useful, since a properly configured
     * system is able to resolve an mDNS-based host name using the system
     * resolver (ie: you can just pass the mDNS hostname to KIO or other
     * library).
     *
     * @param hostname the hostname to be resolved
     * @return a QHostAddress containing the IP address, or QHostAddress() if
     *         resolution failed
     * @since 4.2
     */
    static QHostAddress resolveHostName(const QString &hostname);

    /**
     * The mDNS hostname of the local machine
     *
     * Usually this will return the same as QHostInfo::localHostName(),
     * but it may be changed to something different
     * in the Avahi configuration file (if using the Avahi backend).
     *
     * @return the hostname, or an empty string on failure
     * @since 4.2
     */
    static QString getLocalHostName();

Q_SIGNALS:
    /**
     * Emitted when new service is discovered
     *
     * If isAutoResolving() returns @c true, this will not be emitted
     * until the service has been resolved.
     *
     * @param service a RemoteService object describing the service
     *
     * @see serviceRemoved() and finished()
     */
    void serviceAdded(KDNSSD::RemoteService::Ptr service);

    /**
     * Emitted when a service is no longer published over DNS-SD
     *
     * The RemoteService object is removed from the services() list
     * and deleted immediately after this signal returns.
     *
     * @warning
     * Do @b not use a delayed connection with this signal
     *
     * @param service a RemoteService object describing the service
     *
     * @see serviceAdded() and finished()
     */
    void serviceRemoved(KDNSSD::RemoteService::Ptr service);

    /**
     * Emitted when the list of published services has settled
     *
     * This signal is emitted once after startBrowse() is called
     * when all the services of the requested type that are
     * currently published have been reported (even if none
     * are available or the DNS-SD service is not available).
     * It is emitted again when a new batch of services become
     * available or disappear.
     *
     * For example, if a new host is connected to network and
     * announces some services watched for by this ServiceBrowser,
     * they will be reported by one or more serviceAdded() signals
     * and the whole batch will be concluded by finished().
     *
     * This signal can be used by applications that just want to
     * get a list of the currently available services
     * (similar to a directory listing) and do not care about
     * adding or removing services that appear or disappear later.
     *
     * @warning
     * There is no guarantee any RemoteService
     * pointers received by serviceAdded() will be valid
     * by the time this signal is emitted, so you should either
     * do all your work involving them in the slot receiving
     * the serviceAdded() signal, or you should listen to
     * serviceRemoved() as well.
     *
     * @see serviceAdded() and serviceRemoved()
     */
    void finished();

protected:
    virtual void virtual_hook(int, void *);

private:
    friend class ServiceBrowserPrivate;
    std::unique_ptr<ServiceBrowserPrivate> const d;
    Q_DECLARE_PRIVATE_D(d, ServiceBrowser)
};

}

#endif
