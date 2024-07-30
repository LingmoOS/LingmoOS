/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004, 2005 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDNSSDPUBLICSERVICE_H
#define KDNSSDPUBLICSERVICE_H

#include "servicebase.h"
#include <QObject>
#include <QStringList>

namespace KDNSSD
{
class PublicServicePrivate;

/**
 * @class PublicService publicservice.h KDNSSD/PublicService
 * @short Represents a service to be published
 *
 * This class allows you to publish the existence of a network
 * service provided by your application.
 *
 * If you are providing a web server and want to advertise it
 * on the local network, you might do
 * @code
 * KDNSSD::PublicService *service = new KDNSSD::PublicService("My files", "_http._tcp", 80);
 * bool isOK = service->publish();
 * @endcode
 *
 * In this example publish() is synchronous: it will not return
 * until publishing is complete.  This is usually not too long
 * but it can freeze an application's GUI for a moment.
 * To publish asynchronously instead, do:
 * @code
 * KDNSSD::PublicService *service = new KDNSSD::PublicService("My files", "_http._tcp", 80);
 * connect(service, SIGNAL(published(bool)), this, SLOT(wasPublished(bool)));
 * service->publishAsync();
 * @endcode
 *
 * @author Jakub Stachowski
 */

class KDNSSD_EXPORT PublicService : public QObject, public ServiceBase
{
    Q_OBJECT

public:
    /**
     * Creates a service description that can be published
     *
     * If no @p name is given, the computer name is used instead.  If there
     * is already a service with the same name, type and domain a number will
     * be appended to the name to make it unique.
     *
     * If no @p domain is specified, the service is published on the link-local
     * domain (.local).
     *
     * The subtypes can be used to specify server attributes, such
     * as "_anon" for anonymous FTP servers, or can specify a specific protocol
     * (such as a web service interface) on top of a generic protocol like SOAP.
     *
     * There is
     * <a href="http://www.dns-sd.org/ServiceTypes.html">a comprehensive list
     * of possible types</a> available, but you are largely on your own for
     * subtypes.
     *
     * @param name     a service name to use instead of the computer name
     * @param type     service type, in the form _sometype._udp or _sometype._tcp
     * @param port     port number, or 0 to "reserve" the service name
     * @param domain   the domain to publish the service on (see DomainBrowser)
     * @param subtypes optional list of subtypes, each with a leading underscore
     *
     * @see ServiceBrowser::ServiceBrowser()
     */
    explicit PublicService(const QString &name = QString(),
                           const QString &type = QString(),
                           unsigned int port = 0,
                           const QString &domain = QString(),
                           const QStringList &subtypes = QStringList());

    ~PublicService() override;

    /**
     * Stops publishing or aborts an incomplete publish request.
     *
     * Useful when you want to disable the service for some time.
     *
     * Note that if you stop providing a service (without exiting the
     * application), you should stop publishing it.
     */
    void stop();

    /**
     * Publish the service synchronously
     *
     * The method will not return (and hence the application interface will
     * freeze, since KDElibs code should be executed in the main thread)
     * until either the service is published or publishing fails.
     *
     * published(bool) is emitted before this method returns.
     *
     * @return @c true if the service was successfully published, @c false otherwise
     */
    bool publish();

    /**
     * Whether the service is currently published
     *
     * @return @c true if the service is being published to the domain,
     *         @c false otherwise
     */
    bool isPublished() const;

    /**
     * Publish the service asynchronously
     *
     * Returns immediately and emits published(bool) when completed.
     * Note that published(bool) may be emitted before this method
     * returns when an error is detected immediately.
     */
    void publishAsync();

    /**
     * Sets new text properties
     *
     * If the service is already published, it will be re-announced with
     * the new data.
     *
     * @param textData the new text properties for the service
     *
     * @see ServiceBase::textData()
     */
    void setTextData(const QMap<QString, QByteArray> &textData);

    /**
     * Sets the name of the service
     *
     * If the service is already published, it will be re-announced with
     * the new name.
     *
     * @param serviceName the new name of the service
     */
    void setServiceName(const QString &serviceName);

    /**
     * Sets the service type
     *
     * If the service is already published, it will be re-announced with
     * the new type.
     *
     * @param type the new type of the service
     *
     * See PublicService() for details on the format of @p type
     */
    void setType(const QString &type);

    /**
     * Sets the subtypetypes of the service
     *
     * If the service is already published, it will be re-announced with
     * the new subtypes.
     *
     * The existing list of substypes is replaced, so an empty list will
     * cause all existing subtypes to be removed.
     *
     * @param subtypes the new list of subtypes
     */
    void setSubTypes(const QStringList &subtypes);

    /**
     * Sets the port
     *
     * If the service is already published, it will be re-announced with
     * the new port.
     *
     * @param port the port of the service, or 0 to simply "reserve" the name
     */
    void setPort(unsigned short port);

    /**
     * Sets the domain where the service is published
     *
     * "local." means link-local, ie: the IP subnet on the LAN containing
     * this computer.
     *
     * If service is already published, it will be removed from the current
     * domain and published on @p domain instead.
     *
     * @param domain the new domain to publish the service on
     */
    void setDomain(const QString &domain);

    /**
     * The subtypes of service.
     *
     * @see setSubTypes()
     */
    QStringList subtypes() const;

Q_SIGNALS:
    /**
     * Emitted when publishing is complete
     *
     * It will also emitted when an already-published service is
     * republished after a property of the service (such as the
     * name or port) is changed.
     */
    void published(bool successful);

protected:
    void virtual_hook(int, void *) override;

private:
    friend class PublicServicePrivate;
};

}

#endif
