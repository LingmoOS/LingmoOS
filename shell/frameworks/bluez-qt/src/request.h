/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_REQUEST_H
#define BLUEZQT_REQUEST_H

#include <QSharedPointer>

#include "bluezqt_export.h"

class QDBusMessage;

namespace BluezQt
{
enum RequestOriginatingType {
    OrgBluezAgent,
    OrgBluezProfile,
    OrgBluezObexAgent,
    OrgBluezMediaEndpoint,
};

/**
 * @class BluezQt::Request request.h <BluezQt/Request>
 *
 * D-Bus request.
 *
 * This class represents a request from a Bluetooth daemon. It is a convenient
 * wrapper around QDBusMessage and easily allows sending replies and handling errors.
 *
 * @see Agent, ObexAgent, Profile
 */
template<typename T = void>
class BLUEZQT_EXPORT Request
{
public:
    /**
     * Creates a new Request object.
     */
    explicit Request();

    /**
     * Destroys a Request object.
     */
    virtual ~Request();

    /**
     * Copy constructor.
     *
     * @param other
     */
    Request(const Request &other);

    /**
     * Copy assignment operator.
     *
     * @param other
     */
    Request &operator=(const Request &other);

    /**
     * Accepts the request.
     *
     * This method should be called to send a reply to indicate
     * the request was accepted.
     *
     * In case the request is of type void, this method does not
     * take any parameter.
     *
     * @param returnValue return value of request
     */
    void accept(T returnValue) const;

    /**
     * Rejects the request.
     *
     * This method should be called to send an error reply to
     * indicate the request was rejected.
     */
    void reject() const;

    /**
     * Cancels the request.
     *
     * This method should be called to send an error reply to
     * indicate the request was canceled.
     */
    void cancel() const;

private:
    explicit Request(RequestOriginatingType type, const QDBusMessage &message);

    QSharedPointer<class RequestPrivate> d;

    friend class AgentAdaptor;
    friend class ObexAgentAdaptor;
    friend class ProfileAdaptor;
    friend class MediaEndpointAdaptor;
};

// void
template<>
class BLUEZQT_EXPORT Request<void>
{
public:
    explicit Request();
    virtual ~Request();

    Request(const Request &other);
    Request &operator=(const Request &other);

    void accept() const;
    void reject() const;
    void cancel() const;

private:
    explicit Request(RequestOriginatingType type, const QDBusMessage &message);

    QSharedPointer<class RequestPrivate> d;

    friend class AgentAdaptor;
    friend class ObexAgentAdaptor;
    friend class ProfileAdaptor;
};

} // namespace BluezQt

#endif // BLUEZQT_REQUEST_H
