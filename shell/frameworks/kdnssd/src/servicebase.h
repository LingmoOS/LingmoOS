/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDNSSDSERVICEBASE_H
#define KDNSSDSERVICEBASE_H

#include "kdnssd_export.h"
#include <QExplicitlySharedDataPointer>
#include <QMap>
#include <QString>
#include <memory>

namespace KDNSSD
{
class ServiceBasePrivate;

/**
 * @class ServiceBase servicebase.h KDNSSD/ServiceBase
 * @short Describes a service
 *
 * This class is used to describe a service. The service
 * can be published by the current application (in which
 * case it is probably a PublicService) or by
 * another application, either on the current machine or
 * a remote machine, in which case it is probably a
 * RemoteService returned by ServiceBrowser.
 *
 * You should not normally need to create a ServiceBase
 * object yourself.
 *
 * @author Jakub Stachowski
 *
 * @see PublicService
 */
class KDNSSD_EXPORT ServiceBase : public QSharedData // krazy:exclude=dpointer (protected)
{
public:
    typedef QExplicitlySharedDataPointer<ServiceBase> Ptr;

    /**
     * Creates a ServiceBase object
     *
     * Note that @p name, @p type and @p domain uniquely identify
     * the service in the DNS-SD system, and @p host and @p port
     * provide the actual location of the service.
     *
     * For example, RemoteService populates @p host and @p port
     * based on the @p name, @p type and @p domain attributes
     * using the DNS-SD resolution system.
     *
     * @param name   service name
     * @param type   service type
     * @param domain the DNS-SD domain name for service
     * @param host   the host name of the service (a fully-qualified domain name)
     * @param port   the port number of the service
     */
    explicit ServiceBase(const QString &name = QString(),
                         const QString &type = QString(),
                         const QString &domain = QString(),
                         const QString &host = QString(),
                         unsigned short port = 0);

    virtual ~ServiceBase();

    /**
     * The name of the service
     */
    QString serviceName() const;

    /**
     * The type of the service
     *
     * This is always in the format _sometype._udp or _sometype._tcp.
     *
     * See the <a href="http://www.dns-sd.org">DNS-SD website</a> for
     * <a href="http://www.dns-sd.org/ServiceTypes.html">a full list of service types</a>.
     */
    QString type() const;

    /**
     * The domain that the service belongs to
     *
     * It is "local." for link-local services.
     */
    QString domain() const;

    /**
     * The hostname of the service
     *
     * Only valid for local and resolved remote services.
     *
     * Together with port(), this can be used to actually
     * access the service.
     *
     * @see RemoteService::resolve() and RemoteService::resolveAsync()
     */
    QString hostName() const;

    /**
     * The port number of the service
     *
     * Only valid for local and resolved remote services.
     *
     * Together with hostName(), this can be used to actually
     * access the service.
     *
     * @see RemoteService::resolve() and RemoteService::resolveAsync()
     */
    unsigned short port() const;

    /**
     * Additional text data associated with the service
     *
     * Only valid for local and resolved remote services.
     *
     * This is data that provides additional information about the
     * service.  For example, it might be used to specify a printer
     * queue on the printer server specified by hostName() and port().
     *
     * You can check for the data that might be associated with a
     * particular service on the <a
     * href="http://www.dns-sd.org/ServiceTypes.html">service types list</a>.
     * If a @c key=value pair is given, this will appear with the @c value
     * in a QByteArray indexed by the @c key.  If the data is on its own
     * (does not have an @c = in it), it will be used to index an empty
     * QByteArray, and can be checked for with QMap::contains().
     *
     * For example, if you are accessing the _ipp._tcp service, you might
     * do something like
     * @code
     * QString printerModel = "unknown";
     * if (service->textData().contains("ty")) {
     *     printQueue = QString::fromUtf8(service->textData()["ty"].constData());
     * }
     * @endcode
     * since the TXT data of the IPP service may contain data like
     * "ty=Apple LaserWriter Pro 630".  Note that you actually have to be
     * a bit more clever than this, since the key should usually be case
     * insensitive.
     */
    QMap<QString, QByteArray> textData() const;

    /**
     * Compares services based on name, type and domain
     *
     * This is enough to for unique identification and omitting
     * port, host and text data allows to compare resolved and
     * unresolved services
     *
     * @param o the service to compare this service to
     * @return @c true if this service represents the same
     *         service (from the point of view of DNS-SD) as
     *         @p o, @c false otherwise
     */
    bool operator==(const ServiceBase &o) const;
    /**
     * Compares services based on name, type and domain
     *
     * This is enough to for unique identification and omitting
     * port, host and text data allows to compare resolved and
     * unresolved services
     *
     * @param o the service to compare this service to
     * @return @c false if this service represents the same
     *         service (from the point of view of DNS-SD) as
     *         @p o, @c true otherwise
     */
    bool operator!=(const ServiceBase &o) const;

protected:
    KDNSSD_NO_EXPORT explicit ServiceBase(ServiceBasePrivate *const d);

    virtual void virtual_hook(int, void *);

protected:
    std::unique_ptr<ServiceBasePrivate> const d;
    // We cannot use Q_DECLARE_PRIVATE_D & Q_D here because of multiple inheritance with some
    // of the subclasses of ServiceBasePrivate, where ServiceBasePrivate is not the first base class,
    // so reinterpret_cast as used by the functions defined with Q_DECLARE_PRIVATE_D would fail.
    // Using a custom macro here with static_cast would require to know about the type definition
    // of the private classes, which we though want to avoid here in the public class.
    // So instead some custom KDNSSD_D macros are used internally...
};

/* Utility functions */

/**
 * Check if the domain is link-local
 *
 * @return @c true if domain is link-local ('local.'), @c false otherwise
 */
bool domainIsLocal(const QString &domain);

}

#endif
