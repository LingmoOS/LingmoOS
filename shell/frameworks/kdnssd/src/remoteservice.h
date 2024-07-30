/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004, 2005 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDNSSDREMOTESERVICE_H
#define KDNSSDREMOTESERVICE_H

#include "servicebase.h"
#include <QMetaType>
#include <QObject>

namespace KDNSSD
{
class RemoteServicePrivate;

/**
 * @class RemoteService remoteservice.h KDNSSD/RemoteService
 * @short Describes a service published over DNS-SD,
 *        typically on a remote machine
 *
 * This class allows delayed or asynchronous resolution of
 * services.  As the name suggests, the service is normally
 * on a remote machine, but the service could just as easily
 * be published on the local machine.
 *
 * RemoteService instances are normally provided by ServiceBrowser,
 * but can be used to resolve any service if you know the name, type
 * and domain for it.
 *
 * @author Jakub Stachowski
 *
 * @see ServiceBrowser
 */
class KDNSSD_EXPORT RemoteService : public QObject, public ServiceBase
{
    Q_OBJECT

public:
    typedef QExplicitlySharedDataPointer<RemoteService> Ptr;

    /**
     * Creates an unresolved RemoteService representing the service with
     * the given name, type and domain
     *
     * @param name   the name of the service
     * @param type   the type of the service (see ServiceBrowser::ServiceBrowser())
     * @param domain the domain of the service
     *
     * @see ServiceBrowser::isAvailable()
     */
    RemoteService(const QString &name, const QString &type, const QString &domain);

    ~RemoteService() override;

    /**
     * Resolves the host name and port of service asynchronously
     *
     * The host name is not resolved into an IP address - use KResolver
     * for that.
     *
     * The resolved(bool) signal will be emitted when the
     * resolution is complete, or when it fails.
     *
     * Note that resolved(bool) may be emitted before this function
     * returns in case of immediate failure.
     *
     * RemoteService will keep monitoring the service for
     * changes in hostname and port, and re-emit resolved(bool)
     * when either changes.
     *
     * @see resolve(), hostName(), port()
     */
    void resolveAsync();

    /**
     * Resolves the host name and port of service synchronously
     *
     * The host name is not resolved into an IP address - use KResolver
     * for that.
     *
     * resolved(bool) is emitted before this function is returned.
     *
     * resolve() will not cause RemoteService to monitor for changes
     * in the hostname or port of the service.
     *
     * @return @c true if successful, @c false on failure
     *
     * @see resolveAsync(), hostName(), port()
     */
    bool resolve();

    /**
     * Whether the service has been successfully resolved
     *
     * @return @c true if hostName() and port() will return
     *         valid values, @c false otherwise
     */
    bool isResolved() const;

Q_SIGNALS:
    /**
     * Emitted when resolving is complete
     *
     * If operating in asynchronous mode this signal can be
     * emitted several times (when the hostName or port of
     * the service changes).
     *
     * @param successful @c true if the hostName and port were
     *                   successfully resolved, @c false otherwise
     */
    void resolved(bool successful);

protected:
    void virtual_hook(int id, void *data) override;

private:
    friend class RemoteServicePrivate;
};

}

Q_DECLARE_METATYPE(KDNSSD::RemoteService::Ptr)

#endif
